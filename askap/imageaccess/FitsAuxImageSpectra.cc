/// @file FitsAuxImageSpectra.cc
/// @brief Access FITS image
/// @details This class implements IImageAccess interface for FITS image
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
/// @author 
///

#include <askap/imageaccess/FitsAuxImageSpectra.h>
#include <askap/imageaccess/Utils.h>
#include <askap/askap/AskapLogging.h>

using namespace askap;
using namespace askap::accessors;

ASKAP_LOGGER(FITSlogger, ".FitsAuxImageSpectra");

/// class static function
void FitsAuxImageSpectra::PrintError(int status)
{
    /*****************************************************/
    /* Print out cfitsio error messages and exit program */
    /*****************************************************/

    char status_str[FLEN_STATUS];
    fits_get_errstatus(status, status_str);

    if (status) {
        ASKAPLOG_ERROR_STR(FITSlogger, "FitsIO error: " << status_str); /* print error report */

        exit(status);      /* terminate the program, returning error status */
    }
}

FitsAuxImageSpectra::FitsAuxImageSpectra(const std::string& fitsFileName, 
                                         const casacore::RecordInterface &tableInfo,
                                         const int nChannels, const int nrows)
    : itsStatus(0), itsName(fitsFileName), itsNChannels(nChannels)
{
    if (fits_create_file(&itsFitsPtr, fitsFileName.c_str(), &itsStatus)) /* create new FITS file */
         PrintError(itsStatus);           /* call printerror if error occurs */        

    // create a dummy image
    int bitpix = USHORT_IMG; /* 16-bit unsigned short pixel values       */
    long naxis    =   2;  /* 2-dimensional image                            */
    long naxes[2] = { 2, 2 };   /* image is 1 pixels wide by 1 rows */
    
    if ( fits_create_img(itsFitsPtr,  bitpix, naxis, naxes, &itsStatus) )
         PrintError(itsStatus);

    // 2x2 array ie 2 rows and 2 columns
    unsigned short array[2][2] = {{65535,65535},{65535,65535}};
    long fpixel = 1;
    long nelements = naxes[0] * naxes[1];
    if ( fits_write_img(itsFitsPtr, TUSHORT, fpixel, nelements, array[0], &itsStatus) )
        PrintError(itsStatus);

    create(tableInfo,nChannels,nrows);

    if ( fits_close_file(itsFitsPtr,&itsStatus) )       /* close the FITS file */
         PrintError(itsStatus);
}

void 
FitsAuxImageSpectra::create(const casacore::RecordInterface &tableInfo,
                            const int nChannels, const int nrows)
{
    const char extname[] = "Stoke";

    // check to see if the user specifies the column name for the stoke
    casacore::String stoke("spectrum"); // spectrum for I,Q,U and V
    if ( tableInfo.isDefined("Stoke") ) {
        casacore::RecordFieldId fieldId(tableInfo.fieldNumber("Stoke"));
        tableInfo.get(fieldId,stoke);
    }
    unsigned int nCol = 2;
    CPointerWrapper cPointerWrapper { nCol };
    std::string col1 = "Id";
    cPointerWrapper.itsTType[0] = new char[sizeof(char) * col1.length() + 1];
    std::memset(cPointerWrapper.itsTType[0],'\0',col1.length() + 1);
    std::memcpy(cPointerWrapper.itsTType[0],col1.data(),col1.length());
    cPointerWrapper.itsTForm[0] = new char[sizeof(char)*4];
    std::memset(cPointerWrapper.itsTForm[0],'\0',4);
    std::memcpy(cPointerWrapper.itsTForm[0],"20A",3);
    cPointerWrapper.itsUnits[0] = new char[2];
    std::memset(cPointerWrapper.itsUnits[0],'\0',2);
    std::memcpy(cPointerWrapper.itsUnits[0],"",1);

    std::string col2(stoke.data());
    cPointerWrapper.itsTType[1] = new char[sizeof(char) * col2.length() + 1];
    std::memset(cPointerWrapper.itsTType[1],'\0',col2.length() + 1);
    std::memcpy(cPointerWrapper.itsTType[1],col2.data(),col2.length());
    // Dont know why QE does not work on galaxy. It always shows QE(1)
    // instead of QE(288) in fv
    //std::string s = std::string("QE(") + std::to_string(nChannels) + ")";
    std::string s = std::to_string(nChannels) +  std::string("E");
    cPointerWrapper.itsTForm[1] = new char[s.length()+1];
    std::memset(cPointerWrapper.itsTForm[1],'\0',s.length()+1);
    std::memcpy(cPointerWrapper.itsTForm[1],s.data(),s.length());
    cPointerWrapper.itsUnits[1] = new char[2];
    std::memset(cPointerWrapper.itsUnits[1],'\0',2);
    std::memcpy(cPointerWrapper.itsUnits[1],"",1);

    
    itsStatus = 0;

    int tfields = 2;
    auto t = cPointerWrapper.itsTType;
    auto f = cPointerWrapper.itsTForm;
    auto u = cPointerWrapper.itsUnits;

//char *tform[] = { "a8",     "288E"};
    if ( fits_create_tbl(itsFitsPtr, BINARY_TBL, nrows, tfields,t,f,u,extname,&itsStatus) )
         PrintError(itsStatus);    

}

void 
FitsAuxImageSpectra::add(const std::string& id, const SpectrumT& spectrum,
                         const unsigned int startingRow)
{
    itsStatus = 0;
    // open the file again
    if ( fits_open_file(&itsFitsPtr,itsName.c_str(), READWRITE, &itsStatus) )
         PrintError(itsStatus);

    // move to the HDU 2 which is the spectrum binary table
    int hduType;
    if ( fits_movabs_hdu(itsFitsPtr,spectrumHDU(),&hduType,&itsStatus) )
         PrintError(itsStatus);

    long firstElem = 1;
    char* bptr[] = {const_cast<char *> (id.c_str())};
    if (fits_write_col(itsFitsPtr,TSTRING,1,startingRow,firstElem,1,
                       bptr,&itsStatus) )
        PrintError(itsStatus);

    if (fits_write_col(itsFitsPtr,TFLOAT,2,startingRow,firstElem,spectrum.size(),
                       const_cast<float *> (spectrum.data()),&itsStatus) )
        PrintError(itsStatus);

    // close the FITS file
    if (fits_close_file(itsFitsPtr,&itsStatus))
         PrintError(itsStatus);
}

void
FitsAuxImageSpectra::get(const long row, SpectrumT& spectrum)
{
    itsStatus = 0;
    // open the file again
    if ( fits_open_file(&itsFitsPtr,itsName.c_str(), READWRITE, &itsStatus) )
         PrintError(itsStatus);

    // move to the HDU 2 which is the spectrum binary table
    int hduType;
    if ( fits_movabs_hdu(itsFitsPtr,spectrumHDU(),&hduType,&itsStatus) )
         PrintError(itsStatus);

    float fnull = 0.0;    
    int anynull = 0;
    const int spectrumCol = 2; // for now
    const long felem = 1;
    const long nelem = itsNChannels;
    
    spectrum.resize(itsNChannels);
    float* data = const_cast<float *> (spectrum.data());
    if (fits_read_col(itsFitsPtr,TFLOAT,spectrumCol,row,felem,nelem,&fnull,data,&anynull,&itsStatus))
        PrintError(itsStatus);

    // close the FITS file
    if (fits_close_file(itsFitsPtr,&itsStatus))
         PrintError(itsStatus);
}

//        void addI(const std::string& fitsFileName, const std::string& id, const std::vector<float>& spectrum) override;
