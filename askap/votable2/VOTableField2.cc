/// @file VOTableField2.cc
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
#include "VOTableField2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>

// Local package includes
#include "askap/votable2/TinyXml2Utils.h"
#include "boost/algorithm/string/trim.hpp"

ASKAP_LOGGER(logger, ".VOTableField2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableField2::VOTableField2()
{
}

void VOTableField2::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableField2::getDescription() const
{
    return itsDescription;
}

void VOTableField2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableField2::getName() const
{
    return itsName;
}

void VOTableField2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableField2::getID() const
{
    return itsID;
}

void VOTableField2::setDatatype(const std::string& datatype)
{
    itsDatatype = datatype;
}

std::string VOTableField2::getDatatype() const
{
    return itsDatatype;
}

void VOTableField2::setArraysize(const std::string& arraysize)
{
    itsArraysize = arraysize;
}

std::string VOTableField2::getArraysize() const
{
    return itsArraysize;
}

void VOTableField2::setUnit(const std::string& unit)
{
    itsUnit = unit;
}

std::string VOTableField2::getUnit() const
{
    return itsUnit;
}

void VOTableField2::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableField2::getUCD() const
{
    return itsUCD;
}

void VOTableField2::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableField2::getUType() const
{
    return itsUType;
}

void VOTableField2::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableField2::getRef() const
{
    return itsRef;
}

VOTableField2 VOTableField2::fromXmlElement(const tinyxml2::XMLElement& fieldElement)
{
    VOTableField2 f;

    // Get attributes
    f.setName(TinyXml2Utils::getAttribute(fieldElement, "name"));
    f.setID(TinyXml2Utils::getAttribute(fieldElement, "ID"));
    f.setDatatype(TinyXml2Utils::getAttribute(fieldElement, "datatype"));
    f.setArraysize(TinyXml2Utils::getAttribute(fieldElement, "arraysize"));
    f.setUnit(TinyXml2Utils::getAttribute(fieldElement, "unit"));
    f.setUCD(TinyXml2Utils::getAttribute(fieldElement, "ucd"));
    f.setUType(TinyXml2Utils::getAttribute(fieldElement, "utype"));
    f.setRef(TinyXml2Utils::getAttribute(fieldElement, "ref"));

    // FIELD element has a child DESCRIPTION element
    const XMLElement* descElement = fieldElement.FirstChildElement();
    if ( descElement ) {
        const char* ptr = descElement->GetText();
        if ( ptr ) {
            std::string s(ptr);
            boost::trim(s);
            f.setDescription(s);
            //ASKAPLOG_DEBUG_STR(logger,"field id: " << f.getID() << ", desc: " << f.getDescription());
        }
    }


    return f;
}
