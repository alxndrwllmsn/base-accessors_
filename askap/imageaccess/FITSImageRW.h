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
        casacore::Vector<casacore::Quantity> getRestoringBeam() const;

        void addHistory(const std::vector<std::string> &historyLines);

        // write into a FITS image
        bool write(const casacore::Array<float>&);
        bool write(const casacore::Array<float> &arr, const casacore::IPosition &where);

        /// @brief this method is the implementation of the interface FitsImageAccess::setInfo()
        /// @see the description in FitsImageAccess::setInfo() for details.
        /// @param[in] the top level casacore::Record object.
        void setInfo(const casacore::RecordInterface &info);

        /// @brief this method is the implementation of the interface FitsImageAccess::getInfo()
        /// @see the description in FitsImageAccess::getInfo() for details.
        /// @param[in] tbleName  name of the table in the FITS file. tbleName = "All" gets all the tables in
        ///                      the FITS file.
        /// @param[in] the top level casacore::Record object.
        void getInfo(const std::string& tblName, casacore::RecordInterface &info) const;
    private:

        /// @brief this structure wraps the c pointers required by cfitsio library to ensure
        ///        memory used is properly freed.
        struct CPointerWrapper
        {
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

        /// @brief a helper method to write the keywords to FITS binary table
        /// @param[in] fptr  FITS file pointer. The file must be opened for writting before calling this
        ///                  method. It does not close the file pointer after the call
        /// @param[in] info  a casacore::Record contains the keywords and table columns data
        void writeTableKeywords(fitsfile* fptr, const casacore::RecordInterface& info);

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

        /// @brief this method gets the FITS table column that contains string data.
        /// param[in] fptr - fits file pointer. Must be opened before calling this method.
        /// param[in] columnName - name of the column
        /// param[in] columnNum - column number of the FITS table
        /// param[in] frow - first row
        /// param[in] felem - first element
        /// param[in] nelem - number of road to read
        /// param[in] strnull
        /// param[in] anynull
        /// param[in] status - status of the fits call
        /// param[out] table - casacore::Record to store the FITS binary table data
        void getStringColumnType(fitsfile* fptr,const std::string& columnName,
                                 long columnNum,long frow,long felem,long nelem,
                                 char* strnull, int& anynull, int& status,
                                 char** stringArrayValues,casacore::Record& table) const;

        /// @brief helper method. It copies FITS table data to casacore::Record
        /// param[in] fptr - fits file pointer. Must be opened before calling this method.
        /// param[in] nelem - number of road to read
        /// param[in] numColumns - column to read
        /// param[in] cPtrWrapper - wrapper class for c pointer
        /// param[in] status - status of the fits call
        /// param[out] table - casacore::Record to store the FITS binary table data
        void copyFitsToCasa(fitsfile* fptr,long nelem, long numColumns,
                            CPointerWrapper& cPtrWrapper,int& status,
                            casacore::Record& table) const;

        /// @brief copy the FITS binary table keywords to casacore:Record
        /// param[in] fptr - fits file pointer. Must be opened before calling this method.
        /// param[out] table - casacore::Record to store the FITS binary table keywords
        /// param[in] status - status of the fits call
        void copyTableExtKeywords(fitsfile* ptr, casacore::Record& table, int& status) const;

        /// @brief extract a FITS record (i.e keyword name, keyword value and comment in a string)
        /// param[in] record - FITS record consisting of keyword name, keyword value and comment in a string
        /// param[out] keyword - FITS keyword name
        /// param[out] value - FITS keyword value
        /// param[out] comment - FITS keyword comment
        void extractFitsRecord(const std::string& record, std::string& keyword,
                               std::string& value, std::string& comment) const;

        /// @brief a helper template method to copy table column data to casacore::Record
        /// param[in] fptr - fits file pointer. Must be opened before calling this method.
        /// param[in] columnName - name of the column
        /// param[in] datatype - FITS column datatype e.g TSTRING, TINT and etc
        /// param[in] columnNum - column number of the FITS table
        /// param[in] frow - first row
        /// param[in] felem - first element
        /// param[in] nelem - number of road to read
        /// param[in] strnull
        /// param[in] anynull
        /// param[in] status - status of the fits call
        /// param[out] table - casacore::Record to store the FITS binary table data
        template<typename T>
        bool getColumnData(fitsfile* fptr,const std::string& columnName,int datatype,
                           long columnNum,long frow,long felem,long nelem,
                           char* strnull, int& anynull, int& status,
                           casacore::Record& table) const
        {
            boost::shared_array<T> tArray {new T[nelem]};
            fits_read_col(fptr,datatype,columnNum,frow,felem,nelem,strnull,
                tArray.get(), &anynull, &status);

            //this->addColToRecord<T>(columnName,nelem,tArray.get(),table);
            bool r = (status == 0 ? true : false);

            casacore::Vector<T> casaVector(nelem);
            const T* data = tArray.get();
            for (std::size_t index = 0; index < nelem; index++) {
                casaVector[index] = data[index];
            }
            table.define(columnName,casaVector);

            return r;
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
