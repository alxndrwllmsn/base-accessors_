#
# base/accessors/imageaccess
#
add_library(imageaccess OBJECT
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

