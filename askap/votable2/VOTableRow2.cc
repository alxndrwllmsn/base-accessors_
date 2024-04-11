/// @file VOTableRow2.cc
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
#include "VOTableRow2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
# include <vector>
# include <string>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>
#include "boost/algorithm/string/trim.hpp"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "askap/votable2/TinyXml2Utils.h"

ASKAP_LOGGER(logger, ".VOTableRow2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableRow2::VOTableRow2()
{
}

void VOTableRow2::addCell(const std::string& cell)
{
    itsCells.push_back(cell);
}

std::vector<std::string> VOTableRow2::getCells() const
{
    return itsCells;
}

VOTableRow2 VOTableRow2::fromXmlElement(const tinyxml2::XMLElement& trElement)
{
    VOTableRow2 r;

    // Process TD element
    const XMLElement* tdElement = trElement.FirstChildElement();
    //ASKAPLOG_DEBUG_STR(logger, "+++++++++++++++++++++++++++++++++++++++++++++++++");
    while ( tdElement ) {
        std::string text("");
        const char* ptr = tdElement->GetText();
        if ( ptr ) {
            text = ptr;
            boost::trim(text);
            r.addCell(text);
        } else {
            r.addCell("ERROR: TD element has no value");
        }
        //ASKAPLOG_DEBUG_STR(logger,text);
        tdElement = tdElement->NextSiblingElement();
    }
    
    return r;
}
