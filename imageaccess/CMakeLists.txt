#
# base/accessors/imageaccess
#
add_library(imageaccess
BeamLogger.cc
CasaImageAccess.cc
FITSImageRW.cc
FitsImageAccess.cc
IImageAccess.cc
ImageAccessFactory.cc
)
install (FILES
BeamLogger.h
CasaImageAccess.h
FITSImageRW.h
FitsImageAccess.h
IImageAccess.h
ImageAccessFactory.h

DESTINATION include/askap/imageaccess
)

target_link_libraries(
	imageaccess
	${CASACORE_LIBRARIES}
	${log4cxx_LIBRARY}
	${LofarBlob_LIBRARY}
	${LofarCommon_LIBRARY}
	${ASKAPPARALLEL_LIBRARY}
	)

