cmake_minimum_required(VERSION 3.0)

project(just-parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  # mingw
  message("Static windows format")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

# stand up test
enable_testing()

include_directories(include)

# find sources for compiling
file(GLOB TARGET_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/justparser"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(justio SHARED ${TARGET_SOURCES})

# target include <...>
target_include_directories(justio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# set add test
add_subdirectory(just-test/)
