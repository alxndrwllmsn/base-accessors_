cmake_minimum_required (VERSION 3.1.0)

project(base-accessors VERSION 0.0.1 LANGUAGES C CXX)


if (EXISTS ${CMAKE_SOURCE_DIR}/askap-cmake)
    execute_process(COMMAND git pull 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/askap-cmake
	TIMEOUT 5)
else ()	
    execute_process(COMMAND git clone https://bitbucket.csiro.au/scm/askapsdp/askap-cmake.git
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	TIMEOUT 5)
endif ()
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/askap-cmake")

option (CXX11 "Compile as C++11 if possible" YES)
option (ENABLE_SHARED "Build shared libraries" YES)
option (ENABLE_RPATH "Include rpath in executables and shared libraries" YES)

# Yanda Packages
# YandaSoft dependencies .. in order
include(install_yanda_depends)
find_package(LofarCommon)

set (EXTERNAL_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

if (NOT LofarCommon_FOUND)
	install_yanda_depends(lofar-common)
	find_package(LofarCommon REQUIRED)
endif ()

find_package(LofarBlob)
if (NOT LofarBlob_FOUND)
	install_yanda_depends(lofar-blob)
	find_package(LofarBlob REQUIRED)
endif ()

find_package(Askap)
if (NOT ASKAP_FOUND)
	install_yanda_depends(base-askap)
	find_package(Askap REQUIRED)
endif ()

find_package(Scimath)
if (NOT SCIMATH_FOUND)
	install_yanda_depends(base-scimath)
	find_package(Scimath REQUIRED)
endif ()

find_package(AskapParallel)
if (NOT ASKAPPARALLEL_FOUND)
	install_yanda_depends(base-askapparallel)
	find_package(AskapParallel REQUIRED)
endif ()

# find packages
find_package(Boost REQUIRED)
find_package(log4cxx REQUIRED)
find_package(Casacore REQUIRED COMPONENTS  ms images mirlib coordinates fits lattices measures scimath scimath_f tables casa)
find_package(XercesC REQUIRED)
find_package(CPPUnit)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

if (ENABLE_SHARED)
option (BUILD_SHARED_LIBS "" YES)
    if (ENABLE_RPATH)
        # Set RPATH to use for installed targets; append linker search path
        set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" )
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    endif (ENABLE_RPATH)
endif(ENABLE_SHARED)



# include directories
include_directories (${Boost_INCLUDE_DIRS})
add_definitions(-DHAVE_BOOST)
include_directories(${log4cxx_INCLUDE_DIRS})
add_definitions(-DHAVE_LOG4CXX)
include_directories(${CASACORE_INCLUDE_DIRS})
add_definitions(-DHAVE_AIPSPP)
add_definitions(-Dcasa=casacore)
include_directories(${XercesC_INCLUDE_DIR})

if (CPPUNIT_FOUND)
    include_directories(${CPPUNIT_INCLUDE_DIR})
endif ()

if (CASACORE3 OR CXX11)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


add_subdirectory(askap/imageaccess)
add_subdirectory(askap/dataaccess)
add_subdirectory(askap/calibaccess)
add_subdirectory(askap/votable)

add_library(askap_accessors SHARED
	$<TARGET_OBJECTS:imageaccess>
	$<TARGET_OBJECTS:dataaccess>
	$<TARGET_OBJECTS:calibaccess>
	$<TARGET_OBJECTS:votable>
)
# add some more tests and sub-directories

target_link_libraries(askap_accessors
	lofar::Blob
	lofar::Common
	askap::parallel
	askap::scimath
	askap::askap
	${CASACORE_LIBRARIES}
	${log4cxx_LIBRARY}
	${XercesC_LIBRARY}
	)
    
if (CPPUNIT_FOUND)
    target_link_libraries(askap_accessors
        ${CPPUNIT_LIBRARY})
endif () 

target_include_directories(askap_accessors PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/askap>
	$<INSTALL_INTERFACE:include>
)

add_library(askap::accessors ALIAS askap_accessors)

install (
	TARGETS askap_accessors
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	LIBRARY PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install (FILES
	askap_accessors.h
	DESTINATION include/askap/
	)
if (CPPUNIT_FOUND)
  include(CTest)
  enable_testing()

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/calibaccess)
  add_subdirectory(tests/calibaccess)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/dataaccess)
  add_subdirectory(tests/dataaccess)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/votable)
  add_subdirectory(tests/votable)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/timageaccess)
  add_subdirectory(tests/imageaccess)


endif (CPPUNIT_FOUND)


