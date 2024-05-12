/// @file VOTableTimeSys2.cc
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
#include "VOTableTimeSys2.h"

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

ASKAP_LOGGER(logger, ".VOTableTimeSys2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableTimeSys2::VOTableTimeSys2()
{
}


void VOTableTimeSys2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableTimeSys2::getID() const
{
    return itsID;
}

void VOTableTimeSys2::setTimeOrigin(const std::string& timeOrigin)
{
    itsTimeOrigin = timeOrigin;
}

std::string VOTableTimeSys2::getTimeOrigin() const
{
    return itsTimeOrigin;
}

void VOTableTimeSys2::setTimeScale(const std::string& timeScale)
{
    itsTimeScale = timeScale;
}

std::string VOTableTimeSys2::getTimeScale() const
{
    return itsTimeScale;
}

void VOTableTimeSys2::setRefPosition(const std::string& refPosition)
{
    itsRefPosition = refPosition;
}

std::string VOTableTimeSys2::getRefPosition() const
{
    return itsRefPosition;
}

VOTableTimeSys2 VOTableTimeSys2::fromXmlElement(const tinyxml2::XMLElement& timeSysElement)
{
    VOTableTimeSys2 c;

    // Get attributes
    c.setID(TinyXml2Utils::getAttribute(timeSysElement, "ID"));
    c.setTimeOrigin(TinyXml2Utils::getAttribute(timeSysElement, "timeorigin"));
    c.setTimeScale(TinyXml2Utils::getAttribute(timeSysElement, "timescale"));
    c.setRefPosition(TinyXml2Utils::getAttribute(timeSysElement, "refposition"));

    return c;
}

tinyxml2::XMLElement* VOTableTimeSys2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("TIMESYS");
    // Add attributes
    if (itsID.length() > 0) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsTimeOrigin.length() > 0) {
        e->SetAttribute("timeorigin", itsTimeOrigin.c_str());
    }
    if (itsTimeScale.length() > 0) {
        e->SetAttribute("timescale", itsTimeScale.c_str());
    }
    if (itsRefPosition.length() > 0) {
        e->SetAttribute("refposition", itsRefPosition.c_str());
    }

    return e;
    
}
