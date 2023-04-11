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
#include "askap/imageaccess/FitsAuxImageSpectra.h"
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
#include <casacore/casa/Containers/Record.h>
#include <ctime>
#include <algorithm>


ASKAP_LOGGER(logger, ".tImageWriteBinaryTable");



// casa
#include <casacore/casa/OS/Timer.h>


// boost
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

/// 3rd party
#include <Common/ParameterSet.h>

#include <memory>
#include <fitsio.h>

using namespace boost::filesystem;

namespace askap {
namespace accessors {
class TestReadWriteSpectrumTableApp : public askap::Application {
public:

    void setup() 
    {
        LOFAR::ParameterSet parset;
        //parset.add("imagetype","fits");
        itsCurrentRow = 0;
        itsCol = 288;
        itsImageAccessor = accessors::imageAccessFactory(config());
    }
 
    void printerror( int status)
    {
        if (status)
        {
            fits_report_error(stderr, status); /* print error report */
            exit( status );    /* terminate the program, returning error status */
        }
    }

   void readSpectrum(long row,std::vector<float>& spectrum)
   {
        itsFitsAuxImageSpectraTable->get(row,spectrum);
   }

   void readFits(const std::string& filename, casacore::Array<float>& arr, int& nChannels)
   {
       casacore::Array<float> data = itsImageAccessor->read(filename);

       //std::cout << "nelement: " << data.nelements() << std::endl;
       auto shape = data.shape();
       ASKAPCHECK(data.ndim() == 4,"array is not 4D");

       nChannels = shape(3);
       casacore::IPosition start(4,0,0,0,0);
       casacore::IPosition end(4,0,0,0,nChannels-1);

        arr = data(start,end);

   }
   void collectSpectraType(const std::string& dirname)
   {
        boost::filesystem::path dir(dirname);
        // go thr each filr in dirname
        for(const auto& dir_entry: boost::filesystem::directory_iterator(dir)) {
            std::string fullname = dir_entry.path().c_str();
            auto pos = fullname.rfind("/");
            std::string filename(fullname,pos+1);
            // remove the .fits extension
            pos = filename.rfind(".fits");
            filename = std::string(filename,0,pos);
            //std::string fname(fullname,pos);
            // extract the type from the filename
            pos = filename.find("_component");
            std::string type(filename,0,pos);
            //std::cout << filename << std::endl;
            //std::cout << type << std::endl;
            std::string component(filename,pos+1);
            //std::cout << component << std::endl;
            auto iter = itsSpectrumTypesMap.find(type);
            if (iter != itsSpectrumTypesMap.end()) {
                auto& v = iter->second;
                v.push_back(component);
            } else{
                std::vector<std::string> v;
                v.push_back(component);
                itsSpectrumTypesMap.insert(std::make_pair(type,v));
            }
        }
   }
   virtual int run(int argc, char* argv[])
   {
     try {
        askap::StatReporter stats;
        setup();
        
        std::string srcDir = "/askapbuffer/payne/mvuong/emu";
        collectSpectraType(srcDir);
        for (const auto& m : itsSpectrumTypesMap) {
            std::string st = m.first;
            
            // extract the spectrum type ie spec_I, noise_I etc
            auto pos1 = st.find_first_of("_");
            auto pos2 = st.find_last_of("_");
            //std::cout << "pos1 = " << pos1 << "; pos2 = " << pos2 << std::endl;
            std::string spectrumType(st,pos1+1,pos2-pos1-1);
            //std::cout << spectrumType << std::endl;

            casacore::Record record;
            record.define("Stoke",spectrumType);
            std::string destDir = "/askapbuffer/payne/mvuong/pol_table/";
            destDir.append(st);
            // get the coord system of the first fits file so that we can construct the FitsAuxImageSpectra object
            std::string f = srcDir + "/" + m.first + "_" + m.second[0];
            casacore::CoordinateSystem coord = itsImageAccessor->coordSys(f);
            itsFitsAuxImageSpectraTable.reset(new FitsAuxImageSpectra(destDir,itsCol,0,coord));
            int nChannels = 0;
            for ( const auto& v : m.second ) {
                std::string fitsfile = srcDir + "/" + m.first + "_" + v;
                //std::cout << fitsfile << std::endl;
                int nChannels = 0;
                casacore::Array<float> spectrum;
                readFits(fitsfile,spectrum,nChannels);
                ASKAPCHECK(nChannels == itsCol, "channels mismatch");
                itsFitsAuxImageSpectraTable->add(v,spectrum.tovector());
                
            }
        }
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
   std::unique_ptr<FitsAuxImageSpectra> itsFitsAuxImageSpectraTable;
   unsigned long itsCurrentRow;
   unsigned long itsCol;
   /// @brief image accessor
   //boost::shared_ptr<accessors::IImageAccess<casacore::Float> > itsImageAccessor;

   /// @brief name of the image cube to write
   //std::string itsName;
    std::map<std::string, std::vector<std::string>> itsSpectrumTypesMap;
    boost::shared_ptr<accessors::IImageAccess<casacore::Float> > itsImageAccessor;
};


} // namespace accessors

} // namespace askap

int main(int argc, char *argv[])
{
    askap::accessors::TestReadWriteSpectrumTableApp app;
    return app.main(argc, argv);
}
