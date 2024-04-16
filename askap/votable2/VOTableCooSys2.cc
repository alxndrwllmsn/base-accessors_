/// @file VOTableCooSys2.cc
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
#include "VOTableCooSys2.h"

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

ASKAP_LOGGER(logger, ".VOTableCooSys2");

using namespace askap;
using namespace askap::accessors;
using namespace tinyxml2;

VOTableCooSys2::VOTableCooSys2()
{
}


void VOTableCooSys2::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableCooSys2::getID() const
{
    return itsID;
}

void VOTableCooSys2::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableCooSys2::getRef() const
{
    return itsRef;
}

void VOTableCooSys2::setSystem(const std::string& sys)
{
    itsSystem = sys;
}

std::string VOTableCooSys2::getSystem() const
{
    return itsSystem;
}

void VOTableCooSys2::setEquinox(const std::string& equinox)
{
    itsEquinox = equinox;
}

std::string VOTableCooSys2::getEquinox() const
{
    return itsEquinox;
}

void VOTableCooSys2::setEpoch(const std::string& epoch)
{
    itsEpoch = epoch;
}

std::string VOTableCooSys2::getEpoch() const
{
    return itsEpoch;
}

VOTableCooSys2 VOTableCooSys2::fromXmlElement(const tinyxml2::XMLElement& cooSysElement)
{
    VOTableCooSys2 c;

    // Get attributes
    c.setID(TinyXml2Utils::getAttribute(cooSysElement, "ID"));
    c.setRef(TinyXml2Utils::getAttribute(cooSysElement, "ref"));
    c.setSystem(TinyXml2Utils::getAttribute(cooSysElement, "system"));
    c.setEquinox(TinyXml2Utils::getAttribute(cooSysElement, "equinox"));
    c.setEpoch(TinyXml2Utils::getAttribute(cooSysElement, "epoch"));

    return c;
}

tinyxml2::XMLElement* VOTableCooSys2::toXmlElement(tinyxml2::XMLDocument& doc) const
{
    XMLElement* e = doc.NewElement("COOSYS");
    // Add attributes
    if (itsID.length() > 0) {
        e->SetAttribute("ID", itsID.c_str());
    }
    if (itsRef.length() > 0) {
        e->SetAttribute("ref", itsRef.c_str());
    }
    if (itsSystem.length() > 0) {
        e->SetAttribute("system", itsSystem.c_str());
    }
    if (itsEquinox.length() > 0) {
        e->SetAttribute("equinox", itsEquinox.c_str());
    }
    if (itsEpoch.length() > 0) {
        e->SetAttribute("epoch", itsEpoch.c_str());
    }

    return e;
    
}
