# Command Line

### Show help

```bash
./AudioConducker -H
./AudioConducker --help
```

### Select a focus application

```bash
./AudioConducker -f Firefox
./AudioConducker --focus Firefox
```

### Set duck amount

```bash
./AudioConducker -d 60
./AudioConducker --duck 60
```

`--duck 60` means background streams are reduced to approximately 40% of their original volume.

### Set attack/release/hold time

```bash
./AudioConducker -a 30 -r 500 -h 400
./AudioConducker --attack 30 --release 500 -hold 400
```

You can also use them separately

### Set logging level

```bash
./AudioConducker -l debug
./AudioConducker --log debug
```

Supported levels:

- trace
- debug
- info
- warn (default)
- error


### Show available nodes

```bash
./AudioConducker -n
./AudioConducker --nodes
```

### Show version

```bash
./AudioConducker -v
./AudioConducker --version
```

### Show dynamic nodes addtion/deletion

```bash
./AudioConducker -w
./AudioConducker --watch-nodes
```

### Example:

```bash
./AudioConducker --focus Firefox  --duck 80  --log info --nodes
```


