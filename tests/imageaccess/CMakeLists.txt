add_executable(timageaccess timageaccess.cc)
target_link_libraries(timageaccess
	askap_accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME timageaccess
	COMMAND timageaccess
	)
