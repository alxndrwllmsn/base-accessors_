/// @file VOTableResource2.cc
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
#include "VOTableResource2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>

// Local package includes
#include "askap/votable2/TinyXml2Utils.h"
//#include "askap/votable/VOTableInfo.h"
//#include "askap/votable/VOTableTable.h"

ASKAP_LOGGER(logger, ".VOTableResource2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableResource2::VOTableResource2()
{
}

void VOTableResource2::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableResource2::getDescription() const
{
    return itsDescription;
}

void VOTableResource2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableResource2::getName() const
{
    return itsName;
}

void VOTableResource2::setID(const std::string& ID)
{
    itsID = ID;
}

std::string VOTableResource2::getID() const
{
    return itsID;
}

void VOTableResource2::setType(const std::string& type)
{
    itsType = type;
}

std::string VOTableResource2::getType() const
{
    return itsType;
}

void VOTableResource2::addInfo(const VOTableInfo2& info)
{
    itsInfo.push_back(info);
}

std::vector<VOTableInfo2> VOTableResource2::getInfo() const
{
    return itsInfo;
}

void VOTableResource2::addTable(const VOTableTable2& table)
{
    itsTables.push_back(table);
}

std::vector<VOTableTable2> VOTableResource2::getTables() const
{
    return itsTables;
}

VOTableResource2 VOTableResource2::fromXmlElement(const tinyxml2::XMLElement& resElement)
{
    VOTableResource2 res;

    // Get attributes
    res.setID(TinyXml2Utils::getAttribute(resElement, "ID"));
    res.setName(TinyXml2Utils::getAttribute(resElement, "name"));
    res.setType(TinyXml2Utils::getAttribute(resElement, "type"));

    // Get description
    res.setDescription(TinyXml2Utils::getDescription(resElement));

    // Process INFO
    const XMLElement* infoElement = resElement.FirstChildElement("INFO");
    while ( infoElement ) {
        const VOTableInfo2 info = VOTableInfo2::fromXmlElement(*infoElement);
        res.addInfo(info);
        infoElement = infoElement->NextSiblingElement("INFO");
    }

    // Process TABLE
    const tinyxml2::XMLElement* tableElement = TinyXml2Utils::getFirstElementByTagName(resElement,"TABLE");
    while ( tableElement ) {
        // Process the next TABLE element if there is one
        const VOTableTable2 tab = VOTableTable2::fromXmlElement(*tableElement);
        res.addTable(tab);
        tableElement = tableElement->NextSiblingElement("TABLE"); 
    } // End of processing the TABLE element

    return res;
}

tinyxml2::XMLElement* VOTableResource2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("RESOURCE");

    // Add attributes
    if (itsID.length() > 0) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsName.length() > 0) {
        e->SetAttribute("name", itsName.c_str());
    }
    if (itsType.length() > 0) {
        e->SetAttribute("type", itsType.c_str());
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        XMLElement* descElement = doc.NewElement("DESCRIPTION");
        descElement->SetText(itsDescription.c_str());
        e->InsertEndChild(descElement);
    }

    // Create INFO elements
    for (std::vector<VOTableInfo2>::const_iterator it = itsInfo.begin();
            it != itsInfo.end(); ++it) {
        e->InsertEndChild(it->toXmlElement(doc));
    }

    // Create TABLE elements
    for (std::vector<VOTableTable2>::const_iterator it = itsTables.begin();
            it != itsTables.end(); ++it) {
        e->InsertEndChild(it->toXmlElement(doc));
    }

    return e;
}
