/// @file
///
/// Unit test for the CASA image access code
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <askap/imageaccess/ImageAccessFactory.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/IPosition.h>
#include <casacore/coordinates/Coordinates/LinearCoordinate.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/images/Regions/ImageRegion.h>
#include <casacore/images/Regions/RegionHandler.h>
#include <casacore/casa/Containers/Record.h>


#include <boost/shared_ptr.hpp>

#include <Common/ParameterSet.h>

namespace askap {

namespace accessors {

class CasaADIOSImageAccessTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(CasaADIOSImageAccessTest);
   CPPUNIT_TEST(testReadWrite);
   CPPUNIT_TEST(testWriteTable);
   // CPPUNIT_TEST(testReadTable);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
      LOFAR::ParameterSet parset;
      parset.add("imagetype","adios");
      itsImageAccessor = imageAccessFactory(parset);
      name = "tmp.testimage";
   }

   void testWriteTable() {
      auto rec = create_dummy_record("table 1");
      name = "tmp.testaddtabletoimage";
      testReadWrite();
      
      //itsImageAccessor->setInfo("tmp.testimage",rec);
      itsImageAccessor->setInfo(name,rec);
      auto rec2 = create_dummy_record("table 2");
      itsImageAccessor->setInfo(name,rec2);
   }
   void testReadTable() {
      name = "tmp.testaddtabletoimage";
      casacore::Record rec;
      itsImageAccessor->getInfo(name,"table 2",rec);
   }
   void testReadWrite() {
      //const std::string name = "tmp.testimage";
      CPPUNIT_ASSERT(itsImageAccessor);
      const casacore::IPosition shape(3,10,10,5);
      casacore::Array<float> arr(shape);
      arr.set(1.);
      casacore::CoordinateSystem coordsys(makeCoords());

      // create and write a constant into image
      itsImageAccessor->create(name, shape, coordsys);
      itsImageAccessor->write(name,arr);

      arr(casacore::IPosition(3,0,3,0), casacore::IPosition(3,9,3,0)) = 2.;

      // write a slice
      casacore::Vector<float> vec(10,2.);
      itsImageAccessor->write(name,vec,casacore::IPosition(3,0,3,0));

      // check shape
      CPPUNIT_ASSERT(itsImageAccessor->shape(name) == shape);
      // read the whole array and check
      casacore::Array<float> readBack = itsImageAccessor->read(name);
      CPPUNIT_ASSERT(readBack.shape() == shape);
      for (int x=0; x<shape[0]; ++x) {
           for (int y=0; y<shape[1]; ++y) {
                const casacore::IPosition index(3,x,y,0);
                CPPUNIT_ASSERT(fabs(readBack(index)-arr(index))<1e-7);
           }
      }

      // read a slice
      vec = itsImageAccessor->read(name,casacore::IPosition(3,0,1,0),casacore::IPosition(3,9,1,0));
      CPPUNIT_ASSERT(vec.nelements() == 10);
      for (int x=0; x<10; ++x) {
           CPPUNIT_ASSERT(fabs(vec[x] - arr(casacore::IPosition(3,x,1,0)))<1e-7);
      }
      vec = itsImageAccessor->read(name,casacore::IPosition(3,0,3,0),casacore::IPosition(3,9,3,0));
      CPPUNIT_ASSERT(vec.nelements() == 10);
      for (int x=0; x<10; ++x) {
           CPPUNIT_ASSERT(fabs(vec[x] - 2.)<1e-7);
      }
      // read the whole array and check
      readBack = itsImageAccessor->read(name);
      CPPUNIT_ASSERT(readBack.shape() == shape);
      for (int x=0; x<shape[0]; ++x) {
           for (int y=0; y<shape[1]; ++y) {
                const casacore::IPosition index(3,x,y,0);
                CPPUNIT_ASSERT(fabs(readBack(index) - (y == 3 ? 2. : 1.))<1e-7);
           }
      }
      CPPUNIT_ASSERT(itsImageAccessor->coordSys(name).nCoordinates() == 2);
      CPPUNIT_ASSERT(itsImageAccessor->coordSys(name).type(0) == casacore::CoordinateSystem::LINEAR);
      CPPUNIT_ASSERT(itsImageAccessor->coordSys(name).type(1) == casacore::CoordinateSystem::SPECTRAL);

      // auxilliary methods
      itsImageAccessor->setUnits(name,"Jy/pixel");
      itsImageAccessor->setBeamInfo(name,0.02,0.01,1.0);
      // set per plane beam information
      BeamList beamlist;
      int nchan = 5;
      for (int chan = 0; chan < nchan; chan++) {
        casacore::Vector<casacore::Quantum<double> > currentbeam(3);
        currentbeam[0] = casacore::Quantum<double>(10+chan*0.1, "arcsec");
        currentbeam[1] = casacore::Quantum<double>(5+chan*0.1, "arcsec");
        currentbeam[2] = casacore::Quantum<double>(12.0+chan, "deg");
        beamlist[chan] = currentbeam;
      }
      itsImageAccessor->setBeamInfo(name, beamlist);

      BeamList beamlist2 = itsImageAccessor->beamList(name);
      for (int chan = 0; chan < nchan; chan++) {
        CPPUNIT_ASSERT(beamlist[chan][0] == beamlist2[chan][0]);
        CPPUNIT_ASSERT(beamlist[chan][1] == beamlist2[chan][1]);
        CPPUNIT_ASSERT(beamlist[chan][2] == beamlist2[chan][2]);
      }

      // mask tests

      itsImageAccessor->makeDefaultMask(name);

   }

protected:

   casacore::CoordinateSystem makeCoords() {
      casacore::Vector<casacore::String> names(2);
      names[0]="x"; names[1]="y";
      casacore::Vector<double> increment(2 ,1.);

      casacore::Matrix<double> xform(2,2,0.);
      xform.diagonal() = 1.;
      casacore::LinearCoordinate linear(names, casacore::Vector<casacore::String>(2,"pixel"),
             casacore::Vector<double>(2,0.),increment, xform, casacore::Vector<double>(2,0.));

      casacore::CoordinateSystem coords;
      coords.addCoordinate(linear);

      coords.addCoordinate(casacore::SpectralCoordinate(casacore::MFrequency::TOPO,
        1.e9, 1.e8, 0.0));
      return coords;
   }

    casacore::Record create_dummy_record(const std::string& tblName)
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

        record.defineRecord(tblName,subrecord);

        return record;
    }
private:
   /// @brief method to access image
   boost::shared_ptr<IImageAccess<casacore::Float> > itsImageAccessor;
   std::string name = "";
};

} // namespace accessors

} // namespace askap
