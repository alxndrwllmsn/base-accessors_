set (_progs
imageToFITS
tDataAccess
tVerifyUVW
tTableCalSolution
tImageWrite
tImageWriteBinaryTable
tImageReadBinaryTable
extractslice
tWriteBinarySpectrumTable
tReadWriteBinarySpectrumTable
)



foreach(prog ${_progs})
	add_executable (${prog} ${prog}.cc)
        if(MPI_COMPILE_FLAGS)
  		set_target_properties(${prog} PROPERTIES
    		COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
	endif()

	if(MPI_LINK_FLAGS)
  		set_target_properties( ${prog} PROPERTIES
    		LINK_FLAGS "${MPI_LINK_FLAGS}")
	endif()

    target_link_libraries(${prog}
	lofar::Blob
	lofar::Common
	askap::parallel
	askap::scimath
	askap::askap
    askap::accessors
	${CASACORE_LIBRARIES}
	${log4cxx_LIBRARY}
	${XercesC_LIBRARY}
    )

	install (TARGETS ${prog} DESTINATION bin)

endforeach(prog ${_progs})

install (FILES
tVerifyUVW.log_cfg
DESTINATION etc
)

