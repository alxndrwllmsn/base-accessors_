/// @file IonoTerm.h
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
/// @author Daniel Mitchell <daniel.mitchell@csiro.au>

#ifndef ASKAP_ACCESSORS_IONOTERM_H
#define ASKAP_ACCESSORS_IONOTERM_H

// ASKAPsoft includes
#include "casacore/casa/aipstype.h"
#include "casacore/casa/BasicSL/Complex.h"

namespace askap {
namespace accessors {

/// @brief IonoTerm (low-order aperture phases screen parameters)
/// @details Currently only a linear phase gradient per direction is supported
/// @ingroup calibaccess
class IonoTerm {

    public:
        /// @brief Constructor
        /// This default (no-args) constructor is needed by various containers,
        /// for instance to populate a vector or matrix with default values.
        /// This constructor will set paramValid to false, indicating
        /// the data is not valid.
        IonoTerm()
            : itsParam(0.0, 0.0), itsParamValid(false)
        {}

        /// @brief Constructor.
        /// @param[in] param parameter
        /// @param[in] paramValid  flag indicating the validity of the data param.
        ///                     Set this to true to indicate param contains a
        ///                     valid parameter, otherwise false.
        IonoTerm(const casa::Complex& param,
                   const casa::Bool paramValid)
           : itsParam(param), itsParamValid(paramValid)
        {}

        /// Returns the gain for polarisation 1.
        /// @return the gain for polarisation 1.
        casa::Complex param(void) const { return itsParam; }

        /// Returns a flag indicating the validity of param;
        /// @return true if param contains a valid value, otherwise false.
        casa::Bool paramIsValid(void) const { return itsParamValid; }

    private:
        casacore::Complex itsParam;
        casacore::Bool itsParamValid;
};

};
};

#endif
