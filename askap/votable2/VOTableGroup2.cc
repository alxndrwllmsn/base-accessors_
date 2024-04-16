/// @file VOTableGroup2.cc
///
/// @copyright (c) 2012 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Minh Vuong <Minh.Vuong@csiro.au>

// Include own header file first
#include "VOTableGroup2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>
#include "askap/votable/XercescString.h"

// Local package includes
#include "askap/votable2/TinyXml2Utils.h"
#include "askap/votable2/VOTableParam2.h"

ASKAP_LOGGER(logger, ".VOTableGroup2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableGroup2::VOTableGroup2()
{
}

void VOTableGroup2::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableGroup2::getDescription() const
{
    return itsDescription;
}

void VOTableGroup2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableGroup2::getName() const
{
    return itsName;
}

void VOTableGroup2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableGroup2::getID() const
{
    return itsID;
}

void VOTableGroup2::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableGroup2::getUCD() const
{
    return itsUCD;
}

void VOTableGroup2::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableGroup2::getUType() const
{
    return itsUType;
}

void VOTableGroup2::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableGroup2::getRef() const
{
    return itsRef;
}

void VOTableGroup2::addParam(const VOTableParam2& param)
{
    itsParams.push_back(param);
}

std::vector<VOTableParam2> VOTableGroup2::getParams() const
{
    return itsParams;
}

void VOTableGroup2::addFieldRef(const std::string& fieldRef)
{
    itsFieldRefs.push_back(fieldRef);
}

std::vector<std::string> VOTableGroup2::getFieldRefs() const
{
    return itsFieldRefs;
}

void VOTableGroup2::addParamRef(const std::string& paramRef)
{
    itsParamRefs.push_back(paramRef);
}

std::vector<std::string> VOTableGroup2::getParamRefs() const
{
    return itsParamRefs;
}

VOTableGroup2 VOTableGroup2::fromXmlElement(const tinyxml2::XMLElement& groupElement)
{
    VOTableGroup2 g;

    // Get attributes
    g.setName(TinyXml2Utils::getAttribute(groupElement, "name"));
    g.setID(TinyXml2Utils::getAttribute(groupElement, "ID"));
    g.setUCD(TinyXml2Utils::getAttribute(groupElement, "ucd"));
    g.setUType(TinyXml2Utils::getAttribute(groupElement, "utype"));
    g.setRef(TinyXml2Utils::getAttribute(groupElement, "ref"));

    // Get description
    g.setDescription(TinyXml2Utils::getDescription(groupElement));

    // Process PARAM
    const XMLElement* paramElement = groupElement.FirstChildElement("PARAM");
    while ( paramElement ) {
        const VOTableParam2 param = VOTableParam2::fromXmlElement(*paramElement);
        g.addParam(param);
        paramElement = paramElement->NextSiblingElement("PARAM");
    }

    // Process FIELDref elements
    const XMLElement* fieldRefElement = groupElement.FirstChildElement("FIELDref");
    while ( fieldRefElement ) {
        g.addFieldRef(TinyXml2Utils::getAttribute(*fieldRefElement, "ref"));
        fieldRefElement = fieldRefElement->NextSiblingElement("FIELDref");
    }

    // Process PARAMref elements
    const XMLElement* paramRefElement = groupElement.FirstChildElement("PARAMref");
    while ( paramRefElement ) {
        g.addParamRef(TinyXml2Utils::getAttribute(*paramRefElement, "ref"));
        paramRefElement = paramRefElement->NextSiblingElement("PARAMref");
    }
    return g;
}

tinyxml2::XMLElement* VOTableGroup2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("GROUP");

    // Add attributes
    if (itsName.length() > 0) {
        e->SetAttribute("name", itsName.c_str());
    }
    if (itsID.length() > 0) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsUCD.length() > 0) {
        e->SetAttribute("ucd", itsUCD.c_str());
    }
    if (itsUType.length() > 0) {
        e->SetAttribute(("utype"), itsUType.c_str());
    }
    if (itsRef.length() > 0) {
        e->SetAttribute("ref", itsRef.c_str());
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        XMLElement* descElement = doc.NewElement("DESCRIPTION");
        descElement->SetText(itsDescription.c_str());
        e->InsertEndChild(descElement);
    }

    // Create PARAM elements
    for (std::vector<VOTableParam2>::const_iterator it = itsParams.begin();
            it != itsParams.end(); ++it) {
        e->InsertEndChild(it->toXmlElement(doc));
    }

    // Create FIELDref elements
    for (std::vector<std::string>::const_iterator it = itsFieldRefs.begin();
            it != itsFieldRefs.end(); ++it) {
        XMLElement* fr = doc.NewElement("FIELDref");
        fr->SetAttribute("ref", (*it).c_str());
        e->InsertEndChild(fr);
    }

    // Create PARAMref elements
    for (std::vector<std::string>::const_iterator it = itsParamRefs.begin();
            it != itsParamRefs.end(); ++it) {
        XMLElement* fr = doc.NewElement("PARAMref");
        fr->SetAttribute("ref", (*it).c_str());
        e->InsertEndChild(fr);
    }
    return e;
}
