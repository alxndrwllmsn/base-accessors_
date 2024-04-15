/// @file VOTableTable2.cc
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
#include "VOTableTable2.h"
#include "TinyXml2Utils.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>

// Local package includes
//#include "askap/votable/VOTableField.h"
//#include "askap/votable/VOTableRow.h"

ASKAP_LOGGER(logger, ".VOTableTable2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableTable2::VOTableTable2()
{
}

void VOTableTable2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableTable2::getID() const
{
    return itsID;
}

void VOTableTable2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableTable2::getName() const
{
    return itsName;
}

void VOTableTable2::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableTable2::getDescription() const
{
    return itsDescription;
}

void VOTableTable2::addGroup(const VOTableGroup2& group)
{
    itsGroups.push_back(group);
}

void VOTableTable2::addField(const VOTableField2& field)
{
    itsFields.push_back(field);
}

void VOTableTable2::addRow(const VOTableRow2& row)
{
    itsRows.push_back(row);
}

std::vector<VOTableGroup2> VOTableTable2::getGroups() const
{
    return itsGroups;
}

std::vector<VOTableField2> VOTableTable2::getFields() const
{
    return itsFields;
}

std::vector<VOTableRow2> VOTableTable2::getRows() const
{
    return itsRows;
}

VOTableTable2 VOTableTable2::fromXmlElement(const tinyxml2::XMLElement& tableElement)
{
    VOTableTable2 tab;

    // Get attributes
    tab.setID(TinyXml2Utils::getAttribute(tableElement, "ID"));
    tab.setName(TinyXml2Utils::getAttribute(tableElement, "name"));

    // Get description
    tab.setDescription(TinyXml2Utils::getDescription(tableElement));

    // Process GROUP
    const XMLElement* groupElement = tableElement.FirstChildElement("GROUP");
    while ( groupElement ) {
        const VOTableGroup2 group = VOTableGroup2::fromXmlElement(*groupElement);
        tab.addGroup(group);
        groupElement = groupElement->NextSiblingElement("GROUP");
    }

    // Process FIELD element
    const XMLElement* fieldElement = tableElement.FirstChildElement("FIELD");
    while ( fieldElement ) {
        VOTableField2 f = VOTableField2::fromXmlElement(*fieldElement);
        tab.addField(f);
        // Get the next field element
        fieldElement = fieldElement->NextSiblingElement("FIELD");
    } // No more FIELD element to process
    
    // Process DATA element
    const XMLElement* dataElement = tableElement.FirstChildElement("DATA");
    while ( dataElement ) {
        // Process TABLEDATA element
        const XMLElement* tableDataElement = dataElement->FirstChildElement("TABLEDATA");
        while ( tableDataElement ) {
            // Process the TR element
            unsigned long k = 0;
            const XMLElement* trElement = tableDataElement->FirstChildElement();
            while ( trElement ) {
                VOTableRow2 row = VOTableRow2::fromXmlElement(*trElement);
                tab.addRow(row);
                unsigned long r = k % 100000;
                if ( r == 0 ) {
                    ASKAPLOG_DEBUG_STR(logger,"Has proccessed more than " << k*100000 + 1 << " components");
                }
                k += 1;
                // process next TR element
                trElement = trElement->NextSiblingElement();
            } // no more TR element to process
            // process the next TABLEDATA element
            tableDataElement= tableDataElement->NextSiblingElement();
        } // no more TABLEDATA to process
        // process the next DATA element
        dataElement = dataElement->NextSiblingElement();
    } // no more ele ment to process
    
    return tab;
}

tinyxml2::XMLElement* VOTableTable2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("TABLE");

    // Add attributes
    if (itsID.length()) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsName.length() > 0) {
        e->SetAttribute("name", itsName.c_str());
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        XMLElement* descElement = doc.NewElement("DESCRIPTION");
        descElement->SetText(itsDescription.c_str());
        e->InsertEndChild(descElement);
    }

    // Create GROUP elements
    for (std::vector<VOTableGroup2>::const_iterator it = itsGroups.begin();
            it != itsGroups.end(); ++it) {
        e->InsertEndChild(it->toXmlElement(doc));
    }

    // Create FIELD elements
    for (std::vector<VOTableField2>::const_iterator it = itsFields.begin();
            it != itsFields.end(); ++it) {
        e->InsertEndChild(it->toXmlElement(doc));
    }

    // Create DATA element
    XMLElement* dataElement = doc.NewElement("DATA");
    e->InsertEndChild(dataElement);

    // Create TABLEDATA element
    XMLElement* tableDataElement = doc.NewElement("TABLEDATA");
    dataElement->InsertEndChild(tableDataElement);

    // Add rows
    for (std::vector<VOTableRow2>::const_iterator it = itsRows.begin();
            it != itsRows.end(); ++it) {
        tableDataElement->InsertEndChild(it->toXmlElement(doc));
    }

    return e;
}
