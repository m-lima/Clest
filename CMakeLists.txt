cmake_minimum_required(VERSION 3.2)
project(Clest CXX)

# Handle Mac's Clang
if(APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
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
  MESSAGE(STATUS "Setting BOOST_ROOT: " "${ENV_HOME_DIR}/Lib/Boost/boost_1_62_0")
  set(BOOST_ROOT "${ENV_HOME_DIR}/Lib/Boost/boost_1_62_0")

  # CGAL
  MESSAGE(STATUS "Setting CGAL_DIR: " "${ENV_HOME_DIR}/Lib/CGAL/CGAL-4.9/build")
  set(CGAL_DIR "${ENV_HOME_DIR}/Lib/CGAL/CGAL-4.9/build")

  # TBB
  MESSAGE(STATUS "Setting ENV{TBB_ARCH_PLATFORM}: " "intel64/vc14")
  MESSAGE(STATUS "Setting TBB_INSTALL_DIR: " "${ENV_HOME_DIR}/Lib/TBB/Current")
  set(ENV{TBB_ARCH_PLATFORM} "intel64/vc14")
  set(TBB_INSTALL_DIR "${ENV_HOME_DIR}/Lib/TBB/Current")

else()
  # set(BOOST_ROOT "/usr/Local/Cellar/boost/1.61.0_1")
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
find_package(Boost 1.62 OPTIONAL_COMPONENTS date_time)
if (NOT Boost_FOUND)
  MESSAGE(STATUS "Skipping BOOST Components and trying header-only..")
  find_package(Boost 1.62)
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
  )

set(START_SRC
  ${CPP_SRC_DIR}/operator.cpp
  ${CPP_SRC_DIR}/grider.cpp
  ${CPP_SRC_DIR}/fractal.cpp
  )

set(LAS_CPP
  ${CPP_SRC_DIR}/las/las_file.cpp
  ${CPP_SRC_DIR}/las/grid_file.cpp
  ${CPP_SRC_DIR}/las/operations.cpp
  )
list(APPEND SOURCES ${LAS_CPP})

set(LAS_HPP
  ${CPP_SRC_DIR}/las/public_header.hpp
  ${CPP_SRC_DIR}/las/record_header.hpp
  ${CPP_SRC_DIR}/las/point_data.hpp
  ${CPP_SRC_DIR}/las/las_file.hpp
  ${CPP_SRC_DIR}/las/grid_file.hpp
  ${CPP_SRC_DIR}/las/operations.hpp
  ${CPP_SRC_DIR}/las/wlop_simplify_verbose.hpp
  )

set(LIB_HPP
  ${CPP_SRC_DIR}/lib/clest/ostream.hpp
  ${CPP_SRC_DIR}/lib/clest/util.hpp
  ${CPP_SRC_DIR}/lib/clest/cl.hpp
  ${CPP_SRC_DIR}/lib/clest/las.hpp
  )

set(OPENCL_SRC
  ${OPENCL_SRC_DIR}/fractal.cl
  ${OPENCL_SRC_DIR}/lop.cl
  )

if(MSVC)
  # For MSVC, all files that are going to be visible in the IDE
  # should be added to the source
  list(APPEND SOURCES ${START_SRC})
  list(APPEND SOURCES ${LAS_HPP})
  list(APPEND SOURCES ${LIB_HPP})
  list(APPEND SOURCES ${OPENCL_SRC})

  # Tell CMake to, even though these files are being added to the source,
  # they should not be compiled
  set_source_files_properties(${START_SRC} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${LAS_HPP} PROPERTIES HEADER_FILE_ONLY true)
  set_source_files_properties(${LIB_HPP} PROPERTIES HEADER_FILE_ONLY true)
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
set_target_properties(clest PROPERTIES CXX_LANGUAGE_STANDARD 11)

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
  source_group("Source Files\\Las" FILES ${LAS_CPP})
  source_group("Header Files\\Las" FILES ${LAS_HPP})
  source_group("Header Files\\Libs" FILES ${LIB_HPP})
  source_group("OpenCL" FILES ${OPENCL_SRC})

  # Choose the main project
  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT clest)
  endif()
endif()
