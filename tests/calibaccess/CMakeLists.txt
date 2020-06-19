add_executable(tcalibaccess tcalibaccess.cc)
include_directories(${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tcalibaccess
	askap::accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tcalibaccess
	COMMAND tcalibaccess
	)
