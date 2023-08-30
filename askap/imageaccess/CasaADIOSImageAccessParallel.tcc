/// @file CasaADIOSImageAccessParallel.cc
/// @brief Access casa image via ADIOS Storage Manager using parallel interface
/// @details This class implements IImageAccess interface for CASA image and specifies use of the ADIOS storage manager
///
/// @copyright (c) 2007 CSIRO
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
/// @author Alexander Williamson <alex.williamson@icrar.org>
/// @author Pascal Jahan Elahi <pascal.elahi@pawsey.org.au>

// have to remove this due to templating #include <askap_accessors.h>

#include <askap/imageaccess/CasaADIOSImageAccessParallel.h>

#include <askap/askap/AskapLogging.h>
#include <askap/imageaccess/ADIOSImage.h>
#include <casacore/images/Images/SubImage.h>
#include <casacore/images/Regions/ImageRegion.h>
#include <casacore/images/Regions/RegionHandler.h>

#include <mpi.h>
#include <askap/askapparallel/MPIComms.h>
#include <askap/askapparallel/AskapParallel.h>

ASKAP_LOGGER(casaADIOSImAccessLogger, ".casaADIOSImageAccessorParallel");

using namespace askap;
using namespace askap::accessors;

/// @brief constructor
/// @param[in] comms, MPI communicator
/// @param[in] config, configuration file name 
template <class T>
CasaADIOSImageAccessParallel::CasaADIOSImageAccessParallel<T>(askapparallel::AskapParallel &comms, std::string configname):
    itsComms(comms), itsParallel(-1), CasaADIOSImageAccess(config)
{
    config = configname;
    if (config != "") {
        ASKAPLOG_INFO_STR(logger, "Creating parallel ADIOS accessor with configuration file " << config);
    }
    else {
        ASKAPLOG_INFO_STR(logger, "Creating parallel ADIOS accessor with default configuration");
    }
}

