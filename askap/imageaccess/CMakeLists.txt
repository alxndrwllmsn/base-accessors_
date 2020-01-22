#
# base/accessors/imageaccess
#
add_sources_to_accessors(
BeamLogger.cc
FITSImageRW.cc
FitsImageAccess.cc
ImageAccessFactory.cc
)

install (FILES
BeamLogger.h
CasaImageAccess.h
CasaImageAccess.tcc
FITSImageRW.h
FitsImageAccess.h
IImageAccess.h
IImageAccess.tcc
ImageAccessFactory.h

DESTINATION include/askap/imageaccess
)

