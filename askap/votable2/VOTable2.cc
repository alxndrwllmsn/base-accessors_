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
