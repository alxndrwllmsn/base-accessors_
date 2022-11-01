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
#include <sstream>
#include <cmath>


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
#include <casacore/casa/Arrays/Array.h>

#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/coordinates/Coordinates/StokesCoordinate.h>

#include <askap/scimath/utils/PolConverter.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>

// LOFAR for communications
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include "Blob/BlobAipsIO.h"
#include <Blob/BlobSTL.h>
#include "utils/CasaBlobUtils.h"



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
                const bool success = dc.toPixel(pixel, casacore::MDirection(radec, casacore::MDirection::J2000));
                ASKAPCHECK(success, "Failed to convert direction "<<direction<<" to pixel space, error = "<<dc.errorMessage());
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
        std::ostringstream os;
        os << "# slice from " << itsName <<std::endl;
        os << "# (cube with shape: "<<itsShape<<")"<<std::endl;
        os <<"# slice columns are channel, freq. or velocity, value(s)"<<std::endl;
        if (itsPolAxisIndex >= 0) {
            const casacore::StokesCoordinate pc = cs.stokesCoordinate();
            casacore::Vector<casacore::Stokes::StokesTypes> stokesVec(itsShape[itsPolAxisIndex]);
            for (casacore::Int pol = 0; pol < itsShape[itsPolAxisIndex]; ++pol) {
                 const bool success = pc.toWorld(stokesVec[pol], pol);
                 ASKAPCHECK(success, "Unable to convert polarisation index into physical label for plane "<<pol);
            }
            
            os <<"# polarisation axis (dimension "<<itsPolAxisIndex + 1<<"): " << scimath::PolConverter::toString(stokesVec) << std::endl;
        }
        itsHeader = os.str();
   }
   
   /// @brief helper method to perform MPI scatter operation on a complex type
   /// @details It essentially scatters the blob across all available ranks according
   /// to the per-rank length vector. 
   /// @param[in] comms communication object
   /// @param[in] blob blob string object to scatter (modified on all ranks, rank 0 will have its portion)
   /// @param[in] lengths vector of lengths, one per rank need to be defined on the master rank (always zero)
   /// @note Technical debt: we should move this method to AskapParallel/MPIComms, 
   /// this way it will have access to the active communicator (instead of using the world one),
   /// as well as provide better encapsulation of MPI calls / similar interface to other methods.
   static void scatterBlob(askap::askapparallel::AskapParallel &comms, LOFAR::BlobString &bs, const std::vector<int> &lengths = std::vector<int>()) {
       #ifdef HAVE_MPI
       if (comms.isMaster()) {
           ASKAPCHECK(lengths.size() == comms.nProcs(), "scatterBlob received "<<lengths.size()<<" lengths, but we have "<<comms.nProcs()<<" ranks");
           ASKAPCHECK(lengths.size() > 1, "Expect at least two ranks in this section of the code");
           ASKAPCHECK(bs.size() > 0, "Empty string is passed to scatterBlob");
           const std::vector<int> tempCounts(lengths.size(), 1);
           std::vector<int> tempDisplacements(lengths.size(), 0);
           for (size_t i = 0; i<lengths.size(); ++i) {
                tempDisplacements[i] = static_cast<int>(i);
           }

           // scattering individual lengths first
           const int status = MPI_Scatterv(lengths.data(), tempCounts.data(), tempDisplacements.data(), MPI_INT, MPI_IN_PLACE, 1, MPI_INT, 0, MPI_COMM_WORLD);
           ASKAPCHECK(status == MPI_SUCCESS, "Failed to scatter per-rank lengths, error = "<<status);

           // now prepare actual displacements for the second scatter call with the actual data
           for (size_t i = 0, sum = 0; i<lengths.size(); ++i) {
                tempDisplacements[i] = static_cast<int>(sum);
                const int thisRankLength = lengths[i];
                ASKAPCHECK(thisRankLength >= 0, "Blob string length for rank "<<i<<" is negative");
                ASKAPCHECK(sum < bs.size() || (thisRankLength == 0 && sum == bs.size()), "Blob string length for rank "<<i<<" exceeds the bounds of the whole blob string");
                sum += static_cast<size_t>(thisRankLength);
           }
   
           // scattering the actual data
           const int status1 = MPI_Scatterv(bs.data(), lengths.data(), tempDisplacements.data(), MPI_BYTE, MPI_IN_PLACE, lengths[0], MPI_BYTE, 0, MPI_COMM_WORLD);
           ASKAPCHECK(status1 == MPI_SUCCESS, "Failed to scatter per-rank lengths, error = "<<status1);
           
       } else {
           // first, receive the length to deal with on this particular rank
           int length = -1;
           const int status = MPI_Scatterv(NULL, NULL, NULL, MPI_INT, &length, 1, MPI_INT, 0, MPI_COMM_WORLD);
           ASKAPCHECK(status == MPI_SUCCESS, "Failed to receive scattered per-rank lengths, error = "<<status);
 
           ASKAPCHECK(length >= 0, "Message length is supposed to be non-negative");
           bs.resize(length);

           const int status1 = MPI_Scatterv(NULL, NULL, NULL, MPI_BYTE, bs.data(), length, MPI_BYTE, 0, MPI_COMM_WORLD);
           ASKAPCHECK(status1 == MPI_SUCCESS, "Failed to receive scattered per-rank lengths, error = "<<status1);
       }    
       #else 
       ASKAPTHROW(AskapError, "scatterBlob has been called, but the code appears to be built without MPI");
       #endif
   }

   /// @brief distribute slices across the whole rank space
   /// @details This method assumes that itsSlices has been initialised on rank 0 and
   /// is empty on all other ranks. Upon completion, itsSlices on each rank will contain
   /// only own share of the elements. This method shouldn't be called in the serial mode.
   /// @param[in] comms communication object
   void distributeSlices(askap::askapparallel::AskapParallel &comms) {
       ASKAPDEBUGASSERT(comms.isParallel());
       const int formatId = 0;
       if (comms.isMaster()) {
           std::vector<std::string> names;
           names.reserve(itsSlices.size());
           for (auto ci : itsSlices) {
                names.push_back(ci.first);
           }
           ASKAPDEBUGASSERT(comms.nProcs() > 0u);
           size_t slicesPerRank = names.size() < comms.nProcs() ? 1u : names.size() / comms.nProcs();
           if (names.size() > slicesPerRank * comms.nProcs()) {
               ++slicesPerRank;
           }
           ASKAPLOG_DEBUG_STR(logger, "Distribution pattern will have (about) "<<slicesPerRank<<" slice(s) per rank");

           std::vector<int> lengths(comms.nProcs(), 0);

           LOFAR::BlobString bs;
           LOFAR::BlobOBufString bob(bs);
           LOFAR::BlobOStream out(bob);
           for (size_t rank = 0, index = 0; rank < comms.nProcs(); ++rank) {
                const size_t sizeBefore = bs.size();
                out.putStart("SliceParametersForRank"+utility::toString(rank), formatId);
                const uint32_t nSlicesThisMsg = index + slicesPerRank <= names.size() ? slicesPerRank : names.size() - index;
                out << nSlicesThisMsg;
                if (nSlicesThisMsg > 0) {
                    // MV: a bit of technical debt, it would be neater to do a proper broadcast of the fixed info instead of copying it to each message
                    // (i.e. we have duplication of number of ranks times). But saves writing the same packing/unpacking into blob string for the broadcast
                    // although unlike for scatter, we do have appropriate broadcast method in MPIComms
                    out << itsShape << itsPolAxisIndex << itsSpcAxisIndex;
                    //
                    for (size_t cnt = 0; cnt < nSlicesThisMsg; ++cnt, ++index) {
                         ASKAPDEBUGASSERT(index < names.size());
                         const std::string curName = names[index];
                         std::map<std::string, casacore::IPosition>::const_iterator ci = itsSlices.find(curName);
                         ASKAPDEBUGASSERT(ci != itsSlices.end());
                         out << curName<<ci->second;
                         if (rank != 0) {
                             // basically, remove jobs sent to other ranks
                             itsSlices.erase(ci);
                         }
                    }
                }
                out.putEnd();
                lengths[rank] = bs.size() - sizeBefore;
           }
           scatterBlob(comms, bs, lengths);
       } else {
           ASKAPCHECK(itsSlices.size() == 0u, "Expected an empty slices buffer on the worker ranks");

           LOFAR::BlobString bs;

           // receive its blob string
           scatterBlob(comms, bs);

           LOFAR::BlobIBufString bib(bs);
           LOFAR::BlobIStream in(bib);
           const int version=in.getStart("SliceParametersForRank"+utility::toString(comms.rank()));
           ASKAPASSERT(version == formatId);
           uint32_t nSlicesThisMsg = 0;
           in >> nSlicesThisMsg;
           if (nSlicesThisMsg > 0) {
               // MV: a bit of technical debt, it would be neater to do a proper broadcast of the fixed info instead of copying it to each message
               // (i.e. we have duplication of number of ranks times). But saves writing the same packing/unpacking into blob string for the broadcast
               // although unlike for scatter, we do have appropriate broadcast method in MPIComms
               in >> itsShape >> itsPolAxisIndex >> itsSpcAxisIndex;
               //
               ASKAPLOG_DEBUG_STR(logger, "Extracting "<<nSlicesThisMsg<<" slices from blob");
               for (size_t cnt = 0; cnt < nSlicesThisMsg; ++cnt) {
                    std::string name;
                    casacore::IPosition where;
                    in >> name >> where;
                    ASKAPCHECK(itsSlices.find(name) == itsSlices.end(), "Duplicate slice "<<name<<" encountered");
                    itsSlices[name] = where;
               }
           } else {
               ASKAPLOG_DEBUG_STR(logger, "No job for this rank");
           }
           in.getEnd();
       }
   }


   /// @brief actual extraction
   void extractSlices() {
        const casacore::CoordinateSystem cs = itsImageAccessor->coordSys(itsName);
        const casacore::SpectralCoordinate sc = cs.spectralCoordinate();
        for (auto ci : itsSlices) {
             ASKAPLOG_INFO_STR(logger, "Exporting "<<ci.first);
             casacore::IPosition trc(ci.second);
             if (itsPolAxisIndex >= 0) {
                 trc[itsPolAxisIndex] = itsShape[itsPolAxisIndex] - 1;
             }
             trc[itsSpcAxisIndex] = itsShape[itsSpcAxisIndex] - 1;
             casacore::Array<casacore::Float> data = itsImageAccessor->read(itsName, ci.second, trc);
             const casacore::IPosition sliceShape = data.shape();
             std::ofstream os(itsPrefix + ci.first + ".dat");
             os<<itsHeader;
             os<<"# extracted from "<<ci.second<<" to "<<trc<<std::endl;
             for (casacore::Int chan = 0; chan < itsShape[itsSpcAxisIndex]; ++chan) {
                  casacore::IPosition sliceStart(sliceShape.nelements(),0);
                  sliceStart[itsSpcAxisIndex] = chan;
                  casacore::IPosition sliceEnd(sliceStart);
                  if (itsPolAxisIndex >= 0) {
                      sliceEnd[itsPolAxisIndex] = sliceShape[itsPolAxisIndex] - 1;
                  }

                  casacore::Array<casacore::Float> slice = data(sliceStart, sliceEnd);
                  const casacore::Vector<casacore::Float> dataVec = slice.reform(casacore::IPosition(1, slice.nelements()));
                  casacore::Double freqOrVel = -1.;
                  const bool success = sc.toWorld(freqOrVel, static_cast<casacore::Double>(chan));
                  ASKAPCHECK(success, "Unable to convert channel index "<<chan<<" to the physical units for "<<ci.first<<", error = "<<sc.errorMessage());
                  ASKAPCHECK(!std::isnan(freqOrVel), "Encountered NaN after frequency conversion for channel = "<<chan<<" for "<<ci.first);
                  os<<chan<<" "<<std::setprecision(15)<<freqOrVel<<" ";
                  for (size_t elem = 0; elem < dataVec.nelements(); ++elem) {
                       if (std::isnan(dataVec[elem])) {
                           os<<" flagged";
                       } else {
                           os<<" "<<std::setprecision(15)<<dataVec[elem];
                       }
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
        // parameter used to setup the image accessor 
        const std::string mode = config().getString("mode",comms.isParallel() ? "parallel" : "serial");
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
            distributeSlices(comms);
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
