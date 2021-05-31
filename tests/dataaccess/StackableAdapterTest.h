/// @file 
/// $brief Tests of the multi-chunk iterator adapter
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
/// 

#ifndef STACKABLE_ADAPTER_TEST_H
#define STACKABLE_ADAPTER_TEST_H

// boost includes
#include <boost/shared_ptr.hpp>

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <askap/dataaccess/TableDataSource.h>
#include <askap/dataaccess/TableConstDataAccessor.h>
#include <askap/dataaccess/IConstDataSource.h>
#include <askap/dataaccess/TimeChunkIteratorAdapter.h>
#include <askap/dataaccess/MemBufferDataAccessorStackable.h>
#include <askap/dataaccess/StackedDataSource.h>
#include <askap/askap/AskapError.h>
#include "TableTestRunner.h"
#include <askap/askap/AskapUtil.h>

#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/TableIter.h>
#include <casacore/casa/Containers/Block.h>
#include <casacore/casa/BasicSL/String.h>


namespace askap {

namespace accessors {

class StackableAdapterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StackableAdapterTest);
  CPPUNIT_TEST(testInput);
  CPPUNIT_TEST(testInstantiate);
  CPPUNIT_TEST(testConstInstantiate);
  CPPUNIT_TEST(testStack);
  CPPUNIT_TEST(testCompare);
  CPPUNIT_TEST(testChannelSelection);
  CPPUNIT_TEST(testDataSource);
  CPPUNIT_TEST(testIterator);
  CPPUNIT_TEST(testOrderByReverse);
  CPPUNIT_TEST(testOrderByW);
  
  CPPUNIT_TEST_SUITE_END();
protected:
  static size_t countSteps(const IConstDataSharedIter &it) {
     size_t counter;
     for (counter = 0; it!=it.end(); ++it,++counter) {}
     return counter;     
  }
  static bool testAcc(MemBufferDataAccessorStackable &test) {
    test.setAccessorIndex(2);
    // testing channel 2 (0 based) and baseline (2) (0 based)
    // The UVW for this baseline and this input data set is:
    // [-218.044021106325, 975.585041111335 , 826.584555325564]
    // the data complex number should be
    // (0.351497501134872,0.0155263254418969)
    // UVW test
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(-218.044021106325),double(test.uvw()[2](0)),1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(975.585041111335),double(test.uvw()[2](1)),1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(826.584555325564),double(test.uvw()[2](2)),1e-9);
    // Data value test
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(0.351497501134872),double(test.rwVisibility().at(2,2,0).real()),1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(0.0155263254418969),double(test.rwVisibility().at(2,2,0).imag()),1e-9);
    return true;
  }
  
  
public:
  
  void testInstantiate() {
    // Test the instatiation and the auto stacking in the constructor
    // The input data source
    TableDataSource ds(TableTestRunner::msName());
    // An iterator into the data source
    IDataSharedIter it = ds.createIterator();

    MemBufferDataAccessorStackable adapter(it);
    
    // This should have buffered all the input visibilities
   
    testAcc(adapter);
    
    
  }
  void testConstInstantiate() {
    // Test the instatiation and the auto stacking in the constructor
    // The input data source
    // Also tests some pointer semantics
    TableConstDataSource ds(TableTestRunner::msName());
    // An iterator into the data source
    IConstDataSharedIter it = ds.createConstIterator();

    boost::shared_ptr<MemBufferDataAccessorStackable> adapter(new MemBufferDataAccessorStackable(it));
    
    testAcc(*adapter);
    
    
  }
  void testStack() {
    
    // The input data source
    TableConstDataSource ds(TableTestRunner::msName());
    // An iterator into the data source
    IConstDataSharedIter it = ds.createConstIterator();
    size_t count=0;
    vector<MemBufferDataAccessor> theStack;
    boost::shared_ptr<MemBufferDataAccessorStackable> adapter(new MemBufferDataAccessorStackable(*it));
    for (;it != it.end(); it.next()) {
      MemBufferDataAccessor acc(*it);
      acc.rwVisibility() = it->visibility();
      count++;
      adapter->append(acc);
    }
      
    testAcc(*adapter);
  }
  void testChannelSelection() {
    // This tests whether we only pickup the channels we want.
    // It would be bad if all the channels were picked up each time ...
    TableConstDataSource ds(TableTestRunner::msName());
    IDataSelectorPtr sel = ds.createSelector();
    // select a single channel.
    // Generate an iterator based on this channel selection
    sel->chooseChannels(1,0);
    IConstDataSharedIter it=ds.createConstIterator(sel);
    // instantiate the stack based on the iterator
    MemBufferDataAccessorStackable adapter(it);
    CPPUNIT_ASSERT_EQUAL(size_t(adapter.nChannel()),size_t(1));
    
    CPPUNIT_ASSERT_EQUAL(size_t(adapter.rwVisibility().shape().asVector()[1]),size_t(1));
  
  }
  void testCompare() {
    TableConstDataSource ds(TableTestRunner::msName());
    IConstDataSharedIter it = ds.createConstIterator();
    // All the work is done in the constructor
    MemBufferDataAccessorStackable adapter(it);
    // compare the contents.
    int index = 0;
    for (it.init() ;it != it.end(); it.next(),index++) {
      adapter.setAccessorIndex(index);
      
      for (casacore::uInt row=0;row<it->nRow();++row) {
        
        const casacore::RigidVector<casacore::Double, 3> &uvw = it->uvw()(row);
        const casacore::Double uvDist = sqrt(casacore::square(uvw(0))+
                                       casacore::square(uvw(1)));
        
        const casacore::RigidVector<casacore::Double, 3> &adapteruvw = adapter.uvw()(row);
        const casacore::Double adapteruvDist = sqrt(casacore::square(adapteruvw(0))+
        casacore::square(adapteruvw(1)));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(adapteruvDist),double(uvDist),1e-7);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(it->time()),double(adapter.time()),1e-1);
        
        // lets just test if the rotatedUVW are in the correct spot.
        const casacore::MDirection fakeTangent(it->dishPointing1()[0], casacore::MDirection::J2000);
        const casacore::Vector<casacore::RigidVector<casacore::Double, 3> >& Ruvw = it->rotatedUVW(fakeTangent);
        const casacore::Vector<casacore::RigidVector<casacore::Double, 3> >& adapterRuvw = adapter.rotatedUVW(fakeTangent);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(Ruvw(row)(0)), double(adapterRuvw(row)(0)), 1e-9);
        
        // lets test the visibilities.
        // in the adapter case we load the visibilities into the rwVisibility() cube.
        for (int nchan = 0; nchan < it->nChannel(); nchan++)
        {
          for (int npol = 0; npol < it-> nPol(); npol++)
          {
            CPPUNIT_ASSERT_DOUBLES_EQUAL(adapter.rwVisibility().at(row,nchan,npol).real(), it->visibility().at(row,nchan,npol).real(),1e-9);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(adapter.rwVisibility().at(row,nchan,npol).imag(), it->visibility().at(row,nchan,npol).imag(),1e-9);
          }
        }
      }
    }
  }
  void testOrderByW() {
    TableConstDataSource ds(TableTestRunner::msName());
    IConstDataSharedIter it = ds.createConstIterator();
    // All the work is done in the constructor
    MemBufferDataAccessorStackable adapter(it);
    
    adapter.orderBy( MemBufferDataAccessorStackable::OrderByOptions::W_ORDER );
    
    
  }
  void testOrderByReverse() {
    TableConstDataSource ds(TableTestRunner::msName());
    IConstDataSharedIter it = ds.createConstIterator();
    // All the work is done in the constructor
    MemBufferDataAccessorStackable adapter(it);
    
    adapter.orderBy( MemBufferDataAccessorStackable::OrderByOptions::REVERSE ); // this by default inverts the order for testing.
    
    // compare the contents // starting at the end this time
    int index = adapter.numAcc()-1;
    for (it.init() ;it != it.end(); it.next(),index--) {
      adapter.setAccessorIndex(index);
      
      for (casacore::uInt row=0;row<it->nRow();++row) {
        
        const casacore::RigidVector<casacore::Double, 3> &uvw = it->uvw()(row);
        const casacore::Double uvDist = sqrt(casacore::square(uvw(0))+
                                       casacore::square(uvw(1)));
        
        const casacore::RigidVector<casacore::Double, 3> &adapteruvw = adapter.uvw()(row);
        const casacore::Double adapteruvDist = sqrt(casacore::square(adapteruvw(0))+
        casacore::square(adapteruvw(1)));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(adapteruvDist),double(uvDist),1e-7);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(it->time()),double(adapter.time()),1e-1);
        
        // lets just test if the rotatedUVW are in the correct spot.
        const casacore::MDirection fakeTangent(it->dishPointing1()[0], casacore::MDirection::J2000);
        const casacore::Vector<casacore::RigidVector<casacore::Double, 3> >& Ruvw = it->rotatedUVW(fakeTangent);
        const casacore::Vector<casacore::RigidVector<casacore::Double, 3> >& adapterRuvw = adapter.rotatedUVW(fakeTangent);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(Ruvw(row)(0)), double(adapterRuvw(row)(0)), 1e-9);
        
        // lets test the visibilities.
        // in the adapter case we load the visibilities into the rwVisibility() cube.
        for (int nchan = 0; nchan < it->nChannel(); nchan++)
        {
          for (int npol = 0; npol < it-> nPol(); npol++)
          {
            CPPUNIT_ASSERT_DOUBLES_EQUAL(adapter.rwVisibility().at(row,nchan,npol).real(), it->visibility().at(row,nchan,npol).real(),1e-9);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(adapter.rwVisibility().at(row,nchan,npol).imag(), it->visibility().at(row,nchan,npol).imag(),1e-9);
          }
        }
      }
    }
  }
  void testInput() {
     TableConstDataSource ds(TableTestRunner::msName());
     IDataConverterPtr conv=ds.createConverter();
     conv->setEpochFrame(); // ensures seconds since 0 MJD
     CPPUNIT_ASSERT_EQUAL(size_t(420), countSteps(ds.createConstIterator(conv)));
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator(conv)));
     CPPUNIT_ASSERT_EQUAL(size_t(420), countSteps(it));
     it.reset(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),599));
     size_t counter = 0;
     for (;it->moreDataAvailable();++counter) {
          CPPUNIT_ASSERT_EQUAL(size_t(1),countSteps(it));
          if (it->moreDataAvailable()) {
              it->resume();
          }
     }
     CPPUNIT_ASSERT_EQUAL(size_t(420), counter);     
     // now trying bigger chunks
     it.reset(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),5990));
     for (counter = 0; it->moreDataAvailable(); ++counter) {
          CPPUNIT_ASSERT_EQUAL(size_t(10),countSteps(it));
          if (it->moreDataAvailable()) {
              it->resume();
          }
     }
     CPPUNIT_ASSERT_EQUAL(size_t(42), counter);     
     
  }
  void testDataSource() {
    TableConstDataSource ds(TableTestRunner::msName());
    IConstDataSharedIter it = ds.createConstIterator();
    MemBufferDataAccessorStackable adapter(it);
    
    StackedDataSource ds2(adapter);
    
    
  }
   void testIterator() {
     TableConstDataSource ds(TableTestRunner::msName());
     IConstDataSharedIter it = ds.createConstIterator();
     MemBufferDataAccessorStackable adapter(it);
     
     StackedDataSource ds2(adapter);
     IConstDataSharedIter it2 = ds2.createConstIterator();
     for (it2.init(),it.init() ;it2 != it2.end(); it2.next(),it.next())
     {
       MemBufferDataAccessor accBuffer(*it2);
       accBuffer.rwVisibility().set(0.0); // this happens in the vis processing loop and I'm worried that I clobber the vis.
       for (casacore::uInt row=0;row<it2->nRow();++row)
       {
         for (int nchan = 0; nchan < it2->nChannel(); nchan++)
           {
             for (int npol = 0; npol < it2->nPol(); npol++)
             {
      
               CPPUNIT_ASSERT_DOUBLES_EQUAL(it->visibility().at(row,nchan,npol).real(),it2->visibility().at(row,nchan,npol).real(),1e-9);
               CPPUNIT_ASSERT_DOUBLES_EQUAL(it->visibility().at(row,nchan,npol).imag(),it2->visibility().at(row,nchan,npol).imag(),1e-9);
      
             }
           }
        }
     }
   }
/*
  void testReadOnlyBuffer() {
     TableConstDataSource ds(TableTestRunner::msName());
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator()));
     // this should generate an exception
     it->buffer("TEST");
  }

  void testReadOnlyAccessor() {
     TableConstDataSource ds(TableTestRunner::msName());
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator()));
     boost::shared_ptr<IDataAccessor> acc;
     try {
        boost::shared_ptr<IDataAccessor> staticAcc(it->operator->(),utility::NullDeleter());
        ASKAPASSERT(staticAcc);
        acc = staticAcc;
     }
     catch (const AskapError &) {
        // just to ensure no exception is thrown from the try-block
        CPPUNIT_ASSERT(false);
     }
     CPPUNIT_ASSERT(acc);
     // this should generate an exception
     acc->rwVisibility();
  }
  
  void testNoResume() {
     TableConstDataSource ds(TableTestRunner::msName());
     IDataConverterPtr conv=ds.createConverter();
     conv->setEpochFrame(); // ensures seconds since 0 MJD
     boost::shared_ptr<TimeChunkIteratorAdapter> it(new TimeChunkIteratorAdapter(ds.createConstIterator(conv),5990));     
     try {
        // this code shouldn't throw AskapError 
        CPPUNIT_ASSERT(it->hasMore());        
        boost::shared_ptr<IConstDataIterator> cit = boost::dynamic_pointer_cast<IConstDataIterator>(it);
        CPPUNIT_ASSERT(cit);
        cit->next();
        CPPUNIT_ASSERT(cit->hasMore());        
        // just to access some data field
        *(*cit);
        (*cit)->antenna1();
        // access those fields directly
        *(*it);
        (*it)->antenna1();
        CPPUNIT_ASSERT_EQUAL(size_t(9),countSteps(it));
        CPPUNIT_ASSERT(it->moreDataAvailable());
        CPPUNIT_ASSERT(!it->hasMore());
     }
     catch (const AskapError &) {
        CPPUNIT_ASSERT(false);
     }
     // the following should throw AskapError
     CPPUNIT_ASSERT(it->next());
  }
*/
};

} // namespace accessors

} // namespace askap

#endif // #ifndef STACKABLE_ADAPTER_TEST_H


