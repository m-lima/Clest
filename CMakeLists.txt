cmake_minimum_required(VERSION 3.2)
project(Clest CXX)

# Handle Mac's Clang
if(APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
endif()

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
  set(BOOST_ROOT "${ENV_HOME_DIR}/Lib/Boost/boost_1_62_0")

  # CGAL
  set(CGAL_DIR "${ENV_HOME_DIR}/Lib/CGAL/CGAL-4.9/build/msvc")

  # TBB
  set(ENV{TBB_ARCH_PLATFORM} "intel64/vc14")
  set(TBB_INSTALL_DIR "${ENV_HOME_DIR}/Lib/TBB/tbb2017_20160916oss")

else()
  # set(BOOST_ROOT "/usr/Local/Cellar/boost/1.61.0_1")
endif()

##------------------------------------------------------------------------------
## Dependencies
##

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
message(STATUS "OpenCL Include: " "${OpenCL_INCLUDE_DIR}")
message(STATUS "OpenCL Libraries: " "${OpenCL_LIBRARIES}")

find_package(CGAL REQUIRED COMPONENTS CORE)
include(${CGAL_USE_FILE})
list(APPEND INCLUDE_DIRS ${CGAL_INCLUDE_DIRS})
list(APPEND LIBRARIES ${CGAL_LIBRARIES})
message(STATUS "CGAL Include: " "${CGAL_INCLUDE_DIRS}")
message(STATUS "CGAL Libraries: " "${CGAL_LIBRARIES}")

#set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost 1.62 REQUIRED)
find_package(Boost 1.62 REQUIRED COMPONENTS date_time)
list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
list(APPEND LIBRARIES ${Boost_LIBRARIES})
message(STATUS "Boost Include: " "${Boost_INCLUDE_DIRS}")
message(STATUS "Boost Libraries: " "${Boost_LIBRARIES}")

add_subdirectory(fmt)
list(APPEND INCLUDE_DIRS fmt)
list(APPEND LIBRARIES fmt)

find_package(TBB REQUIRED)
include(${TBB_USE_FILE})
# include_directories ( ${TBB_INCLUDE_DIRS} )
# link_directories( ${TBB_LIBRARY_DIRS} )
# add_definitions( -DNOMINMAX -DCGAL_LINKED_WITH_TBB )
list(APPEND INCLUDE_DIRS ${TBB_INCLUDE_DIRS})
list(APPEND LIBRARIES ${TBB_LIBRARIES})
message(STATUS "TBB Include: " "${TBB_INCLUDE_DIRS}")
message(STATUS "TBB Libraries: " "${TBB_LIBRARIES}")

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
  # ${CPP_SRC_DIR}/main.cpp
  ${CPP_SRC_DIR}/fractal.cpp
  ${CPP_SRC_DIR}/util.hpp
  ${CPP_SRC_DIR}/las/PublicHeader.hpp
  ${CPP_SRC_DIR}/las/RecordHeader.hpp
  ${CPP_SRC_DIR}/las/PointData.hpp
  ${CPP_SRC_DIR}/las/LASFile.hpp
  ${CPP_SRC_DIR}/las/LASFile.cpp
  ${CPP_SRC_DIR}/las/Operations.hpp
  ${CPP_SRC_DIR}/las/Operations.cpp
  ${CPP_SRC_DIR}/las/WlopSimplifyVerbose.hpp
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
