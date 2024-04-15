/// @file VOTable2.cc
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
#include "VOTable2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>
#include <sstream>
#include <istream>
#include <ostream>
#include <iostream>
#include <memory>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>
#include "boost/scoped_ptr.hpp"
#include "boost/algorithm/string/trim.hpp"

// For XML
#include "askap/votable2/TinyXml2Utils.h"

ASKAP_LOGGER(logger, ".VOTable2");

using namespace std;
using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTable2::VOTable2(void)
{
}

std::string VOTable2::getDescription() const
{
    return itsDescription;
}

std::vector<askap::accessors::VOTableInfo2> VOTable2::getInfo() const
{
    return itsInfo;
}

std::vector<askap::accessors::VOTableResource2> VOTable2::getResource() const
{
    return itsResource;
}

void VOTable2::setDescription(const std::string& desc)
{
    itsDescription = desc;
}

void VOTable2::addResource(const askap::accessors::VOTableResource2& resource)
{
    itsResource.push_back(resource);
}

void VOTable2::addInfo(const askap::accessors::VOTableInfo2& info)
{
    itsInfo.push_back(info);
}

VOTable2 VOTable2::fromXMLImpl(tinyxml2::XMLDocument& doc)
{
    XMLElement* root = doc.RootElement();
    ASKAPASSERT(root != nullptr);

    // Build the VOTable
    VOTable2 vot;

    // Process DESCRIPTION
    std::string desc = TinyXml2Utils::getDescription(*root);
    boost::trim(desc);
    vot.setDescription(desc);

    // Process the RESOURCE element
    const XMLElement* resElement = root->FirstChildElement("RESOURCE");
    while ( resElement ) {
        VOTableResource2 res = VOTableResource2::fromXmlElement(*resElement);
        vot.addResource(res);
        // Get the next RESOURCE element
        resElement = resElement->NextSiblingElement("RESOURCE");
    } // no more RESOURCE element to process

    // Process INFO
    // Note: Cant see where the INFO element is specified as a child element of the
    // VOTABLE (ie root) element in the link here -
    // https://www.ivoa.net/documents/VOTable/20130920/REC-VOTable-1.3-20130920.html#ToC19
    const XMLElement* infoElement = root->FirstChildElement("INFO");
    while ( infoElement ) {
        VOTableInfo2 info = VOTableInfo2::fromXmlElement(*infoElement);
        vot.addInfo(info);
        // Get the next INFO element
        infoElement = infoElement->NextSiblingElement("INFO");
    } // no more INFO element to process

    return vot;
}

VOTable2 VOTable2::fromXML(const std::string& filename)
{
    // Check if the file exists
    std::ifstream fs(filename.c_str());
    if (!fs) {
        ASKAPTHROW(AskapError, "File " << filename << " could not be opened");
    }

    // Setup a parser
    XMLDocument doc;
    if ( doc.LoadFile(filename.c_str()) != XML_SUCCESS ) {
        ASKAPTHROW(AskapError, "Can not parse/load File " << filename);
    }

    return VOTable2::fromXMLImpl(doc);
}

void VOTable2::toXML(const std::string& filename) const
{
    XMLDocument doc;
    toXMLImpl(doc);
    doc.SaveFile(filename.c_str());
}

void VOTable2::toXML(std::ostream& os) const
{
    XMLDocument doc;
    toXMLImpl(doc);
    
    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);
    os << printer.CStr();
}

void VOTable2::toXMLImpl(tinyxml2::XMLDocument& doc) const
{
    doc.Clear();

    // Add <?xml version="1.0" standalone="yes"?> as the first line in the xml
    doc.InsertFirstChild(doc.NewDeclaration());

    // Create the root element and add it to the document
    XMLElement* root = doc.NewElement("VOTABLE");
    root->SetAttribute("version", "1.2");
    root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    root->SetAttribute("xmlns", "http://www.ivoa.net/xml/VOTable/v1.2");
    root->SetAttribute("xmlns:stc", "http://www.ivoa.net/xml/STC/v1.30");
    doc.InsertEndChild(root);


    // Create DESCRIPTION element
    if (itsDescription != "") {
        XMLElement* descElement = doc.NewElement("DESCRIPTION");
        descElement->SetText(itsDescription.c_str());
        root->InsertEndChild(descElement);
    }

    // Create INFO elements
    for (vector<VOTableInfo2>::const_iterator it = itsInfo.begin();
            it != itsInfo.end(); ++it) {
        root->InsertEndChild(it->toXmlElement(doc));
    }

    // Create RESOURCE elements
    for (vector<VOTableResource2>::const_iterator it = itsResource.begin();
            it != itsResource.end(); ++it) {
        root->InsertEndChild(it->toXmlElement(doc));
    }
}

VOTable2 VOTable2::fromXML(std::istream& is)
{
    // Read the stream into a memory buffer
    std::vector<char> buf;
    while (is.good()) {
        const char c = is.get();
        if (is.good())
            buf.push_back(c);
    }
    // Setup a parser
    XMLDocument doc;
    if ( doc.Parse(buf.data(),buf.size()) != XML_SUCCESS ) {
        ASKAPTHROW(AskapError, "Can not parse/load File ");
    }

    return VOTable2::fromXMLImpl(doc);
    
}
