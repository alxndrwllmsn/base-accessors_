#
# base/accessors/votable
#
add_library(votable OBJECT
VOTable.cc
VOTableField.cc
VOTableGroup.cc
VOTableInfo.cc
VOTableParam.cc
VOTableResource.cc
VOTableRow.cc
VOTableTable.cc
XercescString.cc
XercescUtils.cc
)

set_property(TARGET votable PROPERTY POSITION_INDEPENDENT_CODE ON)

install (FILES
VOTable.h
VOTableField.h
VOTableGroup.h
VOTableInfo.h
VOTableParam.h
VOTableResource.h
VOTableRow.h
VOTableTable.h
XercescString.h
XercescUtils.h

DESTINATION include/askap/votable
)

