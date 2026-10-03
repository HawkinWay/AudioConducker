include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(ActivityDetectorTests ActivityDetectorTests.cpp)
target_link_libraries(ActivityDetectorTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(ConfigManagerTests ConfigManagerTests.cpp)
target_link_libraries(ConfigManagerTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(DuckingEngineTests DuckingEngineTests.cpp)
target_link_libraries(DuckingEngineTests PRIVATE AudioConduckerCore GTest::gtest_main)

add_executable(VolumeSmootherTests VolumeSmootherTests.cpp)
target_link_libraries(VolumeSmootherTests PRIVATE AudioConduckerCore GTest::gtest_main)


set(TEST_TARGETS
    ActivityDetectorTests
    ConfigManagerTests
    DuckingEngineTests
    VolumeSmootherTests
)

if(BUILD_PIPEWIRE_TESTS)
        add_executable(PipeWireBackendTests PipeWireBackendTests.cpp)
        target_link_libraries(PipeWireBackendTests PRIVATE AudioConduckerCore GTest::gtest_main)

        list(APPEND TEST_TARGETS PipeWireBackendTests)
endif()

include(GoogleTest)

# foreach loop, enable ctest --test-dir build/ can find these files
foreach(t ${TEST_TARGETS})
        gtest_discover_tests(${t})
endforeach()
