cmake_minimum_required(VERSION 3.2)
project(CLEST CXX)

list (APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/include")
# list (APPEND LIBRARIES "${CMAKE_SOURCE_DIR}/lib/bin")

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OpenCL_LIBRARY})
message(STATUS "OpenCL Include: " "${OpenCL_INCLUDE_DIR}")
message(STATUS "OpenCL Library: " "${OpenCL_LIBRARY}")

##------------------------------------------------------------------------------
## Sources
##

set(SOURCES
  src/main.cpp)

##------------------------------------------------------------------------------
## Targets
##

add_executable(clest ${SOURCES})
target_link_libraries(clest ${LIBRARIES})
target_include_directories(clest PRIVATE ${INCLUDE_DIRS})
set_target_properties(clest PROPERTIES CXX_LANGUAGE_STANDARD 11)

