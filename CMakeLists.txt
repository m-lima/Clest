cmake_minimum_required(VERSION 3.2)
project(CLEST CXX)

list (APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/include")
# list (APPEND LIBRARIES "${CMAKE_SOURCE_DIR}/lib/bin")
if(WIN32)
  set(BOOST_ROOT "C:/Program Files/Boost/boost_1_62_0")
else()
  set(BOOST_ROOT "/usr/Local/Cellar/boost/1.61.0_1")
endif()

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
message(STATUS "OpenCL Include: " "${OpenCL_INCLUDE_DIR}")
message(STATUS "OpenCL Libraries: " "${OpenCL_LIBRARIES}")

find_package(Boost REQUIRED)
list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIR})
list(APPEND LIBRARIES ${Boost_LIBRARY_DIR})
message(STATUS "Boost Include: " "${Boost_INCLUDE_DIRS}")
message(STATUS "Boost Libraries: " "${Boost_LIBRARIES}")

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
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
