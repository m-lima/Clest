cmake_minimum_required(VERSION 3.2)
project(Clest CXX)

if(APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
endif()

set(CPP_SRC_DIR "src/cpp")
set(OPENCL_SRC_DIR "src/opencl")

list (APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/include")

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

add_subdirectory(fmt)
list(APPEND INCLUDE_DIRS fmt)
list(APPEND LIBRARIES fmt)

# add_subdirectory(laslib)
# list(APPEND INCLUDE_DIRS laslib)
# list(APPEND LIBRARIES laslib)

##------------------------------------------------------------------------------
## LAS
##
# list (APPEND LIBRARIES "${CMAKE_SOURCE_DIR}/lib/bin/LASlib.lib")
# list (APPEND LIBRARIES "${CMAKE_SOURCE_DIR}/lib/bin/LASzip.dll")

##------------------------------------------------------------------------------
## Sources
##

set(SOURCES
  ${CPP_SRC_DIR}/main.cpp
  ${CPP_SRC_DIR}/util.hpp
  ${CPP_SRC_DIR}/las/PublicHeader.hpp
  ${CPP_SRC_DIR}/las/RecordHeader.hpp
  ${CPP_SRC_DIR}/las/PointData.hpp
  ${CPP_SRC_DIR}/las/LASFile.hpp
  ${CPP_SRC_DIR}/las/LASFile.cpp
  )

set(OPENCL_SRC
  ${OPENCL_SRC_DIR}/fractal.cl
  ${OPENCL_SRC_DIR}/lop.cl
  )

if(MSVC)
  list(APPEND SOURCES ${OPENCL_SRC})
endif()

##------------------------------------------------------------------------------
## OpenCL
##

add_custom_target(always ALL DEPENDS copy_opencl_sources)
add_custom_command(
  OUTPUT copy_opencl_sources
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/opencl
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${OPENCL_SRC_DIR} ${CMAKE_BINARY_DIR}/opencl
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lib/bin ${CMAKE_BINARY_DIR}
  DEPENDS always_execute
  )
add_custom_command(OUTPUT always_execute COMMAND ${CMAKE_COMMAND} -E echo)

##------------------------------------------------------------------------------
## Targets
##

add_executable(clest ${SOURCES})
set_target_properties(clest PROPERTIES CXX_LANGUAGE_STANDARD 11)
target_link_libraries(clest ${LIBRARIES})
target_include_directories(clest PRIVATE ${INCLUDE_DIRS})
set_target_properties(clest PROPERTIES CXX_LANGUAGE_STANDARD 11)
add_dependencies(clest always)

##------------------------------------------------------------------------------
## MSVC Specifics
##

if(MSVC)
  source_group("OpenCL" FILES ${OPENCL_SRC})

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT clest)
  endif()
endif()
