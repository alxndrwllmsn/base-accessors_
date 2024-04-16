/// @file VOTableTable2.h
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

#ifndef ASKAP_ACCESSORS_VOTABLE_VOTABLETABLE2_H
#define ASKAP_ACCESSORS_VOTABLE_VOTABLETABLE2_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "tinyxml2.h" // Includes all DOM

// Local package includes
#include "askap/votable2/VOTableField2.h"
#include "askap/votable2/VOTableRow2.h"
#include "askap/votable2/VOTableGroup2.h"
#include "askap/votable2/VOTableParam2.h"

namespace askap {
    namespace accessors {

        /// @brief Encapsulates the TABLE element
        ///
        /// @ingroup votableaccess
        class VOTableTable2 {
            public:

                /// @brief Constructor
                VOTableTable2();

                void setID(const std::string& id);
                std::string getID() const;

                void setName(const std::string& name);
                std::string getName() const;

                void addParam(const VOTableParam2& param);
                std::vector<VOTableParam2> getParams() const;

                void setDescription(const std::string& description);
                std::string getDescription() const;

                void addGroup(const VOTableGroup2& group);
                void addField(const VOTableField2& field);
                void addRow(const VOTableRow2& row);

                std::vector<VOTableGroup2> getGroups() const;
                std::vector<VOTableField2> getFields() const;
                std::vector<VOTableRow2> getRows() const;

                tinyxml2::XMLElement* toXmlElement(tinyxml2::XMLDocument& doc) const;

                static VOTableTable2 fromXmlElement(const tinyxml2::XMLElement& tableElement);

            private:

                std::string itsDescription;
                std::string itsName;
                std::string itsID;
                std::vector<VOTableGroup2> itsGroups;
                std::vector<VOTableParam2> itsParams;
                std::vector<VOTableField2> itsFields;
                std::vector<VOTableRow2> itsRows;
        };

    }
}

#endif
