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
/// @author Steve Ord <stephen.ord@csiro.au>

#include <askap/imageaccess/WeightsLog.h>
#include <cppunit/extensions/HelperMacros.h>

#include <boost/shared_ptr.hpp>

#include <Common/ParameterSet.h>

#include "askap_accessors.h"
#include <askap/askap/AskapLogging.h>



namespace askap {

namespace accessors {

class WeightsLogTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(WeightsLogTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testReadWrite);
   CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {
        itsWeightsLog = WeightsLog("");
    }

    void testCreate() {
        // create from parset
        LOFAR::ParameterSet parset;
        const std::string name = "weightslog.test.txt";
        parset.add("WeightsLog",name);
        itsWeightsLog = WeightsLog(parset);
        CPPUNIT_ASSERT(itsWeightsLog.filename() == name);

        // create from string
        itsWeightsLog = WeightsLog(name);
        CPPUNIT_ASSERT(itsWeightsLog.filename() == name);

        // fill with values
        std::map<unsigned int, float>& mywts = itsWeightsLog.weightslist();
        mywts[0] = 10.;
        mywts[1] = 11.;
        mywts[2] = 12.;

        CPPUNIT_ASSERT(itsWeightsLog.weight(1) == 11.);
        CPPUNIT_ASSERT(itsWeightsLog.weight(2) == 12.);
        CPPUNIT_ASSERT(itsWeightsLog.weight(3) == 0.);
    }

    void testReadWrite() {
        // first need to make the weightslog again.
        testCreate();
        // Write weights log file
        itsWeightsLog.write();

        // Make sure it is empty
        itsWeightsLog.weightslist().clear();

        // Read weights log
        itsWeightsLog.read();
        CPPUNIT_ASSERT(itsWeightsLog.weight(0) == 10.);
        CPPUNIT_ASSERT(itsWeightsLog.weight(1) == 11.);
        CPPUNIT_ASSERT(itsWeightsLog.weight(2) == 12.);
   }

private:
   WeightsLog itsWeightsLog;
};

} // namespace accessors

} // namespace askap
