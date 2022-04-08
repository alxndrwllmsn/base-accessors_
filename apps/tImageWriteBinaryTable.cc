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




ASKAP_LOGGER(logger, ".tImageWriteBinaryTable");



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
        LOFAR::ParameterSet parset;
        //parset.add("imagetype","fits");
        itsName = config().getString("name","testCreateFitsBinaryTable");
        itsImageAccessor = imageAccessFactory(config());
    }
    casacore::Record create_dummy_record()
    {
        casacore::Record record;

        // keyword EXPOSURE
        record.define("EXPOSURE",1500);
        record.setComment("EXPOSURE","Camera exposure");
        record.define("KWORD1","Testing");

        // Create a sub record to be converted to binary table
        casacore::Record subrecord;

        // add a colunm named "Col1" that contains 10 cells of Double
        casacore::IPosition shape(1);
        shape(0) = 10;
        casacore::Array<casacore::Double> Col1Values(shape);
        casacore::Array<casacore::Double>::iterator iterend(Col1Values.end());
        int count = 1;
        for (casacore::Array<double>::iterator iter=Col1Values.begin(); iter!=iterend; ++iter) {
            *iter = count * 2.2;
            count += 1;
        }
        subrecord.define("Col1",Col1Values);

        // add a colunm named "Col2" that contains 10 cells of String
        casacore::IPosition shapeCol2(1);
        shapeCol2(0) = 10;
        casacore::Array<casacore::String> Col2Values(shapeCol2);
        casacore::Array<casacore::String>::iterator iterend2(Col2Values.end());
        count = 1;
        for (casacore::Array<casacore::String>::iterator iter=Col2Values.begin(); iter!=iterend2; ++iter) {
            *iter = std::string("col2 string") + std::to_string(count);
            //std::cout << *iter << std::endl;
            count += 1;
        }
        subrecord.define("Col2",Col2Values);

        casacore::IPosition shapeRA(1);
        shapeRA(0) = 5;
        casacore::Array<casacore::Float> raValues(shapeRA);
        casacore::Array<casacore::Float>::iterator iterendRA(raValues.end());
        count = 10;
        for (casacore::Array<casacore::Float>::iterator iter=raValues.begin(); iter!=iterendRA; ++iter) {
            *iter = count * 2.2;
            count *= 10;
        }
        subrecord.define("RA",raValues);
        
        casacore::IPosition shapeDec(1);
        shapeDec(0) = 5;
        casacore::Array<casacore::Int64> decValues(shapeDec);
        casacore::Array<casacore::Int64>::iterator iterendDec(decValues.end());
        count = 1;
        for (casacore::Array<casacore::Int64>::iterator iter=decValues.begin(); iter!=iterendDec; ++iter) {
            *iter = count*3 ;
            count = count * -2 ;
        }
        subrecord.define("Dec",decValues);

        // set up the unit for Col1 and Col2
        std::vector<std::string> vUnit;
        vUnit.push_back("Unit4Col1");
        vUnit.push_back("Unit4Col2");
        //vUnit.push_back("Unit4RACol");
        //vUnit.push_back("Unit4DecCol");

        casacore::IPosition shape2(1);
        shape2(0) = 2;
        casacore::Array<casacore::String> UnitValues(shape2);
        casacore::Array<casacore::String>::iterator iterUnit(UnitValues.end());
        count = 0;
        for (casacore::Array<casacore::String>::iterator iter=UnitValues.begin(); iter!=iterUnit; ++iter) {
            if ( count < vUnit.size() ) {
                *iter = vUnit[count] ;
            }
            count += 1;
        }
        subrecord.define("Units",UnitValues);

        record.defineRecord("Table",subrecord);

        return record;
    }
    void createTable()
    {
        // Create FITS image
        size_t ra=100, dec=100, spec=5;
        const casacore::IPosition shape(3,ra,dec,spec);
        casacore::Array<float> arr(shape);
        arr.set(1.);
        // Build a coordinate system for the image
        casacore::Matrix<double> xform(2,2);                                    // 1
        xform = 0.0; xform.diagonal() = 1.0;                          // 2
        casacore::DirectionCoordinate radec(casacore::MDirection::J2000,                  // 3
            casacore::Projection(casacore::Projection::SIN),        // 4
            135*casacore::C::pi/180.0, 60*casacore::C::pi/180.0,    // 5
            -1*casacore::C::pi/180.0, 1*casacore::C::pi/180,        // 6
            xform,                              // 7
            ra/2., dec/2.);                       // 8


        casacore::Vector<casacore::String> units(2); units = "deg";                        //  9
        radec.setWorldAxisUnits(units);
        
        // Build a coordinate system for the spectral axis
        // SpectralCoordinate
        casacore::SpectralCoordinate spectral(casacore::MFrequency::TOPO,               // 27
                    1400 * 1.0E+6,                  // 28
                    20 * 1.0E+3,                    // 29
                    0,                              // 30
                    1420.40575 * 1.0E+6);           // 31
        units.resize(1);
        units = "MHz";
        spectral.setWorldAxisUnits(units);

        casacore::CoordinateSystem coordsys;
        coordsys.addCoordinate(radec);
        coordsys.addCoordinate(spectral);


        itsImageAccessor->create(itsName, shape, coordsys);

        casacore::Record info = create_dummy_record();
        itsImageAccessor->setInfo(itsName,info);
        
    }
 
   virtual int run(int argc, char* argv[])
   {
     try {
        setup();
        createTable();
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
