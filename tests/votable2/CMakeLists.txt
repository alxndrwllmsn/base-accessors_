add_executable(tvotable2 tvotable2.cc)
include_directories(${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tvotable2
	askap::accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tvotable2
	COMMAND tvotable2
	)
