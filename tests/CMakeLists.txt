cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_SOURCE_DIR} ../ )

# Files common to all tests
set (COMMON_SOURCES ../threadsafe_queue.h ../threadpool.h ../threadpool.cpp)

# Library setup/shutdown testing
set(TEST_BASIC test_basic)
add_executable(${TEST_BASIC} test_basic.cpp ${COMMON_SOURCES})

if (UNIX)
target_link_libraries(${TEST_BASIC} pthread)
endif()

add_test(${TEST_BASIC} ${TEST_BASIC})

