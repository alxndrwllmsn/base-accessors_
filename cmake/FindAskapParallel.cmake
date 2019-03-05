# - Try to find readline, a library for easy editing of command lines.
# Variables used by this module:
#  ASKAPPARALLEL_ROOT_DIR     - Readline root directory
# Variables defined by this module:
#  ASKAPPARALLEL_FOUND - system has LOFAR Common
#  ASKAPPARALLEL_INCLUDE_DIR  - the LOFAR Common/include directory (cached)
#  ASKAPPARALLEL_INCLUDE_DIRS - the LOFAR Common include directories
#                          (identical to ASKAPPARALLEL_INCLUDE_DIR)
#  ASKAPPARALLEL_LIBRARY      - the LOFAR common  library (cached)
#  ASKAPPARALLEL_LIBRARIES    - the LOFAR common library plus the libraries it 
#                          depends on

# Copyright (C) 2019


if(NOT ASKAPPARALLEL_FOUND)

	find_path(ASKAPPARALLEL_INCLUDE_DIR askap/AskapLogging.h
		HINTS ${ASKAPPARALLEL_ROOT_DIR} PATH_SUFFIXES include/askap/)
	find_library(ASKAPPARALLEL_LIBRARY askap_askap
		HINTS ${ASKAPPARALLEL_ROOT_DIR} PATH_SUFFIXES lib)
	mark_as_advanced(ASKAPPARALLEL_INCLUDE_DIR ASKAPPARALLEL_LIBRARY )

	set(ASKAPPARALLEL_INCLUDE_DIRS ${ASKAPPARALLEL_INCLUDE_DIR})
	set(ASKAPPARALLEL_LIBRARIES ${ASKAPPARALLEL_LIBRARY})
        if(CMAKE_VERSION VERSION_LESS "2.8.3")
	   find_package_handle_standard_args(ASKAPPARALLEL DEFAULT_MSG ASKAPPARALLEL_LIBRARY ASKAPPARALLEL_INCLUDE_DIR)
        else ()
	   include(FindPackageHandleStandardArgs)
	   find_package_handle_standard_args(ASKAPPARALLEL DEFAULT_MSG ASKAPPARALLEL_LIBRARY ASKAPPARALLEL_INCLUDE_DIR)
        endif ()


endif(NOT ASKAPPARALLEL_FOUND)
