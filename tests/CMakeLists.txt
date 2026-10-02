include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(PipeWireBackendTests PipeWireBackendTests.cpp)
target_link_libraries(PipeWireBackendTests PRIVATE AudioConduckerCore GTest::gtest_main)

# add_executable(AudioConduckerIntegrationTests test_main.cpp)
# target_link_libraries(AudioConduckerIntegrationTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(ActivityDetectorTests ActivityDetectorTests.cpp)
target_link_libraries(ActivityDetectorTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(ConfigManagerTests ConfigManagerTests.cpp)
target_link_libraries(ConfigManagerTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(DuckingEngineTests DuckingEngineTests.cpp)
target_link_libraries(DuckingEngineTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(VolumeSmootherTests VolumeSmootherTests.cpp)
target_link_libraries(VolumeSmootherTests PRIVATE AudioConduckerCore GTest::gtest_main)



include(GoogleTest)

set(TEST_TARGETS
    ActivityDetectorTests
    ConfigManagerTests
    DuckingEngineTests
    VolumeSmootherTests
    PipeWireBackendTests
)

# foreach loop, enable ctest --test-dir build/ can find these files
foreach(t ${TEST_TARGETS})
        gtest_discover_tests(${t})
endforeach()
