/// @file VOTableTimeSys2.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_ACCESSORS_VOTABLE_VOTABLETIMESYS2_H
#define ASKAP_ACCESSORS_VOTABLE_VOTABLETIMESYS2_H

// System includes
#include <string>

// ASKAPsoft includes
#include "tinyxml2.h" // Includes all DOM

namespace askap {
    namespace accessors {

        /// @brief TODO: Write documentation...
        /// Encapsulates a TIMESYS element
        ///
        /// @ingroup votableaccess
        class VOTableTimeSys2 {
            public:

                /// @brief Constructor
                VOTableTimeSys2();


                void setID(const std::string& id);

                std::string getID() const;

                void setTimeOrigin(const std::string& timeOrigin);

                std::string getTimeOrigin() const;

                void setTimeScale(const std::string& timeScale);

                std::string getTimeScale() const;

                void setRefPosition(const std::string& refPosition);

                std::string getRefPosition() const;

                tinyxml2::XMLElement* toXmlElement(tinyxml2::XMLDocument& doc) const;

                static VOTableTimeSys2 fromXmlElement(const tinyxml2::XMLElement& timeSysElement);

            private:

                std::string itsID;
                std::string itsTimeOrigin;
                std::string itsTimeScale;
                std::string itsRefPosition;
        };

    }
}

#endif
