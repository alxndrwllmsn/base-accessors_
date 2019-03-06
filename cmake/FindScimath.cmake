# - Try to find readline, a library for easy editing of command lines.
# Variables used by this module:
#  SCIMATH_ROOT_DIR     - Readline root directory
# Variables defined by this module:
#  SCIMATH_FOUND - system has LOFAR Common
#  SCIMATH_INCLUDE_DIR  - the LOFAR Common/include directory (cached)
#  SCIMATH_INCLUDE_DIRS - the LOFAR Common include directories
#                          (identical to SCIMATH_INCLUDE_DIR)
#  SCIMATH_LIBRARY      - the LOFAR common  library (cached)
#  SCIMATH_LIBRARIES    - the LOFAR common library plus the libraries it 
#                          depends on

# Copyright (C) 2019


if(NOT SCIMATH_FOUND)

	find_path(SCIMATH_INCLUDE_DIR utils/ImageUtils.h
		HINTS ${SCIMATH_ROOT_DIR} PATH_SUFFIXES include/askap/scimath)
	find_library(SCIMATH_LIBRARY askap_scimath
		HINTS ${SCIMATH_ROOT_DIR} PATH_SUFFIXES lib)
	mark_as_advanced(SCIMATH_INCLUDE_DIR SCIMATH_LIBRARY )

	set(SCIMATH_INCLUDE_DIRS ${SCIMATH_INCLUDE_DIR})
	set(SCIMATH_LIBRARIES ${SCIMATH_LIBRARY})
        if(CMAKE_VERSION VERSION_LESS "2.8.3")
	   find_package_handle_standard_args(SCIMATH DEFAULT_MSG SCIMATH_LIBRARY SCIMATH_INCLUDE_DIR)
        else ()
	   include(FindPackageHandleStandardArgs)
	   find_package_handle_standard_args(SCIMATH DEFAULT_MSG SCIMATH_LIBRARY SCIMATH_INCLUDE_DIR)
        endif ()


endif(NOT SCIMATH_FOUND)
