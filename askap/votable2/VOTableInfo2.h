/// @file VOTableInfo2.h
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
/// @author Minh Vuong <Minh.Vuong@csiro.au>

#ifndef ASKAP_ACCESSORS_VOTABLE_VOTABLEINFO2_H
#define ASKAP_ACCESSORS_VOTABLE_VOTABLEINFO2_H

// System includes
#include <string>

// ASKAPsoft includes
#include "tinyxml2.h" // Includes all DOM

namespace askap {
    namespace accessors {

        /// @brief Encapsulates the INFO element
        ///
        /// @ingroup votableaccess
        class VOTableInfo2 {
            public:

                /// @brief Constructor
                VOTableInfo2();

                void setID(const std::string& id);
                std::string getID() const;

                void setName(const std::string& name);
                std::string getName() const;

                void setValue(const std::string& value);
                std::string getValue() const;

                void setText(const std::string& text);
                std::string getText() const;

                tinyxml2::XMLElement* toXmlElement(tinyxml2::XMLDocument& doc) const;

                static VOTableInfo2 fromXmlElement(const tinyxml2::XMLElement& infoElement);

            private:

                std::string itsID;
                std::string itsName;
                std::string itsValue;
                std::string itsText;
        };

    }
}

#endif
