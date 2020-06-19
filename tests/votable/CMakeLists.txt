add_executable(tvotable tvotable.cc)
include_directories(${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tvotable
	askap::accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tvotable
	COMMAND tvotable
	)
