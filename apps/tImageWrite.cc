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



// ASKAPSoft includes
#include "askap_accessors.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Application.h"
#include "askap/StatReporter.h"
#include "askapparallel/AskapParallel.h"
#include "askap/imageaccess/ImageAccessFactory.h"


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
   virtual int run(int argc, char* argv[])
   {
     // This class must have scope outside the main try/catch block
     askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
     try {
        StatReporter stats;
        //itsImageAccessor = accessors::imageAccessFactory(config());
        itsImageAccessor = accessors::imageAccessFactory(config(), comms);
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
};


} // namespace accessors

} // namespace askap

int main(int argc, char *argv[])
{
    askap::accessors::TestImageWriteApp app;
    return app.main(argc, argv);
}

