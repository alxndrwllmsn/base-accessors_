/// @file
/// @brief Naming convention for calibratable parameters
/// @details It is handy to use the same names of the calibratable parameters
/// in different parts of the code, e.g. when they're written to a parset file or
/// added as a model parameter. This class holds methods forming the name out of
/// antenna/beam/polarisation indices and parsing the string name to get these
/// indices back. 
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H
#define ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H

#include <askap/scimath/fitting/CalParamNameHelper.h>

namespace askap { namespace accessors {

    typedef askap::scimath::CalParamNameHelper CalParamNameHelper;

}};

#endif // #ifndef ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H

