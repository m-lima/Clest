cmake_minimum_required(VERSION 3.2)
project(Clest C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Current state
if (WIN32)
  message(STATUS "WIN32")
endif()
if (MSVC)
  message(STATUS "MSVC")
endif()
if (APPLE)
  message(STATUS "APPLE")
endif()

message(STATUS "Using C: " ${CMAKE_C_COMPILER_ID})
message(STATUS "Using CXX: " ${CMAKE_CXX_COMPILER_ID})

# Set high warning levels
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# Make $HOME compatible with windows
STRING(REGEX REPLACE "\\\\" "/" ENV_HOME_DIR $ENV{HOME})

# Add custom modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/findcmake")

# Set base directories
set(CPP_SRC_DIR "src/cpp")
set(OPENCL_SRC_DIR "src/opencl")

# Add manual libs
list (APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/include")

# Help FIND_PACKAGE
if(WIN32)

  # Boost
  message(STATUS "Setting BOOST_ROOT: " "${ENV_HOME_DIR}/Lib/Boost/boost_1_62_0")
  set(BOOST_ROOT "${ENV_HOME_DIR}/Lib/Boost/boost_1_62_0")

  # CGAL
  message(STATUS "Setting CGAL_DIR: " "${ENV_HOME_DIR}/Lib/CGAL/CGAL-4.9/build")
  set(CGAL_DIR "${ENV_HOME_DIR}/Lib/CGAL/CGAL-4.9/build")

  # TBB
  message(STATUS "Setting ENV{TBB_ARCH_PLATFORM}: " "intel64/vc14")
  message(STATUS "Setting TBB_INSTALL_DIR: " "${ENV_HOME_DIR}/Lib/TBB/Current")
  set(ENV{TBB_ARCH_PLATFORM} "intel64/vc14")
  set(TBB_INSTALL_DIR "${ENV_HOME_DIR}/Lib/TBB/Current")

else()
  message(STATUS "Setting BOOST_ROOT: " "/usr/Local/Cellar/boost/1.61.0_1")
  set(BOOST_ROOT "/usr/Local/Cellar/boost/1.61.0_1")
endif()

##------------------------------------------------------------------------------
## Dependencies
##

add_subdirectory(fmt)
list(APPEND INCLUDE_DIRS fmt)
list(APPEND LIBRARIES fmt)

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
message(STATUS "OpenCL Include: " "${OpenCL_INCLUDE_DIR}")
message(STATUS "OpenCL Libraries: " "${OpenCL_LIBRARIES}")

find_package(CGAL COMPONENTS CORE)
if (CGAL_FOUND)
  include(${CGAL_USE_FILE})
  list(APPEND INCLUDE_DIRS ${CGAL_INCLUDE_DIRS})
  list(APPEND LIBRARIES ${CGAL_LIBRARIES})
  message(STATUS "CGAL Include: " "${CGAL_INCLUDE_DIRS}")
  message(STATUS "CGAL Libraries: " "${CGAL_LIBRARIES}")
else()
  MESSAGE(STATUS "Skipping CGAL..")
endif()

# set(Boost_USE_STATIC_LIBS ON)
find_package(Boost OPTIONAL_COMPONENTS date_time)
if (NOT Boost_FOUND)
  MESSAGE(STATUS "Skipping BOOST Components and trying header-only..")
  find_package(Boost)
endif()
if (Boost_FOUND)
  list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
  list(APPEND LIBRARIES ${Boost_LIBRARIES})
  message(STATUS "Boost Include: " "${Boost_INCLUDE_DIRS}")
  message(STATUS "Boost Libraries: " "${Boost_LIBRARIES}")
else()
  MESSAGE(STATUS "Skipping BOOST..")
endif()

find_package(TBB)
if (TBB_FOUND)
  if (CGAL_FOUND)
    include(${TBB_USE_FILE})
  endif()
  list(APPEND INCLUDE_DIRS ${TBB_INCLUDE_DIRS})
  list(APPEND LIBRARIES ${TBB_LIBRARIES})
  message(STATUS "TBB Include: " "${TBB_INCLUDE_DIRS}")
  message(STATUS "TBB Libraries: " "${TBB_LIBRARIES}")
else()
  MESSAGE(STATUS "Skipping TBB..")
endif()

##------------------------------------------------------------------------------
## Sources
##

set(SOURCES
  ${CPP_SRC_DIR}/main.cpp
  ${CPP_SRC_DIR}/mesher.cpp
  )

set(HEADER
  ${CPP_SRC_DIR}/mesher.hpp
  )

set(START_SRC
  ${CPP_SRC_DIR}/operator.cpp
  ${CPP_SRC_DIR}/grider.cpp
  ${CPP_SRC_DIR}/fractal.cpp
  )

set(LAS_SRC
  ${CPP_SRC_DIR}/las/las_file.cpp
  ${CPP_SRC_DIR}/las/grid_file.cpp
  ${CPP_SRC_DIR}/las/las_operations.cpp
  )

set(LAS_HPP
  ${CPP_SRC_DIR}/las/public_header.hpp
  ${CPP_SRC_DIR}/las/record_header.hpp
  ${CPP_SRC_DIR}/las/point_data.hpp
  ${CPP_SRC_DIR}/las/las_file.hpp
  ${CPP_SRC_DIR}/las/grid_file.hpp
  ${CPP_SRC_DIR}/las/las_operations.hpp
  ${CPP_SRC_DIR}/las/wlop_simplify_verbose.hpp
  )

set(LIB_HPP
  ${CPP_SRC_DIR}/lib/clest/ostream.hpp
  ${CPP_SRC_DIR}/lib/clest/util.hpp
  ${CPP_SRC_DIR}/lib/clest/cl.hpp
  )

set(LEWINER_SRC
  ${CPP_SRC_DIR}/las/cube_marcher.cpp
  ${CPP_SRC_DIR}/lewiner/MarchingCubes.cpp
  ${CPP_SRC_DIR}/lewiner/ply.c
  )

set(LEWINER_HPP
  ${CPP_SRC_DIR}/las/cube_marcher.hpp
  ${CPP_SRC_DIR}/lewiner/MarchingCubes.h
  ${CPP_SRC_DIR}/lewiner/ply.h
  ${CPP_SRC_DIR}/lewiner/LookUpTable.h
  )

set(CL_SRC
  ${CPP_SRC_DIR}/cl/cl_runner.cpp
  )

set(CL_HPP
  ${CPP_SRC_DIR}/cl/cl_runner.hpp
  ${CPP_SRC_DIR}/cl/clest_program.hpp
  ${CPP_SRC_DIR}/cl/grid_program.hpp
  ${CPP_SRC_DIR}/cl/marching_program.hpp
  )

set(OPENCL_SRC
  ${OPENCL_SRC_DIR}/fractal.cl
  ${OPENCL_SRC_DIR}/marching.cl
  ${OPENCL_SRC_DIR}/grider.cl
  ${OPENCL_SRC_DIR}/vertex.cl
  )

list(APPEND SOURCES ${LAS_SRC})
list(APPEND SOURCES ${LEWINER_SRC})
list(APPEND SOURCES ${CL_SRC})

if(MSVC)
  # For MSVC, all files that are going to be visible in the IDE
  # should be added to the source
  list(APPEND SOURCES ${HEADERS})
  list(APPEND SOURCES ${START_SRC})
  list(APPEND SOURCES ${LAS_HPP})
  list(APPEND SOURCES ${LIB_HPP})
  list(APPEND SOURCES ${LEWINER_HPP})
  list(APPEND SOURCES ${CL_HPP})
  list(APPEND SOURCES ${OPENCL_SRC})

  # Tell CMake to, even though these files are being added to the source,
  # they should not be compiled
  set_source_files_properties(${START_SRC} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${LAS_HPP} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${LIB_HPP} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${LEWINER_HPP} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${CL_HPP} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${OPENCL_SRC} PROPERTIES HEADER_FILE_ONLY true)
endif()

##------------------------------------------------------------------------------
## OpenCL
##

add_custom_target(always ALL DEPENDS copy_opencl_sources)
add_custom_command(
  OUTPUT copy_opencl_sources
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/opencl
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${OPENCL_SRC_DIR} ${CMAKE_BINARY_DIR}/opencl
  # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lib/bin ${CMAKE_BINARY_DIR}
  DEPENDS always_execute
  )
add_custom_command(OUTPUT always_execute COMMAND ${CMAKE_COMMAND} -E echo)

##------------------------------------------------------------------------------
## Targets
##

# Create project
add_executable(clest ${SOURCES})

# Link with FIND_PACKAGE
target_link_libraries(clest ${LIBRARIES})

# Add manual includes
target_include_directories(clest PRIVATE ${INCLUDE_DIRS})

# Specify a global access to the lib directory
target_include_directories(clest PRIVATE ${CPP_SRC_DIR}/lib)

# Perform the always-execute actions
add_dependencies(clest always)

# Cascade to C++ CMake's variables
if (CGAL_FOUND)
  add_definitions(-D_CMAKE_CGAL_FOUND)
endif()
if (Boost_FOUND)
  add_definitions(-D_CMAKE_BOOST_FOUND)
endif()
if (TBB_FOUND)
  add_definitions(-D_CMAKE_TBB_FOUND)
endif()

##------------------------------------------------------------------------------
## MSVC Specifics
##

if(MSVC)

  # Folder organization within MSVC
  source_group("Source Files\\Las" FILES ${LAS_SRC})
  source_group("Header Files\\Las" FILES ${LAS_HPP})
  source_group("Source Files\\Lewiner" FILES ${LEWINER_SRC})
  source_group("Header Files\\Lewiner" FILES ${LEWINER_HPP})
  source_group("Source Files\\OpenCL" FILES ${CL_SRC})
  source_group("Header Files\\OpenCL" FILES ${CL_HPP})
  source_group("Header Files\\Libs" FILES ${LIB_HPP})
  source_group("OpenCL" FILES ${OPENCL_SRC})

  # Choose the main project
  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT clest)
  endif()
endif()
