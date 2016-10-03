cmake_minimum_required(VERSION 3.2)
project(CLEST CXX)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/findcmake")
list (APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/include")
list (APPEND LIBRARIES "${CMAKE_SOURCE_DIR}/lib/bin")

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OPENCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OPENCL_LIBRARIES})
message(STATUS "${OPENCL_INCLUDE_DIR}")
message(STATUS "${OPENCL_LIBRARIES}")

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

