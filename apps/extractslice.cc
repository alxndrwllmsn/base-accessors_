///
/// @file extractslice.cc : tool to extract slices from an image cube using
///                         standard interfaces. It can also be used to test I/O
///                         performance for various access patterns
///                         At this stage implementation is rather basic without fancy
///                         distributed access (although nothing stops us running a
///                         number of applications as an array job + it distributes multiple 
///                         slices between ranks out of the box) and delegates all
///                         optimisation to the interface implementation (i.e. it just
///                         requests a slice it needs).
///
/// @copyright (c) 2020 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// std includes
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>


// ASKAPSoft includes
#include "askap_accessors.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Application.h"
#include "askap/StatReporter.h"
#include "askapparallel/AskapParallel.h"
#include "askap/imageaccess/ImageAccessFactory.h"

#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/IPosition.h>
#include <casacore/casa/Arrays/Matrix.h>

#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/coordinates/Coordinates/StokesCoordinate.h>

#include <casacore/coordinates/Coordinates/Projection.h>

#include <askap/scimath/utils/PolConverter.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>


ASKAP_LOGGER(logger, ".extractslice");

// casa
#include <casacore/casa/OS/Timer.h>


// boost
#include <boost/shared_ptr.hpp>

/// 3rd party
#include <Common/ParameterSet.h>


namespace askap {

namespace accessors {

class ExtractSliceApp : public askap::Application {
   /// @brief fill info, get a list of slices
   void initialiseExtraction() {
        ASKAPCHECK(itsImageAccessor, "Image accessor appears to be uninitialised");
        itsShape = itsImageAccessor->shape(itsName);
        ASKAPLOG_INFO_STR(logger, "Successfully opened input cube "<<itsName<<", shape = "<<itsShape);
        const casacore::CoordinateSystem cs = itsImageAccessor->coordSys(itsName);

        casacore::Vector<casacore::Int> dirAxes = cs.directionAxesNumbers();
        ASKAPCHECK(dirAxes.nelements() == 2u, "The input cube is expected to have 2 direction axes, your one has "<<dirAxes.nelements());
        ASKAPCHECK(dirAxes[0] >=0 && dirAxes[0] < static_cast<casacore::Int>(itsShape.nelements()) && dirAxes[1] >= 0 && dirAxes[1] < static_cast<casacore::Int>(itsShape.nelements()),
                   "Direction axes do not appear to be within the shape dimensions, this shouldn't happen. dirAxes = "<<dirAxes<<" shape = "<<itsShape);
        ASKAPLOG_INFO_STR(logger, "Direction axes: "<<dirAxes);

        const casacore::Int spcAxis = cs.findCoordinate(casacore::Coordinate::SPECTRAL);
        ASKAPCHECK(spcAxis >= 0, "Spectral coordinate is not found in "<<itsName);
        itsSpcAxisIndex = checkSingleAxis(cs.pixelAxes(spcAxis));
        ASKAPCHECK(itsSpcAxisIndex < static_cast<casacore::Int>(itsShape.nelements()), "Spectral axis "<<itsSpcAxisIndex<<" appears to be outside of the cube shape = "<<itsShape);
        ASKAPLOG_INFO_STR(logger, "Spectral axis: "<<itsSpcAxisIndex);

        const casacore::Int polAxis = cs.findCoordinate(casacore::Coordinate::STOKES);
        size_t numAccountedAxes = 3u;
        itsPolAxisIndex = polAxis >= 0 ? checkSingleAxis(cs.pixelAxes(polAxis)) : -1;
        if (itsPolAxisIndex >= 0) {
            ASKAPCHECK(itsPolAxisIndex < static_cast<casacore::Int>(itsShape.nelements()), "Polarisation axis "<<itsPolAxisIndex<<" appears to be outside of the cube shape = "<<itsShape);
            ++numAccountedAxes;
            ASKAPLOG_INFO_STR(logger, "Polarisation axis: "<<itsPolAxisIndex);
        } else {
            ASKAPLOG_WARN_STR(logger, "Polarisation axis is missing");
        }
        if (itsShape.nelements() != numAccountedAxes) {
            ASKAPLOG_INFO_STR(logger, "The cube "<<itsName<<" contains additional axes beyond "<<numAccountedAxes<<" interpreted at the moment, shape = "<<itsShape);
        } else {
            if (cs.nCoordinates() + 1 != numAccountedAxes) {
                ASKAPLOG_INFO_STR(logger, "Coordinate system object of the cube "<<itsName<<" contains additional axes beyond "<<numAccountedAxes<<
                                  " interpreted at the moment and accounted for in the cube, shape = "<<itsShape);
            }
        }
        const casacore::DirectionCoordinate &dc = cs.directionCoordinate();

        // now get names from the parset
        const std::vector<std::string> sliceNames = config().getStringVector("slices.names");
        for (const std::string name : sliceNames) {
             ASKAPCHECK(itsSlices.find(name) == itsSlices.end(), "Duplicated name "<<name<<" found");
             const std::vector<std::string> direction = config().getStringVector("slices."+name+".direction");
             ASKAPCHECK(direction.size() == 3u, "Expected 3 elements for the direction for "<<name<<", you have "<<direction);
             casacore::Vector<casacore::Double> pixel(2);
             if (direction[2] == "pixel") {
                 for (int coord = 0; coord < 2; ++coord) {
                      pixel[coord] = utility::fromString<double>(direction[coord]);
                 }
             } else {
                ASKAPCHECK(direction[2] == "J2000", "Only 'pixel' and 'J2000' are supported as possible frames, for "<<name<<" you have "<<direction);
                const double ra = convertQuantity(direction[0],"rad");
                const double dec = convertQuantity(direction[1],"rad");
                const casacore::MVDirection radec(ra,dec);
                dc.toPixel(pixel, casacore::MDirection(radec, casacore::MDirection::J2000));
             }
             ASKAPCHECK(pixel.nelements() == 2u, "Expected 2 elements in the pixel vector, you have "<<pixel);
             const casacore::Int x = static_cast<casacore::Int>(pixel[0]);
             const casacore::Int y = static_cast<casacore::Int>(pixel[1]);
             ASKAPLOG_INFO_STR(logger, "Slice position "<<name<<" is at "<<pixel<<" rounded to ["<<x<<","<<y<<"]");
             if (x >= 0 && x<itsShape[dirAxes[0]] && y >= 0 && y <itsShape[dirAxes[1]]) {
                 casacore::IPosition where(itsShape.nelements(), 0);
                 where[dirAxes[0]] = x;
                 where[dirAxes[1]] = y;
                 itsSlices[name] = where;
             } else {
               ASKAPLOG_INFO_STR(logger, "       - outside the bounds of the image");
             }
        }
        itsHeader = "# slice from "+itsName+"\n#columns are channel, freq., value(s)";
   }

   /// @brief actual extraction
   void extractSlices() {
        for (auto ci : itsSlices) {
             ASKAPLOG_INFO_STR(logger, "Exporting "<<ci.first);
             casacore::IPosition trc(ci.second);
             if (itsPolAxisIndex >= 0) {
                 trc[itsPolAxisIndex] = itsShape[itsPolAxisIndex] - 1;
             }
             trc[itsSpcAxisIndex] = itsShape[itsSpcAxisIndex] - 1;
             casacore::Matrix<casacore::Float> data = itsImageAccessor->read(itsName, ci.second, trc);
             std::ofstream os(itsPrefix + ci.first);
             os<<itsHeader<<std::endl;
             os<<"# extracted from "<<ci.second<<" to "<<trc<<std::endl;
             for (casacore::Int chan = 0; chan < itsShape[itsSpcAxisIndex]; ++chan) {
                  const casacore::Vector<casacore::Float> dataVec = ((itsPolAxisIndex < 0) || (itsPolAxisIndex > itsSpcAxisIndex)) ? data.row(chan) : data.column(chan);
                  os<<chan<<" vel. to be extracted ";
                  for (size_t elem = 0; elem < dataVec.nelements(); ++elem) {
                       os<<" "<<std::setprecision(15)<<dataVec[elem];
                  }
                  os<<std::endl;
             } 
        }
   }

   /// @brief helper method to ensure that the given axis is single
   /// @details It processes the output of pixelAxes, checks that only one
   /// axis is present and returns it (i.e. the first element)
   /// @param[in] in result of CoordinateSystem::pixelAxes
   casacore::Int checkSingleAxis(const casacore::Vector<casacore::Int> &in) {
      ASKAPCHECK(in.nelements() == 1, "Expected only one element in the pixelAxes output, you have "<<in);
      return in[0];
   }

    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. 8.0arcsec. This method allows
    /// to parse a single string corresponding to such a parameter and return
    /// a double value converted to the requested units.
    /// @param[in] strval input string
    /// @param[in] unit required units (given as a string)
    /// @return converted value
    /// @note copied from SynthesisParamsHelper in yanda, need to be factored out to base
    static double convertQuantity(const std::string &strval,
                       const std::string &unit)
    {
       casacore::Quantity q;

       casacore::Quantity::read(q, strval);
       return q.getValue(casacore::Unit(unit));
    }


public:
   virtual int run(int argc, char* argv[])
   {
     // This class must have scope outside the main try/catch block
     askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
     try {
        StatReporter stats;
        casacore::Timer timer;
        timer.mark();
        // name of the input cube
        itsName = config().getString("image");
        ASKAPCHECK(itsName != "", "Cube name is not supposed to be empty");
        // name prefix for the output slices
        itsPrefix = config().getString("prefix","");
        // parameters used to setup the image accessor (just copy the full complexity from another app, although for serial access we don't need it)
        const std::string mode = config().getString("mode",comms.isParallel() ? "parallel" : "serial");
        bool collective = config().getString("imageaccess","individual") == "collective";
        bool contiguous = config().getString("imageaccess.order","distributed") == "contiguous";
        // enforce contiguous access for collective IO
        contiguous |= collective;
        ASKAPCHECK(mode == "parallel" || mode == "serial", "Unsupported mode '"<<mode<<"', it should be either parallel or serial");
        if (mode == "serial") {
            ASKAPLOG_INFO_STR(logger, "Using image accessor factory in the serial mode");
            itsImageAccessor = accessors::imageAccessFactory(config());
        } else {
          itsImageAccessor = accessors::imageAccessFactory(config(), comms);
          ASKAPLOG_INFO_STR(logger, "Using image accessor factory in the parallel mode");
        }

        if (comms.isMaster()) {
            ASKAPLOG_INFO_STR(logger, "Obtaining image parameters, building list of slices to extract");
            timer.mark();
            initialiseExtraction();
            ASKAPLOG_INFO_STR(logger, "Got "<<itsSlices.size()<<" slices to extract in "<<timer.real()<<" seconds"); 
        }
        if (comms.isParallel()) {
            ASKAPLOG_INFO_STR(logger, "Distributing the job across "<<comms.nProcs()<<" ranks");
            timer.mark();
            // logic is to be written here
            ASKAPLOG_INFO_STR(logger, "Job distribution completed in "<<timer.real()<<" seconds, this rank has "<<itsSlices.size()<<" slices to extract");
        }

        timer.mark();
        // the following will work for the serial case too if done under MPI and will just cause a single iteration over slices
        extractSlices();
        ASKAPLOG_INFO_STR(logger, "Completed extraction in "<<timer.real()<<" seconds");
        comms.barrier();
        stats.logSummary();
        return 0;
     }
     catch (const askap::AskapError& e) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
        std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
        return 1;
     } catch (const std::exception& e) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
                  << std::endl;
        return 1;
     }
   }
private:

   std::string getVersion() const override {
      const std::string pkgVersion = std::string("base-accessor:") + ASKAP_PACKAGE_VERSION;
      return pkgVersion;
   }

   /// @brief image accessor
   boost::shared_ptr<accessors::IImageAccess<casacore::Float> > itsImageAccessor;

   /// @brief name of the image cube to read
   std::string itsName;

   /// @brief shape of the input cube
   casacore::IPosition itsShape;

   /// @brief list of slices to extract
   /// @details Each element is the IPosition with all axes except spectral being filled.
   /// The number of dimensions matches that of the input cube. The key is the string name for each slice
   std::map<std::string, casacore::IPosition> itsSlices;

   /// @brief file name prefix for all output slices
   /// @details The slices are stored each in a separate file, the file name is the prefix + the name of each slice
   /// (e.g. the name of the source or whatever the user gives in the parset)
   std::string itsPrefix;

   /// @brief optional common header to be written at the start of each slice 
   std::string itsHeader;

   /// @brief spectral axis index in the cube
   casacore::Int itsSpcAxisIndex;

   /// @brief polarisation axis index in the cube
   casacore::Int itsPolAxisIndex;
};


} // namespace accessors

} // namespace askap

int main(int argc, char *argv[])
{
    askap::accessors::ExtractSliceApp app;
    return app.main(argc, argv);
}
