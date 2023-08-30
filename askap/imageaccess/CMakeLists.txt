#
# base/accessors/imageaccess
#
add_sources_to_accessors(
BeamLogger.cc
FITSImageRW.cc
FitsImageAccess.cc
FitsImageAccessParallel.cc
ImageAccessFactory.cc
WeightsLog.cc
FitsAuxImageSpectra.cc
)

install (FILES
BeamLogger.h
CasaImageAccess.h
CasaImageAccess.tcc
ADIOSImage.h
CasaADIOSImageAccess.h
CasaADIOSImageAccess.tcc
FITSImageRW.h
FitsImageAccess.h
FitsImageAccessParallel.h
IImageAccess.h
IImageAccess.tcc
ImageAccessFactory.h
WeightsLog.h
FitsAuxImageSpectra.h

DESTINATION include/askap/imageaccess
)

