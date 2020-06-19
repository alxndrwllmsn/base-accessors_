add_executable(timageaccess timageaccess.cc)
include_directories(${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(timageaccess
	askap::accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME timageaccess
	COMMAND timageaccess
	)
