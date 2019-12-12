#
# base/accessors/imageaccess
#
add_library(imageaccess OBJECT
BeamLogger.cc
FITSImageRW.cc
FitsImageAccess.cc
ImageAccessFactory.cc
)

target_link_libraries(imageaccess PRIVATE
	askap::askap
	askap::parallel
)

target_include_directories(imageaccess PRIVATE
	${PROJECT_SOURCE_DIR}
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

