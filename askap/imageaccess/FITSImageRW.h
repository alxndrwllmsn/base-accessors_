/// @file FITSImageRW.h
/// @brief Read/Write FITS image class
/// @details This class implements the write methods that are absent
/// from the casacore FITSImage.
///
///
/// @copyright (c) 2016 CSIRO
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
/// @author Stephen Ord <stephen.ord@csiro.au
///
#ifndef ASKAP_ACCESSORS_FITS_IMAGE_RW_H
#define ASKAP_ACCESSORS_FITS_IMAGE_RW_H

#include <casacore/images/Images/FITSImage.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/DataType.h>
#include <casacore/fits/FITS/fitsio.h>
#include <casacore/casa/Containers/RecordInterface.h>

#include <Common/ParameterSet.h>
#include <askap/imageaccess/IImageAccess.h>

#include <sstream>
#include <tuple>
#include <map>
#include "boost/scoped_ptr.hpp"

namespace askap {
namespace accessors {

/// @brief Extend FITSImage class functionality
/// @details It is made clear in the casacore implementation that there are
/// difficulties in writing general FITS access routines for writing.
/// I will implement what ASKAP needs here
/// @ingroup imageaccess


extern bool created;

class FITSImageRW {

    public:

        FITSImageRW();

        FITSImageRW(const std::string &name);

        /// @brief create a new FITS image
        /// @details A call to this method should preceed any write calls. The actual
        /// image may be created only upon the first write call. Details depend on the
        /// implementation.


        bool create(const std::string &name, const casacore::IPosition &shape, \
                    const casacore::CoordinateSystem &csys, \
                    uint memoryInMB = 64, \
                    bool preferVelocity = false, \
                    bool opticalVelocity = true, \
                    int BITPIX = -32, \
                    float minPix = 1.0, \
                    float maxPix = -1.0, \
                    bool degenerateLast = false, \
                    bool verbose = true, \
                    bool stokesLast = false, \
                    bool preferWavelength = false, \
                    bool airWavelength = false, \
                    bool primHead = true, \
                    bool allowAppend = false, \
                    bool history = true);

        // Destructor does nothing
        virtual ~FITSImageRW();

        bool create();

        void print_hdr();
        void setUnits(const std::string &units);

        void setHeader(const std::string &keyword, const std::string &value, const std::string &desc);
        void setHeader(const LOFAR::ParameterSet & keywords);

        void setRestoringBeam(double, double, double);
        void setRestoringBeam(const BeamList& beamlist);
        void addHistory(const std::string &history);
        void addHistory(const std::vector<std::string> &historyLines);

        // write into a FITS image
        bool write(const casacore::Array<float>&);
        bool write(const casacore::Array<float> &arr, const casacore::IPosition &where);

        /// @brief this method is the implementation of the interface FitsImageAccess::setInfo()
        /// @see the description in FitsImageAccess::setInfo() for details.
        /// @param[in] the top level casacore::Record object.
        void setInfo(const casacore::RecordInterface &info);
        void getInfo(const std::string& filename, const std::string& tblName,casacore::RecordInterface &info);
    private:

        /// @brief this structure wraps the c pointers required by cfitsio library to ensure
        ///        memory used is properly freed.
        struct CPointerWrapper 
        {
            friend class FITSImageRW;

            explicit CPointerWrapper(unsigned int numColumns);
            ~CPointerWrapper();

            unsigned int itsNumColumns;
            char** itsTType;
            char** itsTForm;
            char** itsUnits;
        };

        /// @brief check the given info object confirms to the requirements specified in the setInfo() method
        /// @param[in] info  the casacore::Record object to be validated.
        void setInfoValidityCheck(const casacore::RecordInterface &info);

        using TableKeywordInfo = std::tuple<std::string, // keyword name
                                            std::string, // keyword value
                                            std::string>; // keyword comment

        /// @brief a helper method to parse the casacore::Record and collect the keywords
        /// @param[in] info  a casacore::Record contains the keywords and table columns data
        /// @param[out] tableKeyworda  a map of keywords with their vaues and comments (optional)
        void getTableKeywords(const casacore::RecordInterface& info,
                              std::map<std::string,TableKeywordInfo>& tableKeywords);

        /// @brief a helper method to write the keywords to FITS binary table
        /// @param[in] fptr  FITS file pointer. The file must be opened for writting before calling this 
        ///                  method. It does not close the file pointer after the call
        /// @param[in] tableKeywords  a map of FITS keywords to be written to the FITS table
        void writeTableKeywords(fitsfile* fptr, std::map<std::string,TableKeywordInfo>& tableKeywords);

        /// @brief a helper method to write the casacore::Record to the FITS binary table columns.
        /// @param[in] fptr  FITS file pointer. The file must be opened for writting before calling this
        ///                  method. It does not close the file pointer after the call.
        /// @param[in] table a casacore::Record contains the columns' data to be written FITS binary table.
        ///                  The Record (table) must confirm to the format outlined in theFitsImageAccess:: setInfo() method.
        void writeTableColumns(fitsfile* fptr,  const casacore::RecordInterface &table);

        /// @brief this method creates and writes the keywords and table data stored in the casacore::Record 
        ///        to the FITS binary table.
        /// @param[in] info  keywords and table data kept in the casacore::Record
        void createTable(const casacore::RecordInterface &info);

        template<typename T>
        void addColToRecord(const std::string& columnName,
                            long nelem, const T* data,
                            casacore::Record& table) 
        {
            casacore::Vector<T> casaVector(nelem);
            for (std::size_t index = 0; index < nelem; index++) {
                casaVector[index] = data[index];
            }
            table.define(columnName,casaVector);
        }

        std::string name;
        casacore::IPosition shape;
        casacore::CoordinateSystem csys;
        uint memoryInMB;
        bool preferVelocity;
        bool opticalVelocity;
        int BITPIX;
        float minPix;
        float maxPix;
        bool degenerateLast;
        bool verbose;
        bool stokesLast;
        bool preferWavelength;
        bool airWavelength;
        bool primHead;
        bool allowAppend;
        bool history;

        casacore::FitsKeywordList theKeywordList;

};
}
}
#endif
