//
// @file tVerifyUVW.cc : read UVWs from the given measurement set via the 
//                       standard accessor interface, check them vs. predicted
//                       values given times, array layout and phasing info
//
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


#include <askap/dataaccess/TableDataSource.h>
#include <askap_accessors.h>
#include <askap/askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".tVerifyUVW");

#include <askap/askap/AskapError.h>
#include <askap/dataaccess/SharedIter.h>
#include <askap/dataaccess/ParsetInterface.h>

#include <askap/dataaccess/TableManager.h>
#include <askap/dataaccess/IDataConverterImpl.h>

// casa
#include <casacore/measures/Measures/MFrequency.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/casa/OS/Timer.h>
#include <casacore/casa/Arrays/ArrayMath.h>


// std
#include <stdexcept>
#include <iostream>

using namespace askap;
using namespace accessors;

class UVWChecker {
public:
    /// @brief constructor
    /// @param[in] ds table data source to work with
    /// @note other constructors can be written later (i.e. for some streaming applications which cannot access table-specific info)
    explicit UVWChecker(const TableConstDataSource &ds);

    /// @brief iterate over the data - main entry point
    void run();
private:
    /// @brief const reference data source - it is set in the constructor only
    const IConstDataSource& itsDataSource;

    /// @brief buffer with antenna positions
    std::vector<casacore::MPosition> itsLayout;
};

/// @brief constructor
/// @param[in] ds table data source to work with
UVWChecker::UVWChecker(const TableConstDataSource &ds) : itsDataSource(ds) 
{
   // all operations specific to the table-based accessor are confined to this constructor
   // the rest of the work can proceed through the general interface (hence the type of 
   // itsDataSource in this class w.r.t. the type of ds)
   const casacore::uInt nAnt = ds.getNumberOfAntennas();
   itsLayout.reserve(nAnt);
   for (casacore::uInt ant = 0; ant < nAnt; ++ant) {
        itsLayout.push_back(ds.getAntennaPosition(ant));
   }
}

void UVWChecker::run() {
  IDataSelectorPtr sel=itsDataSource.createSelector();
  IDataConverterPtr conv=itsDataSource.createConverter();  
  conv->setFrequencyFrame(casacore::MFrequency::Ref(casacore::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casacore::MEpoch(casacore::Quantity(59000.,"d"),
                      casacore::MEpoch::Ref(casacore::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casacore::MDirection::Ref(casacore::MDirection::J2000)); 
    
  for (IConstDataSharedIter it=itsDataSource.createConstIterator(sel,conv);it!=it.end();++it) {  
       //cout<<"this is a test "<<it->visibility().nrow()<<" "<<it->frequency()<<endl;
       //cout<<"flags: "<<it->flag()<<endl;
       //cout<<"feed1 pa: "<<it->feed1PA()<<endl;
       std::cout<<"w: [";
       for (casacore::uInt row = 0; row<it->nRow(); ++row) {
            std::cout<<it->uvw()[row](2);
	    if (row + 1 != it->nRow()) {
	        std::cout<<", ";
	    }
       }
       std::cout<<"]"<<std::endl;
       //cout<<"noise: "<<it->noise().shape()<<endl;
       //cout<<"direction: "<<it->pointingDir2()<<endl;
       //cout<<"direction: "<<it->dishPointing2()<<endl;
       //cout<<"ant1: "<<it->antenna1()<<endl;
       //cout<<"ant2: "<<it->antenna2()<<endl;
       std::cout<<"time: "<<it->time()<<std::endl;
  }
}

// don't use the whole application harness for now, we don't need any parallelism or passing a parset
// like it is for the standard application
int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         std::cerr<<"Usage "<<argv[0]<<" measurement_set"<<std::endl;
	 return -2;
     }

     casacore::Timer timer;

     timer.mark();
     TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS); 
     UVWChecker checker(ds);
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     checker.run();
     std::cerr<<"Job: "<<timer.real()<<std::endl;
  }
  catch(const AskapError &ce) {
     std::cerr<<"AskapError has been caught. "<<ce.what()<<std::endl;
     return -1;
  }
  catch(const std::exception &ex) {
     std::cerr<<"std::exception has been caught. "<<ex.what()<<std::endl;
     return -1;
  }
  catch(...) {
     std::cerr<<"An unexpected exception has been caught"<<std::endl;
     return -1;
  }
  return 0;
}
