cmake_minimum_required(VERSION 3.10)


include_directories(../.)
include_directories(./.)

#include_directories(${CMAKE_SOURCE_DIR} . ../ )

# Files common to all tests
set (COMMON_SOURCES test_common.h ../tp/threadsafe_queue.h ../tp/threadpool.h)

set(TEST_BASIC test_basic)
add_executable(${TEST_BASIC} test_basic.cpp ${COMMON_SOURCES})

set(TEST_AFFINITY test_affinity)
add_executable(${TEST_AFFINITY} test_affinity.cpp ${COMMON_SOURCES})

set(TEST_ORDERED test_ordered)
add_executable(${TEST_ORDERED} test_ordered.cpp ${COMMON_SOURCES})

set(TEST_FUTURE test_future)
add_executable(${TEST_FUTURE} test_future.cpp ${COMMON_SOURCES})

set(TEST_INTERFACE test_interface)
add_executable(${TEST_INTERFACE} test_interface.cpp ${COMMON_SOURCES})

set(TEST_RACECOND test_raceCond)
add_executable(${TEST_RACECOND} test_raceCond.cpp ${COMMON_SOURCES})


set(exes ${TEST_BASIC} ${TEST_AFFINITY} ${TEST_ORDERED} ${TEST_FUTURE} ${TEST_INTERFACE} ${TEST_RACECOND})

if (UNIX)
foreach (exe IN LISTS exes)
	target_link_libraries(${exe} pthread)
endforeach()
endif()

foreach (exe IN LISTS exes)
	add_test(${exe} ${exe})
endforeach()
