#
# base/accessors/votable2
#
add_sources_to_accessors(
VOTable2.cc
VOTableField2.cc
VOTableGroup2.cc
VOTableInfo2.cc
VOTableParam2.cc
VOTableResource2.cc
VOTableRow2.cc
VOTableTable2.cc
VOTableInfo2.cc
TinyXml2Utils.cc
tinyxml2.cpp
)

install (FILES
VOTable2.h
VOTableField2.h
VOTableGroup2.h
VOTableInfo2.h
VOTableParam2.h
VOTableResource2.h
VOTableRow2.h
VOTableInfo2.h
VOTableTable2.h
TinyXml2Utils.h
tinyxml2.h

DESTINATION include/askap/votable2
)

