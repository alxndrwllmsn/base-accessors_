//
// @file tDataAccess.cc : evolving test/demonstration program of the
//                        data access layer
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
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <askap_accessors.h>
#include <askap/askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".tVOTable");

#include <askap/askap/AskapError.h>
#include <askap/votable2/VOTable2.h>
#include <askap/votable2/VOTableField2.h>
#include <askap/votable2/VOTableRow2.h>
#include <askap/votable2/VOTableResource2.h>
#include <askap/dataaccess/ParsetInterface.h>

#include <casacore/casa/OS/Timer.h>

// std
#include <stdexcept>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace accessors;


int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" xml file to load"<<endl;
	 return -2;
     }

     ASKAPLOG_INIT("askap.log_cfg");
     casacore::Timer timer;
     timer.mark();
     ASKAPLOG_INFO_STR(logger,"Start testing ...");
     const VOTable2 vot = VOTable2::fromXML(argv[1]);
     std::vector<VOTableResource2> resources = vot.getResource();
     ASKAPLOG_INFO_STR(logger,"number of RESOURCE elements: " << resources.size());
     for ( const auto& r : resources ) {
        std::vector<VOTableTable2> tables = r.getTables();
        for ( const auto& t : tables ) {
            std::vector<VOTableField2> fields = t.getFields();
            ASKAPLOG_INFO_STR(logger,"Number of FIELD elements: " << fields.size());
            std::vector<VOTableRow2> rows = t.getRows();
            ASKAPLOG_INFO_STR(logger,"Number of rows/components: " << rows.size());
            std::vector<std::string> cells = rows[0].getCells();
            ASKAPLOG_INFO_STR(logger,"Each component has " << cells.size() << " fields");

        }
    }
    ASKAPLOG_INFO_STR(logger,"Completed testing ... time taken - " << timer.real()/60 << " minutes");

    //vot.toXML("out.xml");
    std::ostringstream s; 
    vot.toXML(s);
    //std::cout << s.str();
    std::istringstream is(s.str());
    const VOTable2 vot2 = VOTable2::fromXML(is);  
    vot2.toXML("out.xml");
  }
  catch(const AskapError &ce) {
     cerr<<"AskapError has been caught. "<<ce.what()<<endl;
     return -1;
  }
  catch(const std::exception &ex) {
     cerr<<"std::exception has been caught. "<<ex.what()<<endl;
     return -1;
  }
  catch(...) {
     cerr<<"An unexpected exception has been caught"<<endl;
     return -1;
  }
  return 0;
}
