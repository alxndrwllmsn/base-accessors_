add_executable(tdataaccess tdataaccess.cc)
target_link_libraries(tdataaccess
	askap_accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tdataaccess
	COMMAND tdataaccess
	)
