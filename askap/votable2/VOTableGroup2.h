/// @file VOTableGroup.h
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

#ifndef ASKAP_ACCESSORS_VOTABLE_VOTABLEGROUP2_H
#define ASKAP_ACCESSORS_VOTABLE_VOTABLEGROUP2_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "tinyxml2.h" // Includes all DOM

// Local package includes
#include "askap/votable2/VOTableParam2.h"

namespace askap {
    namespace accessors {

        /// @brief Encapsulates the GROUP element
        ///
        /// @ingroup votableaccess
        class VOTableGroup2 {
            public:

                /// @brief Constructor
                VOTableGroup2();

                void setDescription(const std::string& description);

                std::string getDescription() const;

                void setName(const std::string& name);

                std::string getName() const;

                void setID(const std::string& id);

                std::string getID() const;

                void setUCD(const std::string& ucd);

                std::string getUCD() const;

                void setUType(const std::string& utype);

                std::string getUType() const;

                void setRef(const std::string& ref);

                std::string getRef() const;

                void addParam(const VOTableParam2& param);

                std::vector<VOTableParam2> getParams() const;

                void addFieldRef(const std::string& fieldRef);

                std::vector<std::string> getFieldRefs() const;

                void addParamRef(const std::string& paramRef);

                std::vector<std::string> getParamRefs() const;

                //xercesc::DOMElement* toXmlElement(xercesc::DOMDocument& doc) const;

                static VOTableGroup2 fromXmlElement(const tinyxml2::XMLElement& e);

            private:

                std::string itsDescription;
                std::string itsName;
                std::string itsID;
                std::string itsUCD;
                std::string itsUType;
                std::string itsRef;
                std::vector<VOTableParam2> itsParams;
                std::vector<std::string> itsFieldRefs;
                std::vector<std::string> itsParamRefs;
        };

    }
}

#endif
