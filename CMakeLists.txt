cmake_minimum_required(VERSION 3.10)

set (PROJECT_NAME "threadpool")
set (EXE_NAME "app")

# set the project name
project(${PROJECT_NAME})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (SOURCES main.cpp tp/threadsafe_queue.h tp/threadpool.h)

# add the executable
add_executable(${EXE_NAME} ${SOURCES})

if (UNIX)
target_link_libraries(${EXE_NAME} pthread)
endif()

enable_testing()
add_subdirectory(tests)

