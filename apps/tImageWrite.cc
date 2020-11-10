//
// @file tImageWrite.cc : functional test to exercise image accessor and   
//                        write image cube with fake data 
//
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


// ASKAPSoft includes
#include "askap_accessors.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Application.h"
#include "askap/StatReporter.h"
#include "askapparallel/AskapParallel.h"
#include "askap/imageaccess/ImageAccessFactory.h"
#include <askap/scimath/utils/ComplexGaussianNoise.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/IPosition.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <askap/scimath/utils/MultiDimPosIter.h>
#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/coordinates/Coordinates/StokesCoordinate.h>
#include <casacore/coordinates/Coordinates/Projection.h>
#include <askap/scimath/utils/PolConverter.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>




ASKAP_LOGGER(logger, ".tImageWrite");



// casa
#include <casacore/casa/OS/Timer.h>


// boost
#include <boost/shared_ptr.hpp>

/// 3rd party
#include <Common/ParameterSet.h>


namespace askap {

namespace accessors {

class TestImageWriteApp : public askap::Application {
public:
   /// @brief fill the buffer with fake data
   /// @param[in] seed seed for the random number generator
   void setupData(casa::Int seed = 0) {
     const casa::uInt size = config().getUint32("size", 1024u);
     itsPixels.resize(size,size);
     const double variance = config().getDouble("variance", 1.);
     // it's handy to reuse existing code, performance aspects are not an issue as this is done at initialisation time
     scimath::ComplexGaussianNoise cgn(variance, seed);
     for (casacore::Array<casacore::Float>::contiter it = itsPixels.cbegin(); it != itsPixels.cend(); ++it) {
          const casacore::Complex val = cgn();
          *it = casacore::real(val);
          if (++it != itsPixels.cend()) {
              *it = casacore::imag(val);
          }
     }
     if (config().getBool("mask",false)) {
         const float threshold = config().getFloat("mask.threshold", 3.);
         ASKAPLOG_INFO_STR(logger, "Image pixel masking is enabled, pixels greater than "<<threshold<<" by absolute value will be masked");
         itsMask.resize(size,size);
         itsMask.set(false);
         casacore::Array<casacore::Float>::contiter it1 = itsPixels.cbegin();
         casacore::Array<casacore::Bool>::contiter it2 = itsMask.cbegin();
         for (; it1 != itsPixels.cend() && it2 != itsMask.cend(); ++it1,++it2) {
              if (casacore::abs(*it1) > threshold) {
                  *it2 = true;
              }
         }
     }
   }

   void writeData(casa::uInt nChunks, casa::uInt chunk) {
      ASKAPDEBUGASSERT(itsShape.nelements() > 2u);
      const casacore::IPosition addPlanesShape(itsShape.getLast(itsShape.nelements() - 2u));
      scimath::MultiDimPosIter it;
      for (it.init(addPlanesShape, nChunks, chunk); it.hasMore(); it.next()) {
           // it.cursor() corresponds to all other dimensions of the cube except the first two
           // prepending [0,0] gets the 'where' IPosition we want to pass to the interface
           // chunk/nChunks allow to iterate over subsection in the parallel case
           // in principle, it could be extended to spatial pixels too, the iterator doesn't care - 
           // it guaranteed to iterate over the whole shape passed to init (or the constructor) splitting
           // iteration over the given number of chunks deterministically, which we setup to be one per rank
           casacore::IPosition where(2,0,0);
           where.append(it.cursor());
           if (itsMask.nelements() == 0u) {
               ASKAPLOG_INFO_STR(logger, "Writing the plane data to: "<<where);
               itsImageAccessor->write(itsName, itsPixels, where);
           } else {
               ASKAPLOG_INFO_STR(logger, "Writing the plane data and mask to: "<<where);
               itsImageAccessor->write(itsName, itsPixels, itsMask, where);
           }
      }
   }

   /// @brief create the cube via the interface
   /// @param[in] isMaster true if executed on the master rank in the parallel mode (does the actual creation of the cube)
   void createCube(bool isMaster) {
      itsName = config().getString("name","fakecube");
      ASKAPDEBUGASSERT(itsName != "");
      ASKAPDEBUGASSERT(itsPixels.nelements() > 0u);
      // Build a coordinate system for the image
      casacore::Matrix<double> xform(2,2); 
      xform = 0.0; xform.diagonal() = 1.0; 
      casacore::DirectionCoordinate radec(casacore::MDirection::J2000, 
          casacore::Projection(casacore::Projection::SIN),  
          294*casacore::C::pi/180.0, -60*casacore::C::pi/180.0, 
          -0.01*casacore::C::pi/180.0, 0.01*casacore::C::pi/180, 
          xform, itsPixels.nrow()/2., itsPixels.ncolumn()/2.); 

      casacore::Vector<casacore::String> units(2); units = "deg"; 
      radec.setWorldAxisUnits(units);

      // Build a coordinate system for the spectral axis
      // SpectralCoordinate
      casacore::SpectralCoordinate spectral(casacore::MFrequency::TOPO, 
                              1400E+6, 20E+3, 0, 1420.40575E+6);
      units.resize(1);
      units = "MHz";
      spectral.setWorldAxisUnits(units);

      itsShape = casacore::IPosition(2,itsPixels.nrow(),itsPixels.ncolumn());

      // Build polarisation axis, default is single plane with stokes I
      // empty vector means do not create Stokes axis
      const std::vector<std::string> stokes(config().getStringVector("stokes",std::vector<std::string>(1, "I")));
      // just to allow some flexibility in defining stokes parameters we concatenate all elements first and then let
      // the standard code to parse it
      std::string stokesStr;
      for (size_t i=0; i<stokes.size(); ++i) {
           stokesStr += stokes[i];
      }
      const casacore::Vector<casacore::Stokes::StokesTypes> stokesVec = scimath::PolConverter::fromString(stokesStr);


      casacore::CoordinateSystem coordsys;
      coordsys.addCoordinate(radec);
      const bool spectralFirst = config().getBool("spectral_first",true);
      for (int order = 0; order < 2; ++order) {
           if ((order == 0) == spectralFirst) {
               coordsys.addCoordinate(spectral);
               itsShape.append(casacore::IPosition(1,itsNChan));
           } else {
               if (stokesVec.nelements() > 0) {
                   // do an explicit cast of StokesTypes to int, the array is small + this is a setup code anyway
                   casacore::Vector<int> stokesInt(stokesVec.nelements());
                   for (casa::uInt i = 0; i < stokesInt.nelements(); ++i) {
                        stokesInt[i] = static_cast<int>(stokesVec[i]);
                   }
                   casacore::StokesCoordinate stokes(stokesInt);
                   coordsys.addCoordinate(stokes);
                   itsShape.append(casacore::IPosition(1,stokesVec.nelements()));
               }
           }
      }

      if (isMaster) {
          itsImageAccessor->create(itsName, itsShape, coordsys);

          itsImageAccessor->setUnits(itsName,"Jy/pixel");
          itsImageAccessor->setBeamInfo(itsName,0.02,0.01,1.0);
  
          if (itsMask.nelements() > 0) {
              itsImageAccessor->makeDefaultMask(itsName);
          }
      }
   }

   virtual int run(int argc, char* argv[])
   {
     // This class must have scope outside the main try/catch block
     askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
     try {
        StatReporter stats;
        casacore::Timer timer;
        timer.mark();
        itsNChan = config().getUint32("nchan", comms.isParallel() ? comms.nProcs() : 10u);
        ASKAPCHECK(itsNChan > 0, "The number of channels is supposed to be positive");
        itsName = config().getString("name","fakecube");
        ASKAPCHECK(itsName != "", "Cube name is not supposed to be empty");
        const std::string mode = config().getString("mode",comms.isParallel() ? "parallel" : "serial");
        ASKAPCHECK(mode == "parallel" || mode == "serial", "Unsupported mode '"<<mode<<"', it should be either parallel or serial");
        if (mode == "serial") {
            ASKAPLOG_INFO_STR(logger, "Using image accessor factory in the serial mode");
            itsImageAccessor = accessors::imageAccessFactory(config());
        } else {
          itsImageAccessor = accessors::imageAccessFactory(config(), comms);
          ASKAPLOG_INFO_STR(logger, "Using image accessor factory in the parallel mode");
        }
        ASKAPLOG_INFO_STR(logger, "Setting up array with fake data");
        setupData(comms.rank() + 1); 
        ASKAPASSERT(itsPixels.nrow() > 0 && itsPixels.ncolumn() > 0);
        ASKAPLOG_INFO_STR(logger, "Filled "<<itsPixels.nrow()<<" x "<<itsPixels.ncolumn()<<" array with random numbers, simulation time "<<timer.real()<<" seconds");

        timer.mark();
        // this should work in serial too. For simplicity, just redo everything except the actual cube creation on the slave ranks
        createCube(comms.isMaster());
        if (comms.isMaster()) {
            ASKAPLOG_INFO_STR(logger, "Successfully created '"<<itsName<<"' cube with shape "<<itsShape<<", time "<<timer.real()<<" seconds");
        }
        // for simplicity, just wait until the cube is created in the parallel mode. We could've distributed itsName and itsShape instead
        timer.mark();
        comms.barrier();
        if (comms.isWorker()) {
            ASKAPLOG_INFO_STR(logger, "Ready to write data, cube should be created by now on the master rank, time "<<timer.real()<<" seconds");
        }

        timer.mark();
        if (mode == "serial") {
            writeData(1, 0);
        } {
           // the following will work for the serial case too if done under MPI and will just cause a single iteration over planes
           writeData(comms.nProcs(), comms.rank());
        }
        ASKAPLOG_INFO_STR(logger, "Completed writing data, time "<<timer.real()<<" seconds");

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
   /// @brief image accessor
   boost::shared_ptr<accessors::IImageAccess<casacore::Float> > itsImageAccessor;

   /// @brief buffer with fake data   
   casacore::Matrix<casacore::Float> itsPixels;

   /// @brief optional mask, used if array is not empty
   casacore::Matrix<casacore::Bool> itsMask;

   /// @brief number of planes in the output cube
   casa::uInt itsNChan;

   /// @brief name of the image cube to write
   std::string itsName;

   /// @brief shape of the resulting cube
   casacore::IPosition itsShape;
};


} // namespace accessors

} // namespace askap

int main(int argc, char *argv[])
{
    askap::accessors::TestImageWriteApp app;
    return app.main(argc, argv);
}

