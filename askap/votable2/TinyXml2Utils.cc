/// @file TinyXml2Utils.cc
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
#include "TinyXml2Utils.h"

// Include package level header file
#include "askap_accessors.h"

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>
#include "boost/scoped_ptr.hpp"
#include "boost/algorithm/string/trim.hpp"

// Local package includes

using namespace tinyxml2;
using namespace askap::accessors;

std::string TinyXml2Utils::getAttribute(const tinyxml2::XMLElement& element, const std::string& name)
{
    std::string attr;
    const char* ptr = element.Attribute(name.c_str());
    if ( ptr ) attr = ptr;
    return attr;
}

tinyxml2::XMLElement* TinyXml2Utils::getFirstElementByTagName(const tinyxml2::XMLElement& element,
        const std::string& name)
{
    return (const_cast<tinyxml2::XMLElement*> (element.FirstChildElement(name.c_str())));
}

std::string TinyXml2Utils::getDescription(const tinyxml2::XMLElement& element)
{
    // Find the DESCRIPTION node
    std::string description;
    const tinyxml2::XMLElement* desElem = getFirstElementByTagName(element,"DESCRIPTION");
    if ( desElem ) {
        if ( desElem->GetText() != nullptr ) {
            description = desElem->GetText();
        }
    }
    
    return description;
}
