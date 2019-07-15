add_executable(tcalibaccess tcalibaccess.cc)
target_link_libraries(tcalibaccess
	askap_accessors
	${CPPUNIT_LIBRARY}
)
add_test(
	NAME tcalibaccess
	COMMAND tcalibaccess
	)
