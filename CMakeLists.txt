cmake_minimum_required (VERSION 3.12.0)
option(UPDATE_CMAKE "Update the cmake modules" ON)
 
if (UPDATE_CMAKE)
# Reuse common cmake infrastructure


if (EXISTS ${CMAKE_SOURCE_DIR}/askap-cmake)
    execute_process(COMMAND git pull 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/askap-cmake
	TIMEOUT 5)
else ()	
    execute_process(COMMAND git clone https://bitbucket.csiro.au/scm/askapsdp/askap-cmake.git
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	TIMEOUT 5)
endif ()

endif ()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/askap-cmake")
include(version_utils)
get_version_string()
if (VERSION_SHORT)
    project(askap-accessors VERSION ${VERSION_SHORT} LANGUAGES C CXX)
else ()
    project(askap-accessors VERSION 0.0.0 LANGUAGES C CXX)
endif ()


configure_file(askap_accessors.cc.in ${CMAKE_CURRENT_BINARY_DIR}/askap_accessors.cc)

# Required Version of ASKAP dependencies is the MAJOR and MINOR version
# of this package. This allows the PATCH versions to change on everything
# but the version number cannot be less or more than the current version
# 
set(REQUIRED_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")


option (CXX11 "Compile as C++11 if possible" YES)
option (ENABLE_SHARED "Build shared libraries" YES)
option (ENABLE_RPATH "Include rpath in executables and shared libraries" YES)
option (ENABLE_OPENMP "Build with OPENMP Support" NO)

# find packages
find_package(lofar-common REQUIRED)
find_package(lofar-blob REQUIRED)
find_package(askap-askap ${REQUIRED_VERSION} REQUIRED)
find_package(askap-scimath ${REQUIRED_VERSION} REQUIRED)
find_package(askap-parallel ${REQUIRED_VERSION} REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread chrono)
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

if (ENABLE_OPENMP)
	find_package(OpenMP)
    if (OPENMP_FOUND)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qopenmp")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopenmp")
        else( )
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        endif( )
    else (OPENMP_FOUND)
        message(WARNING "Cannot fullfill USE_OPENMP, compiler does not support it")
    endif (OPENMP_FOUND)
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

if (CASACORE3 OR CXX11)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


add_library(accessors SHARED
	${CMAKE_CURRENT_BINARY_DIR}/askap_accessors.cc
)

macro(add_sources_to_accessors)
	foreach(arg IN ITEMS ${ARGN})
		target_sources(accessors PRIVATE ${CMAKE_CURRENT_LIST_DIR}/${arg})
	endforeach()
endmacro()

add_subdirectory(askap/imageaccess)
add_subdirectory(askap/dataaccess)
add_subdirectory(askap/calibaccess)
add_subdirectory(askap/votable)
add_subdirectory(apps)

target_compile_definitions(accessors PUBLIC
	casa=casacore
	HAVE_AIPSPP
	HAVE_BOOST
	HAVE_LOG4CXX
)
# add some more tests and sub-directories

set_target_properties(accessors PROPERTIES
	OUTPUT_NAME askap_accessors
)

target_link_libraries(accessors
	lofar::Blob
	lofar::Common
	askap::parallel
	askap::scimath
	askap::askap
	${CASACORE_LIBRARIES}
	${log4cxx_LIBRARY}
	${XercesC_LIBRARY}
)

if (OPENMP_FOUND)
    target_link_libraries(accessors OpenMP::OpenMP_CXX)
endif (OPENMP_FOUND)

if (CPPUNIT_FOUND)
    target_link_libraries(accessors
        ${CPPUNIT_LIBRARY})
    target_include_directories(accessors PUBLIC
        ${CPPUNIT_INCLUDE_DIR}
    )
endif ()

target_include_directories(accessors PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/askap>
	$<INSTALL_INTERFACE:include>
	$<INSTALL_INTERFACE:include/askap>
	${Boost_INCLUDE_DIRS}
	${log4cxx_INCLUDE_DIRS}
	${CASACORE_INCLUDE_DIRS}
	${XercesC_INCLUDE_DIR}
)

add_library(askap::accessors ALIAS accessors)

install (
	TARGETS accessors
	EXPORT askap-accessors-targets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	LIBRARY PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install (FILES
	askap_accessors.h
	DESTINATION include/askap/
	)

include(CTest)
if (BUILD_TESTING)
  if (NOT CPPUNIT_FOUND)
    message(FATAL_ERROR "CPPUnit is needed for building and running unit tests")
  endif()
  enable_testing()
  add_subdirectory(tests/calibaccess)
  add_subdirectory(tests/dataaccess)
  add_subdirectory(tests/votable)
  add_subdirectory(tests/imageaccess)
endif ()


include(yanda_export)
yanda_export(askap)
