#
# base/accessors/imageaccess
#
add_library(imageaccess OBJECT
BeamLogger.cc
FITSImageRW.cc
FitsImageAccess.cc
ImageAccessFactory.cc
)

set_property(TARGET imageaccess PROPERTY POSITION_INDEPENDENT_CODE ON)

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

