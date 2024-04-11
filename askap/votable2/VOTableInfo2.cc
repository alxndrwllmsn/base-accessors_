/// @file VOTableInfo2.cc
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
#include "VOTableInfo2.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>
#include "boost/algorithm/string/trim.hpp"
#include "askap/votable2/TinyXml2Utils.h"

ASKAP_LOGGER(logger, ".VOTableInfo2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableInfo2::VOTableInfo2()
{
}

void VOTableInfo2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableInfo2::getID() const
{
    return itsID;
}

void VOTableInfo2::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableInfo2::getName() const
{
    return itsName;
}

void VOTableInfo2::setValue(const std::string& value)
{
    itsValue = value;
}

std::string VOTableInfo2::getValue() const
{
    return itsValue;
}

void VOTableInfo2::setText(const std::string& text)
{
    itsText = text;
}

std::string VOTableInfo2::getText() const
{
    return itsText;
}

VOTableInfo2 VOTableInfo2::fromXmlElement(const tinyxml2::XMLElement& infoElement)
{
    VOTableInfo2 info;

    info.setID(TinyXml2Utils::getAttribute(infoElement, "ID"));
    info.setName(TinyXml2Utils::getAttribute(infoElement, "name"));
    info.setValue(TinyXml2Utils::getAttribute(infoElement, "value"));

    // Get the text of the INFO element
    std::string text;
    const char* ptr = infoElement.GetText();
    if ( ptr ) {
        text = ptr;
        boost::trim(text);
    }
    info.setText(text);

    return info;
}
