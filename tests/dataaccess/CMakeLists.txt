add_executable(tdataaccess tdataaccess.cc)
include_directories(${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tdataaccess
	askap::accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tdataaccess
	COMMAND tdataaccess
	)
