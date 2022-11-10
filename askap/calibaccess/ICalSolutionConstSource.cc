/// @file
/// @brief A high-level interface to access calibration solutions
/// @details This interface hides the database look up of the appropriate
/// calibration solution. It manages solution IDs and provides access
/// to the actual solution via ICalSolutionConstAccessor.
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

#include <askap/calibaccess/ICalSolutionConstSource.h>


namespace askap {

namespace accessors {

/// @brief virtual destructor to keep the compiler happy
ICalSolutionConstSource::~ICalSolutionConstSource() {}

/// @brief obtain closest solution ID before a given time
/// @details This method looks for the first solution valid before
/// the given time and returns its ID.
/// @param[in] time time stamp in seconds since MJD of 0.
/// @return solution ID, time of solution
std::pair<long, double> ICalSolutionConstSource::solutionIDBefore(const double time) const
{
    // default implementation just returns the solutionID
    // it is up to derived classes to provide the correct implementation
    return std::pair<long, double>(solutionID(time),0.0);
}

/// @brief obtain closest solution ID after a given time
/// @details This method looks for the first solution valid after
/// the given time and returns its ID.
/// @param[in] time time stamp in seconds since MJD of 0.
/// @return solution ID, time of solution
std::pair<long, double> ICalSolutionConstSource::solutionIDAfter(const double time) const
{
    // default implementation just returns the solutionID
    // it is up to derived classes to provide the correct implementation
    return std::pair<long, double>(solutionID(time),0.0);
}


} // namespace accessors

} // namespace askap
