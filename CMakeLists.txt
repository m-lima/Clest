cmake_minimum_required(VERSION 3.2)
project(CLEST CXX)

message("Building for " ${CMAKE_SYSTEM_NAME} " using " ${CMAKE_TOOLCHAIN_FILE})

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
  src/cpp/main.cpp
  src/cpp/util.hpp
  )

set(OPENCL_SRC
  src/opencl/main.cl
  )

if(MSVC)
  list(APPEND SOURCES ${OPENCL_SRC})
endif()

##------------------------------------------------------------------------------
## Targets
##

add_executable(clest ${SOURCES})
target_link_libraries(clest ${LIBRARIES})
target_include_directories(clest PRIVATE ${INCLUDE_DIRS})
set_target_properties(clest PROPERTIES CXX_LANGUAGE_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

##------------------------------------------------------------------------------
## MSVC Specifics
##

if(MSVC)
  source_group("OpenCL" FILES ${OPENCL_SRC})

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT clest)
  endif()
endif()
