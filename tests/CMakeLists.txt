cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_SOURCE_DIR} ../ )

# Files common to all tests
set (COMMON_SOURCES test_common.h ../threadsafe_queue.h ../threadpool.h ../threadpool.cpp)

set(TEST_BASIC test_basic)
add_executable(${TEST_BASIC} test_basic.cpp ${COMMON_SOURCES})

set(TEST_AFFINITY test_affinity)
add_executable(${TEST_AFFINITY} test_affinity.cpp ${COMMON_SOURCES})

if (UNIX)
target_link_libraries(${TEST_BASIC} pthread)
target_link_libraries(${TEST_AFFINITY} pthread)
endif()

add_test(${TEST_BASIC} ${TEST_BASIC})
add_test(${TEST_AFFINITY} ${TEST_AFFINITY})

