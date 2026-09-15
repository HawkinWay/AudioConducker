# AudioConducker Debug Errors

几个很典型的 **C++ 多线程 + PipeWire 异步事件循环 + 资源生命周期** 问题

## AudioConducker：PipeWire Shutdown

| 问题 | 根本原因 | 解决方案 |
| --- | --- | ---|
| **1. Ctrl+C 后无法正常退出 / PipeWire 报 wrong context**  | `pw_main_loop` 已经在专用线程运行，却又在主线程调用 `pw_main_loop_run()`，导致**两个线程同时进入同一个 PipeWire loop**                          | PipeWire loop 只允许专用线程运行；主线程不再 `run()`，改用 `pw_loop_invoke()` 把操作提交给 loop 线程                  |
| **2. `engine.shutDown()` 后程序仍然提前退出**              | `setVolume()` 是**异步**的，只是把恢复音量请求放进 PipeWire loop，并不保证请求已经执行                                                     | 增加 `context.sync()`，通过 `pw_core_sync()` + `core.done` 建立一个**同步屏障**，等待之前的异步操作完成              |
| **3. Shutdown 时销毁 PipeWire 对象不安全**                | `PipeWireStream`、`pw_node`、`NodeObserver` 等对象属于 PipeWire context，不能在线程/loop 生命周期已经结束后随意销毁                       | 增加 `backend.shutdown()`，把资源销毁操作 `pw_loop_invoke()` 到 PipeWire loop 线程执行                     |
| **4. Backend destructor 中异步销毁 proxy 有生命周期问题**     | destructor 里 `pw_loop_invoke(..., false)` 只是排队；但对象可能马上析构、loop 也可能停止，导致 callback 没机会执行                           | 把资源销毁从 destructor 移到显式 `shutdown()`；确保 **destroy → quit → join** 的顺序正确                      |
| **5. `onNodeRemoved()` 中重复使用 `pw_loop_invoke()`** | `onNodeRemoved()` 本身就是在 PipeWire loop callback 中执行，已经处于正确线程                                                     | 直接 `pw_proxy_destroy()`，不需要再次 invoke                                                        |
| **6. `sync()` 出现 SIGSEGV**                        | `pw_loop_invoke()` 的 `data / size / user_data` 参数使用不一致：callback 把 `data` 当成 `SyncData*`，但实际传入的 `size=0`，导致错误解引用 | 明确区分 **data payload** 和 **user_data**；同步状态通过 `user_data` 传给 callback，callback 不再错误解析 `data` |
| **7. Shutdown 时资源销毁顺序混乱**                         | PipeWire 对象之间存在依赖关系，例如 listener、stream、node proxy、registry observer                                             | 在 loop 线程中统一执行 shutdown，并按照生命周期依赖依次移除 listener、销毁 stream/proxy、清理容器                         |

---

## 最核心的两个问题

### ① Thread / PipeWire Loop 问题

之前相当于：

```text
Main Thread
    │
    └── pw_main_loop_run()  ❌
              │
              ↓
        PipeWire Loop

Loop Thread
    │
    └── pw_main_loop_run()  ❌
              │
              ↓
        同一个 PipeWire Loop
```

**同一个 `spa_loop` 不能这样被两个线程同时驱动。**

所以才出现：

```text
impl_ext_end_proxy called from wrong context
pthread_equal(...) failed
Operation not permitted
```

正确结构：

```text
Main Thread                    PipeWire Thread
    │                                │
    │                                ├── pw_main_loop_run()
    │                                │
    │── pw_loop_invoke() ───────────>│
    │                                ├── execute callback
    │                                │
    │<──────── sync/done ────────────│
    │                                │
    │── quit ───────────────────────>│
    │                                └── exit
```

**核心原则：**

> PipeWire 对象在哪个 loop/thread 上运行，就应该在哪个 context 中操作它们。

---

### ② Async / Sync 问题

你的：

```cpp
backend_.setVolume(...)
```

本质上是：

```text
main thread
    │
    └── pw_loop_invoke()
             │
             ↓
       "请求已经提交"
```

并不代表：

```text
"请求已经执行完"
```

所以 Ctrl+C 时如果直接：

```cpp
engine.shutDown();
context.quit();
loop.join();
```

可能发生：

```text
setVolume()
    ↓
请求还在 queue 中
    ↓
context.quit()
    ↓
PipeWire loop 停止
    ↓
恢复音量请求没执行 ❌
```

于是我们加入：

```cpp
engine.shutDown();
context.sync();
backend.shutdown();
context.quit();
loop.join();
```

变成：

```text
1. Queue restore requests
          ↓
2. sync()
          ↓
3. 等 PipeWire 确认之前的请求完成
          ↓
4. Destroy backend resources
          ↓
5. quit loop
          ↓
6. join thread
```

这其实就是一个很典型的 **asynchronous operations + synchronization barrier** 问题。

---

# 最终的 Shutdown 模型

你现在这套设计可以记成一句话：

> **先完成业务操作，再同步异步请求，再销毁资源，最后停止事件循环。**

```text
while (running)
       │
       ↓
 engine.process()
       │
       ↓
 Ctrl+C
       │
       ↓
engine.shutDown()
       │
       │  restore volume
       ↓
context.sync()
       │
       │  wait for PipeWire completion
       ↓
backend.shutdown()
       │
       │  destroy PipeWire resources
       ↓
context.quit()
       │
       ↓
loop.join()
```

**① Thread affinity（线程归属）**  
**② Asynchronous operation & synchronization barrier（异步操作与同步屏障）**  
**③ Resource lifetime / shutdown ordering（资源生命周期与销毁顺序）**


---

# AudioConducker 调试问题总结

## 问题 1：随机性恢复失败（Ctrl+C 后有时音量没恢复）
**原因**：竞态条件。`DuckingEngine::restore()` 通过非阻塞的 `pw_loop_invoke`（block=false）提交音量命令，但进程可能在 PipeWire 服务器**实际处理这些命令之前**就退出了。恢复是否生效完全取决于时序运气。  

**解决方案**：将 `setVolume()` 改为阻塞式（`pw_loop_invoke(..., block=true, ...)`）。控制面操作频率低，且不在实时音频回调路径上，阻塞没有问题。后续又补了 `context.sync()`，确保退出前服务器已处理完所有命令。

---

## 问题 2：`wrong context` / `loop_enter pthread_equal failed` 报错 + 程序无法退出
**原因**：跨线程误用 PipeWire API。主循环跑在独立线程上，但 `roundtrip()` 却从**主线程**调用了 `pw_main_loop_run()`——从非属主线程重入 loop，触发了内部线程检查失败。同样，`pw_main_loop_quit()` 和析构函数里的 `pw_proxy_destroy()` 也是从错误线程调用的。调用被中止后 `quit` 没能生效，`loop.join()` 永远等不到循环退出，程序卡死。  

**解决方案**：
- 从关闭路径中移除了 `roundtrip()`
- 新增 `context.sync()`：通过 `invoke` 在 loop 线程内发起 `pw_core_sync()`，主线程用条件变量等待（完全不碰 `pw_main_loop_run`）
- 新增 `backend.shutdown()`：趁 loop 线程还活着，通过阻塞 invoke 销毁所有 proxy/流/监听器

---

## 问题 3：卡死在 `sync()`（程序仍然无法退出）
**原因**：`PipeWireContext::sync()` 里两层叠加的 bug：
1. **`pw_loop_invoke` 的 `data` 参数是被 memcpy 拷贝的**。`do_sync` 把 `pending` 写进了**副本**，而 `done` 回调读的是**原件**的 `sd.pending`（还是 -1）。`seq == pending` 的判断永远不成立 → `done` 永远不会被置位 → `cv.wait()` 永久阻塞。
2. **invoke 调用时 `size` 传了 0**，实际上什么都没拷贝——`do_sync` 直接解引用了未初始化内存。这就升级成了问题 4。
还有一个潜在隐患：`sync_data` 里包含 `std::mutex`/`condition_variable`——对它们做 memcpy 是未定义行为。  

**解决方案**：改用 `user_data` 传状态（按指针原样传递，不拷贝）。`do_sync` 和 done 回调现在共享**同一个对象**。另外加了等待超时作为保险。

---

## 问题 4：`do_sync` 里的段错误（Segmentation Fault）
**原因**：`size=0` 错误的直接后果——`data` 指向未初始化的 invoke_item 存储，`sd->self` 是垃圾值，解引用 `sd->self->core_` 时崩溃。
**解决方案**：通过正确传递 `user_data` 修复（见问题 3）。教训：**当回调需要与调用方共享可变状态时，用 `user_data`（指针），绝不用 `data`（被拷贝的字节）**。

---

## 问题 5：退出时偶发段错误（数据竞争）


**原因**：`streams_`、`nodes_` 等容器由 PipeWire loop 线程写入（`onNodeAdded`/`onNodeRemoved`/`handleNodeProps`），主线程读取（`getStreams()`、`setVolume()`），**没有任何加锁**。并发访问时 `unordered_map` 正好 rehash → 迭代器悬垂 → 段错误。Ctrl+C 时会涌入大量 `global_remove` 事件，碰撞概率最高。  

**解决方案**：给共享容器加一把 `std::mutex`，两侧都遵守；另外可以把 `nodes_.find` 挪进 `do_set_volume` 的 loop 线程闭包里，避免裸 `pw_node*` 的 use-after-free 窗口。用 ASan（`-fsanitize=address`）验证。

---

## 问题 6：析构函数在 loop 线程死后运行
**原因**：`loop.join()` 之后 loop 线程已死，但 `~PipeWireContext` 仍从主线程调用 `pw_core_disconnect`/`pw_context_destroy`——同样的"wrong context"违规，在 release 构建下是未定义行为。  

**解决方案**：新增 `shutdownCore()`，**趁 loop 线程还活着**通过阻塞 invoke 完成 core 断连和 context 销毁；析构函数只做兜底。

---

## 核心教训
| 规则 | 原因 |
| --- | --- |
| 所有 PipeWire core/proxy/main-loop 调用必须在 loop 线程上执行 | 内部有线程检查，违规即未定义行为 |
| 跨线程工作 → `pw_loop_invoke(block=true)` | 控制面操作最安全的委托模式 |
| 与 invoke 回调共享状态用 `user_data`，绝不用 `data` | `data` 是 memcpy 拷贝——是副本，且对 mutex/cv 不安全 |
| 一切 PipeWire 对象必须在 quit/join **之前**销毁，不能留给析构函数 | loop 死后就没有任何安全线程能销毁它们了 |
| 调试 Ctrl+C 路径需要 `gdb handle SIGINT nostop pass` | 否则 gdb 会吞掉信号，你调试的是错误的时刻 |
