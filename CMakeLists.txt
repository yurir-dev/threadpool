cmake_minimum_required(VERSION 3.10)

set (PROJECT_NAME "threadpool")
set (EXE_NAME "app")

# set the project name
project(${PROJECT_NAME})

if(MSVC)
    add_compile_options("/W4" "/WX" "$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()

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

