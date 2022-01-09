cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_SOURCE_DIR} ../ )

# Files common to all tests
set (COMMON_SOURCES test_common.h ../threadsafe_queue.h ../threadpool.h ../threadpool.cpp)

set(TEST_BASIC test_basic)
add_executable(${TEST_BASIC} test_basic.cpp ${COMMON_SOURCES})

set(TEST_AFFINITY test_affinity)
add_executable(${TEST_AFFINITY} test_affinity.cpp ${COMMON_SOURCES})

set(TEST_ORDERED test_ordered)
add_executable(${TEST_ORDERED} test_ordered.cpp ${COMMON_SOURCES})

set(TEST_FUTURE test_future)
add_executable(${TEST_FUTURE} test_future.cpp ${COMMON_SOURCES})

set(exes ${TEST_BASIC} ${TEST_AFFINITY} ${TEST_ORDERED} ${TEST_FUTURE})

if (UNIX)
foreach (exe IN LISTS exes)
	target_link_libraries(${exe} pthread)
endforeach()
endif()

foreach (exe IN LISTS exes)
	add_test(${exe} ${exe})
endforeach()
