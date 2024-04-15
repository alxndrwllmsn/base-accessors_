/// @file VOTableParam.cc
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "VOTableParam2.h"

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

ASKAP_LOGGER(logger, ".VOTableParam2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableParam2::VOTableParam2()
{
}

void VOTableParam2::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableParam2::getDescription() const
{
    return itsDescription;
}

void VOTableParam2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableParam2::getName() const
{
    return itsName;
}

void VOTableParam2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableParam2::getID() const
{
    return itsID;
}

void VOTableParam2::setDatatype(const std::string& datatype)
{
    itsDatatype = datatype;
}

std::string VOTableParam2::getDatatype() const
{
    return itsDatatype;
}

void VOTableParam2::setArraysize(const std::string& arraysize)
{
    itsArraysize = arraysize;
}

std::string VOTableParam2::getArraysize() const
{
    return itsArraysize;
}

void VOTableParam2::setUnit(const std::string& unit)
{
    itsUnit = unit;
}

std::string VOTableParam2::getUnit() const
{
    return itsUnit;
}

void VOTableParam2::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableParam2::getUCD() const
{
    return itsUCD;
}

void VOTableParam2::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableParam2::getUType() const
{
    return itsUType;
}

void VOTableParam2::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableParam2::getRef() const
{
    return itsRef;
}

void VOTableParam2::setValue(const std::string& value)
{
    itsValue = value;
}

std::string VOTableParam2::getValue() const
{
    return itsValue;
}

VOTableParam2 VOTableParam2::fromXmlElement(const tinyxml2::XMLElement& paramElement)
{
    VOTableParam2 p;

    // Get attributes
    p.setName(TinyXml2Utils::getAttribute(paramElement, "name"));
    p.setID(TinyXml2Utils::getAttribute(paramElement, "ID"));
    p.setDatatype(TinyXml2Utils::getAttribute(paramElement, "datatype"));
    p.setArraysize(TinyXml2Utils::getAttribute(paramElement, "arraysize"));
    p.setUnit(TinyXml2Utils::getAttribute(paramElement, "unit"));
    p.setUCD(TinyXml2Utils::getAttribute(paramElement, "ucd"));
    p.setUType(TinyXml2Utils::getAttribute(paramElement, "utype"));
    p.setRef(TinyXml2Utils::getAttribute(paramElement, "ref"));
    p.setValue(TinyXml2Utils::getAttribute(paramElement, "value"));

    // Get description
    p.setDescription(TinyXml2Utils::getDescription(paramElement));

    return p;
}

tinyxml2::XMLElement* VOTableParam2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("PARAM");

    // Add attributes
    if (itsName.length() > 0) {
        e->SetAttribute("name", itsName.c_str());
    }
    if (itsID.length() > 0) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsDatatype.length() > 0) {
        e->SetAttribute("datatype", itsDatatype.c_str());
    }
    if (itsArraysize.length() > 0) {
        e->SetAttribute("arraysize", itsArraysize.c_str());
    }
    if (itsUnit.length() > 0) {
        e->SetAttribute("unit", itsUnit.c_str());
    }
    if (itsUCD.length() > 0) {
        e->SetAttribute("ucd", itsUCD.c_str());
    }
    if (itsUType.length() > 0) {
        e->SetAttribute("utype", itsUType.c_str());
    }
    if (itsRef.length() > 0) {
        e->SetAttribute("ref", itsRef.c_str());
    }
    if (itsValue.length() > 0) {
        e->SetAttribute("value", itsValue.c_str());
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        XMLElement* descElement = doc.NewElement("DESCRIPTION");
        descElement->SetText(itsDescription.c_str());
        e->InsertEndChild(descElement);
    }

    return e;
}
