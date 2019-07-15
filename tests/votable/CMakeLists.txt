add_executable(tvotable tvotable.cc)
target_link_libraries(tvotable
	askap_accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tvotable
	COMMAND tvotable
	)
