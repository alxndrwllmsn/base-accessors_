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
#include <casacore/casa/Containers/Record.h>




ASKAP_LOGGER(logger, ".tImageReadBinaryTable");

// casa
#include <casacore/casa/OS/Timer.h>


// boost
#include <boost/shared_ptr.hpp>

/// 3rd party
#include <Common/ParameterSet.h>


namespace askap {

namespace accessors {

class TestImageWriteTableApp : public askap::Application {
public:

    void setup()
    {
        itsName = config().getString("name","");
        itsImageAccessor = imageAccessFactory(config());
    }
    void readTable()
    {
        casacore::Record info;
        ASKAPLOG_INFO_STR(logger, "itsName: " << itsName);
        if (itsName.rfind(".fits") == std::string::npos) {
            itsName.append(".fits");
        }
        itsImageAccessor->getInfo(itsName,"All",info);
        auto nfields = info.nfields();
        //std::cout << info << std::endl;;
        for(long i = 0; i < nfields; i++) {
            ASKAPLOG_INFO_STR(logger, "field: " << info.name(i) << ", comment: " << info.comment(i));
            if ( info.type(i) == casacore::TpRecord ) {
                auto subRec = info.subRecord(i);
                subRec.print(std::cout);
                auto nfields2 = subRec.nfields();
                for(long f = 0; f < nfields2; f++) {
                    // ASKAPLOG_INFO_STR(logger, "field: " << subRec.name(f) << ", comment: " << subRec.comment(f));
                }
            }
        }
        ASKAPLOG_INFO_STR(logger, "nfields: " << nfields);
    }

   int run(int argc, char* argv[]) final
   {
     try {
        setup();
        readTable();
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

   /// @brief name of the image cube to write
   std::string itsName;
};


} // namespace accessors

} // namespace askap

int main(int argc, char *argv[])
{
    askap::accessors::TestImageWriteTableApp app;
    return app.main(argc, argv);
}
