/// @file FITSImageRW.cc
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
#include <askap_accessors.h>
#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapUtil.h>
#include <casacore/images/Images/FITSImage.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/DataType.h>
#include <casacore/fits/FITS/fitsio.h>
#include <casacore/fits/FITS/FITSDateUtil.h>
#include <casacore/fits/FITS/FITSHistoryUtil.h>
#include <casacore/fits/FITS/FITSReader.h>

#include <casacore/casa/Quanta/MVTime.h>
#include <askap/imageaccess/FITSImageRW.h>

#include <boost/shared_array.hpp>
#include <fitsio.h>
#include <iostream>
#include <fstream>

ASKAP_LOGGER(FITSlogger, ".FITSImageRW");

void printerror(int status)
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
    return;
}

using namespace askap;
using namespace askap::accessors;
FITSImageRW::CPointerWrapper::CPointerWrapper(unsigned int numColumns)
    : itsNumColumns(numColumns),
      itsTType  { new char* [sizeof(char*) * numColumns] },
      itsTForm { new char* [sizeof(char*) * numColumns] },
      //itsUnits { new char* [sizeof(char*) * numColumns] }
      itsUnits { nullptr }
{
}

FITSImageRW::CPointerWrapper::~CPointerWrapper()
{
    for ( unsigned int i = 0; i < itsNumColumns; i++ ) {
    {
        if ( itsTType[i] != nullptr ) {
            delete [] itsTType[i];
        }
        if ( itsTForm[i] != nullptr ) {
            delete [] itsTForm[i];
        }
        if ( itsUnits[i] != nullptr )
            delete [] itsUnits[i];
        }
    }

    if ( itsTType != nullptr ) {
        delete [] itsTType;
    }
    if ( itsTForm != nullptr ) {
        delete [] itsTForm;
    }
    if ( itsUnits != nullptr ) {
        delete [] itsUnits;
    }
}
///////////////////////////////////////////////////
FITSImageRW::FITSImageRW(const std::string &name)
{
    std::string fullname = name + ".fits";
    this->name = std::string(name.c_str());
}
FITSImageRW::FITSImageRW()
{

}
bool FITSImageRW::create(const std::string &name, const casacore::IPosition &shape, \
                         const casacore::CoordinateSystem &csys, \
                         uint memoryInMB, bool preferVelocity, \
                         bool opticalVelocity, int BITPIX, float minPix, float maxPix, \
                         bool degenerateLast, bool verbose, bool stokesLast, \
                         bool preferWavelength, bool airWavelength, bool primHead, \
                         bool allowAppend, bool history)
{

    std::string fullname = name + ".fits";

    this->name = std::string(fullname.c_str());
    this->shape = shape;
    this->csys = csys;
    this->memoryInMB = memoryInMB;
    this->preferVelocity = preferVelocity;
    this->opticalVelocity = opticalVelocity;
    this->BITPIX = BITPIX;
    this->minPix = minPix;
    this->maxPix = maxPix;
    this->degenerateLast = degenerateLast;
    this->verbose = verbose ;
    this->stokesLast = stokesLast;
    this->preferWavelength = preferWavelength;
    this->airWavelength = airWavelength;
    this->primHead = primHead;
    this->allowAppend = allowAppend;
    this->history = history;

    ASKAPLOG_DEBUG_STR(FITSlogger, "Creating R/W FITSImage " << this->name);

    unlink(this->name.c_str());
    std::ofstream outfile(this->name.c_str());
    ASKAPCHECK(outfile.is_open(), "Cannot open FITS file for output");
    ASKAPLOG_INFO_STR(FITSlogger, "Created Empty R/W FITSImage " << this->name);
    ASKAPLOG_DEBUG_STR(FITSlogger, "Generating FITS header");


    casacore::String error;
    const casacore::uInt ndim = shape.nelements();
    // //
    // // Find scale factors
    // //
    casacore::Record header;
    casacore::Double b_scale, b_zero;
    ASKAPLOG_DEBUG_STR(FITSlogger, "Created blank FITS header");
    if (BITPIX == -32) {

        b_scale = 1.0;
        b_zero = 0.0;
        header.define("bitpix", BITPIX);
        header.setComment("bitpix", "Floating point (32 bit)");

    }

    else {
        error =
            "BITPIX must be -32 (floating point)";
        return false;
    }
    ASKAPLOG_DEBUG_STR(FITSlogger, "Added BITPIX");
    //
    // At this point, for 32 floating point, we must apply the given
    // mask.  For 16bit, we may know that there are in fact no blanks
    // in the image, so we can dispense with looking at the mask again.


    //
    casacore::Vector<casacore::Int> naxis(ndim);
    casacore::uInt i;
    for (i = 0; i < ndim; i++) {
        naxis(i) = shape(i);
    }
    header.define("naxis", naxis);

    ASKAPLOG_DEBUG_STR(FITSlogger, "Added NAXES");
    if (allowAppend)
        header.define("extend", casacore::True);
    if (!primHead) {
        header.define("PCOUNT", 0);
        header.define("GCOUNT", 1);
    }
    ASKAPLOG_DEBUG_STR(FITSlogger, "Extendable");

    header.define("bscale", b_scale);
    header.setComment("bscale", "PHYSICAL = PIXEL*BSCALE + BZERO");
    header.define("bzero", b_zero);
    ASKAPLOG_DEBUG_STR(FITSlogger, "BSCALE");

    header.define("COMMENT1", ""); // inserts spaces
    // I should FITS-ize the units

    header.define("BUNIT", "Jy");
    header.setComment("BUNIT", "Brightness (pixel) unit");
    //
    ASKAPLOG_DEBUG_STR(FITSlogger, "BUNIT");
    casacore::IPosition shapeCopy = shape;
    casacore::CoordinateSystem cSys = csys;

    casacore::Record saveHeader(header);
    ASKAPLOG_DEBUG_STR(FITSlogger, "Saved header");
    casacore::Bool ok = cSys.toFITSHeader(header, shapeCopy, casacore::True, 'c', casacore::True, // use WCS
                                      preferVelocity, opticalVelocity,
                                      preferWavelength, airWavelength);
    if (!ok) {
        ASKAPLOG_WARN_STR(FITSlogger, "Could not make a standard FITS header. Setting" \
                          <<  " a simple linear coordinate system.") ;

        casacore::uInt n = cSys.nWorldAxes();
        casacore::Matrix<casacore::Double> pc(n, n); pc = 0.0; pc.diagonal() = 1.0;
        casacore::LinearCoordinate linear(cSys.worldAxisNames(),
                                      cSys.worldAxisUnits(),
                                      cSys.referenceValue(),
                                      cSys.increment(),
                                      cSys.linearTransform(),
                                      cSys.referencePixel());
        casacore::CoordinateSystem linCS;
        linCS.addCoordinate(linear);

        // Recover old header before it got mangled by toFITSHeader

        header = saveHeader;
        casacore::IPosition shapeCopy = shape;
        casacore::Bool ok = linCS.toFITSHeader(header, shapeCopy, casacore::True, 'c', casacore::False); // don't use WCS
        if (!ok) {
            ASKAPLOG_WARN_STR(FITSlogger, "Fallback linear coordinate system fails also.");
            return false;
        }
    }
    ASKAPLOG_DEBUG_STR(FITSlogger, "Added coordinate system");
    // When this if test is True, it means some pixel axes had been removed from
    // the coordinate system and degenerate axes were added.

    if (naxis.nelements() != shapeCopy.nelements()) {
        naxis.resize(shapeCopy.nelements());
        for (casacore::uInt j = 0; j < shapeCopy.nelements(); j++) {
            naxis(j) = shapeCopy(j);
        }
        header.define("NAXIS", naxis);
    }

    //
    // DATE
    //

    casacore::String date, timesys;
    casacore::Time nowtime;
    casacore::MVTime now(nowtime);
    casacore::FITSDateUtil::toFITS(date, timesys, now);
    header.define("date", date);
    header.setComment("date", "Date FITS file was written");
    if (!header.isDefined("timesys") && !header.isDefined("TIMESYS")) {
        header.define("timesys", timesys);
        header.setComment("timesys", "Time system for HDU");
    }

    ASKAPLOG_DEBUG_STR(FITSlogger, "Added date");
    // //
    // // ORIGIN
    // //

    header.define("ORIGIN", "ASKAPsoft");

    theKeywordList = casacore::FITSKeywordUtil::makeKeywordList(primHead, casacore::True);

    //kw.mk(FITS::EXTEND, True, "Tables may follow");
    // add the general keywords for WCS and so on
    ok = casacore::FITSKeywordUtil::addKeywords(theKeywordList, header);
    if (! ok) {
        error = "Error creating initial FITS header";
        return false;
    }


    //
    // END
    //

    theKeywordList.end();
    ASKAPLOG_DEBUG_STR(FITSlogger, "All keywords created ... adding to file");
    // now get them into a file ...

    theKeywordList.first();
    theKeywordList.next(); // skipping an extra SIMPLE... hack
    casacore::FitsKeyCardTranslator m_kc;
    const size_t cards_size = 2880 * 4;
    char cards[cards_size];
    memset(cards, 0, sizeof(cards));
    while (1) {
        if (m_kc.build(cards, theKeywordList)) {

            outfile << cards;
            memset(cards, 0, sizeof(cards));
        } else {
            if (cards[0] != 0) {
                outfile << cards;
            }
            break;
        }

    }
    // outfile << cards;
    ASKAPLOG_DEBUG_STR(FITSlogger, "All keywords added to file");
    try {
      outfile.close();
      ASKAPLOG_DEBUG_STR(FITSlogger, "Outfile closed");
    }
    catch (...) {
      ASKAPLOG_WARN_STR(FITSlogger, "Failed to properly close outfile");
      return false;
    }



    return true;

}
void FITSImageRW::print_hdr()
{
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */

    int status, nkeys, keypos, hdutype, ii, jj;
    char card[FLEN_CARD];   /* standard string lengths defined in fitsioc.h */

    status = 0;

    if (fits_open_file(&fptr, this->name.c_str(), READONLY, &status))
        printerror(status);

    /* attempt to move to next HDU, until we get an EOF error */
    for (ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status)); ii++) {
        /* get no. of keywords */
        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status))
            printerror(status);

        printf("Header listing for HDU #%d:\n", ii);
        for (jj = 1; jj <= nkeys; jj++)  {
            if (fits_read_record(fptr, jj, card, &status))
                printerror(status);

            printf("%s\n", card); /* print the keyword card */
        }
        printf("END\n\n");  /* terminate listing with END */
    }

    if (status == END_OF_FILE)   /* status values are defined in fitsioc.h */
        status = 0;              /* got the expected EOF error; reset = 0  */
    else
        printerror(status);       /* got an unexpected error                */

    if (fits_close_file(fptr, &status))
        printerror(status);

    return;

}
bool FITSImageRW::write(const casacore::Array<float> &arr)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Writing array to FITS image");
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */


    int status = 0;

    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);

    long fpixel = 1;                               /* first pixel to write      */
    size_t nelements = arr.nelements();          /* number of pixels to write */
    bool deleteIt;
    const float *data = arr.getStorage(deleteIt);
    void *dataptr = (void *) data;

    /* write the array of unsigned integers to the FITS file */
    if (fits_write_img(fptr, TFLOAT, fpixel, nelements, dataptr, &status))
        printerror(status);

    if (fits_close_file(fptr, &status))
        printerror(status);

    return true;
}


bool FITSImageRW::write(const casacore::Array<float> &arr, const casacore::IPosition &where)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Writing array to FITS image at (Cindex)" << where);
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */

    int status, hdutype;


    status = 0;

    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);

    if (fits_movabs_hdu(fptr, 1, &hdutype, &status))
        printerror(status);

    // get the dimensionality & size of the fits file.
    int naxes;
    if (fits_get_img_dim(fptr, &naxes, &status)) {
        printerror(status);
    }
    long *axes = new long[naxes];
    if (fits_get_img_size(fptr, naxes, axes, &status)) {
        printerror(status);
    }

    ASKAPCHECK(where.nelements() == naxes,
               "Mismatch in dimensions - FITS file has " << naxes
               << " axes, while requested location has " << where.nelements());

    long fpixel[4], lpixel[4];
    int array_dim = arr.shape().nelements();
    int location_dim = where.nelements();
    ASKAPLOG_DEBUG_STR(FITSlogger, "There are " << array_dim << " dimensions in the slice");
    ASKAPLOG_DEBUG_STR(FITSlogger," There are " << location_dim << " dimensions in the place");

    fpixel[0] = where[0] + 1;
    lpixel[0] = where[0] + arr.shape()[0];
    ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[0] = " << fpixel[0] << ", lpixel[0] = " << lpixel[0]);
    fpixel[1] = where[1] + 1;
    lpixel[1] = where[1] + arr.shape()[1];
    ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[1] = " << fpixel[1] << ", lpixel[1] = " << lpixel[1]);

    if (array_dim == 2 && location_dim >= 3) {
        ASKAPLOG_DEBUG_STR(FITSlogger,"Writing a single slice into an array");
        fpixel[2] = where[2] + 1;
        lpixel[2] = where[2] + 1;
        ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[2] = " << fpixel[2] << ", lpixel[2] = " << lpixel[2]);
        if (location_dim == 4) {
            fpixel[3] = where[3] + 1;
            lpixel[3] = where[3] + 1;
            ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[3] = " << fpixel[3] << ", lpixel[3] = " << lpixel[3]);
        }
    }
    else if (array_dim == 3 && location_dim >= 3) {
        ASKAPLOG_DEBUG_STR(FITSlogger,"Writing more than 1 slice into the array");
        fpixel[2] = where[2] + 1;
        lpixel[2] = where[2] + arr.shape()[2];
        ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[2] = " << fpixel[2] << ", lpixel[2] = " << lpixel[2]);
        if (location_dim == 4) {
            fpixel[3] = where[3] + 1;
            lpixel[3] = where[3] + 1;
            ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[3] = " << fpixel[3] << ", lpixel[3] = " << lpixel[3]);
        }
    }
    else if (array_dim == 4 && location_dim == 4) {
        fpixel[2] = where[2] + 1;
        lpixel[2] = where[2] + arr.shape()[2];
        ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[2] = " << fpixel[2] << ", lpixel[2] = " << lpixel[2]);
        fpixel[3] = where[3] + 1;
        lpixel[3] = where[3] + arr.shape()[3];
        ASKAPLOG_DEBUG_STR(FITSlogger, "fpixel[3] = " << fpixel[3] << ", lpixel[3] = " << lpixel[3]);
    }


    int64_t nelements = arr.nelements();          /* number of pixels to write */

    ASKAPLOG_DEBUG_STR(FITSlogger, "We are writing " << nelements << " elements");
    bool deleteIt = false;
    const float *data = arr.getStorage(deleteIt);
    float *dataptr = (float *) data;

    // status = 0;

    // if ( fits_write_pix(fptr, TFLOAT,fpixel, nelements, dataptr, &status) )
    //     printerror( status );

    status = 0;
    long group = 0;

    if (fits_write_subset_flt(fptr, group, naxes, axes, fpixel, lpixel, dataptr, &status))
        printerror(status);

    ASKAPLOG_INFO_STR(FITSlogger, "Written " << nelements << " elements");
    status = 0;

    if (fits_close_file(fptr, &status))
        printerror(status);

    delete [] axes;

    return true;

}
void FITSImageRW::setUnits(const std::string &units)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Updating brightness units");
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;

    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);

    if (fits_update_key(fptr, TSTRING, "BUNIT", (void *)(units.c_str()),
                        "Brightness (pixel) unit", &status))
        printerror(status);

    if (fits_close_file(fptr, &status))
        printerror(status);

}

void FITSImageRW::setHeader(const std::string &keyword, const std::string &value, const std::string &desc)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Setting header value for " << keyword);
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;
    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);


    if (fits_update_key(fptr, TSTRING, keyword.c_str(), (char *)value.c_str(),
                        desc.c_str(), &status))
        printerror(status);

    if (fits_close_file(fptr, &status))
        printerror(status);
}

void FITSImageRW::setHeader(const LOFAR::ParameterSet & keywords)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Setting header values from parset ");
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;
    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status)) {
        printerror(status);
    }

    for (auto &elem : keywords) {
      const string keyword = elem.first;
      const std::vector<string> valanddesc = elem.second.getStringVector();
      if (valanddesc.size() > 0) {
        const string value = valanddesc[0];
        const string desc = (valanddesc.size() > 1 ? valanddesc[1] : "");

        const string type = (valanddesc.size() > 2 ? toUpper(valanddesc[2]) : "STRING");
        if (type == "INT") {
          try {
            int intVal = std::stoi(value);
            if (fits_update_key(fptr, TINT, keyword.c_str(), &intVal,
            desc.c_str(), &status)) {
              printerror(status);
            }
          } catch (const std::invalid_argument&) {
            ASKAPLOG_WARN_STR(FITSlogger, "Invalid int value for header keyword "<<keyword<<" : "<<value);
          } catch (const std::out_of_range&) {
            ASKAPLOG_WARN_STR(FITSlogger, "Out of range int value for header keyword "<<keyword<<" : "<<value);
          }
        } else if (type == "DOUBLE") {
          try {
            double doubleVal = std::stod(value);
            if (fits_update_key(fptr, TDOUBLE, keyword.c_str(), &doubleVal,
            desc.c_str(), &status)) {
              printerror(status);
            }
          } catch (const std::invalid_argument&) {
            ASKAPLOG_WARN_STR(FITSlogger, "Invalid double value for header keyword "<<keyword<<" : "<<value);
          } catch (const std::out_of_range&) {
            ASKAPLOG_WARN_STR(FITSlogger, "Out of range double value for header keyword "<<keyword<<" : "<<value);
          }
        } else if (type == "STRING") {
          if (fits_update_key(fptr, TSTRING, keyword.c_str(), (char *)value.c_str(),
          desc.c_str(), &status)) {
            printerror(status);
          }
        } else {
          ASKAPLOG_WARN_STR(FITSlogger, "Invalid type for header keyword "<<keyword<<" : "<<type);
        }

      }
    }

    if (fits_close_file(fptr, &status)) {
        printerror(status);
    }
}

void FITSImageRW::setRestoringBeam(double maj, double min, double pa)
{
    ASKAPLOG_INFO_STR(FITSlogger, "Setting Beam info");
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;
    double radtodeg = 360. / (2 * M_PI);
    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);

    double value = radtodeg * maj;
    if (fits_update_key(fptr, TDOUBLE, "BMAJ", &value,
                        "Restoring beam major axis", &status))
        printerror(status);
    value = radtodeg * min;
    if (fits_update_key(fptr, TDOUBLE, "BMIN", &value,
                        "Restoring beam minor axis", &status))
        printerror(status);
    value = radtodeg * pa;
    if (fits_update_key(fptr, TDOUBLE, "BPA", &value,
                        "Restoring beam position angle", &status))
        printerror(status);
    if (fits_update_key(fptr, TSTRING, "BTYPE", (void *) "Intensity",
                        " ", &status))
        printerror(status);

    if (fits_close_file(fptr, &status))
        printerror(status);

}

void FITSImageRW::setRestoringBeam(const BeamList & beamlist)
{
  ASKAPCHECK(!beamlist.empty(),"Called FITSImageRW::setRestoringBeam with empty beamlist");
  // write multiple beams to a binary table
  ASKAPLOG_INFO_STR(FITSlogger, "Writing BEAMS binary table");
  fitsfile *fptr;
  // note status is passed on from each call and calls do not execute if status is non zero on entry
  int status = 0;
  fits_open_file(&fptr, this->name.c_str(), READWRITE, &status);

  // set header keyword to indicate beams table is present
  int present = 1;
  fits_update_key(fptr, TLOGICAL, "CASAMBM", &present, "CASA Multiple beams table present", &status);

  int nchan = beamlist.size();
  int npol = 1;

  const int tfields = 5;
  const char *ttype[] = {"BMAJ", "BMIN", "BPA", "CHAN", "POL"};
  const char *tform[] = {"1E", "1E", "1E", "1J", "1J"};
  const char *tunit[] = {"arcsec", "arcsec", "deg", "\0", "\0"};
  char extname[] = "BEAMS";
  int extver = 1;

  fits_create_tbl(fptr, BINARY_TBL, nchan, tfields, (char**)ttype, (char**)tform, (char**)tunit,
		  extname, &status);
  fits_write_key(fptr, TINT, "EXTVER", &extver, "", &status);
  fits_write_key(fptr, TINT, "NCHAN", &nchan, "Number of channels", &status);
  fits_write_key(fptr, TINT, "NPOL", &npol, "Number of polarisations", &status);

  int row = 0;
  for (const auto& beam : beamlist) {
    row++;
    // can't make these const due to fits_write_col signature
    ASKAPDEBUGASSERT(beam.second.size()==3);
    float bmaj = beam.second[0].getValue("arcsec");
    float bmin = beam.second[1].getValue("arcsec");
    float bpa = beam.second[2].getValue("deg");
    int chan = row - 1;
    int pol = 0;
    fits_write_col(fptr, TFLOAT, 1, row, 1, 1, &bmaj , &status);
    fits_write_col(fptr, TFLOAT, 2, row, 1, 1, &bmin, &status);
    fits_write_col(fptr, TFLOAT, 3, row, 1, 1, &bpa, &status);
    fits_write_col(fptr, TINT, 4, row, 1, 1, &chan, &status);
    fits_write_col(fptr, TINT, 5, row, 1, 1, &pol, &status);
  }
  fits_close_file(fptr, &status);

  if (status) {
    printerror(status);
  }
}

void FITSImageRW::addHistory(const std::string &history)
{

    ASKAPLOG_INFO_STR(FITSlogger,"Adding HISTORY string: " << history);
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;
    if ( fits_open_file(&fptr, this->name.c_str(), READWRITE, &status) )
        printerror( status );

    if ( fits_write_history(fptr, history.c_str(), &status) )
        printerror( status );

    if ( fits_close_file(fptr, &status) )
        printerror( status );

}

void FITSImageRW::addHistory(const std::vector<std::string> &historyLines)
{
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status = 0;
    if ( fits_open_file(&fptr, this->name.c_str(), READWRITE, &status) )
        printerror( status );

    for ( const auto& history : historyLines ) {
        ASKAPLOG_INFO_STR(FITSlogger,"Adding HISTORY string: " << history);
        if ( fits_write_history(fptr, history.c_str(), &status) )
            printerror( status );
    }


    if ( fits_close_file(fptr, &status) )
        printerror( status );

}

FITSImageRW::~FITSImageRW()
{
}


/// @brief check the given info object confirms to the requirements specified in the setInfo() method
/// @param[in] info  the casacore::Record object to be validated.
void FITSImageRW::setInfoValidityCheck(const casacore::RecordInterface &info)
{
    // check info has one and only one sub record
    casacore::uInt subRecordFieldId = 0;
    int numSubRecord = 0;
    casacore::uInt nFields = info.nfields();
    for(casacore::uInt f = 0; f < nFields; f++) {
        std::string name = info.name(f);
        casacore::DataType type = info.dataType(f);
        if ( type == casacore::DataType::TpRecord ) {
            numSubRecord += 1;
            subRecordFieldId = f;
        } else if ( type != casacore::DataType::TpDouble ||
                    type != casacore::DataType::TpString ||
                    type != casacore::DataType::TpFloat ||
                    type != casacore::DataType::TpInt ||
                    type != casacore::DataType::TpUInt ) {
            // check the datatypes of the info object itself.
            // fields that are not subrecord are treated as table keywords and they can only
            // have the following types: string, double, float and int
            std::stringstream ss;
            ss << "field (table keyword) " << name << " has incorrect datatype. Supported datatype are: TpString, TpDouble, TpFloat, TpInt and TpUInt.";
            ASKAPLOG_INFO_STR(FITSlogger,ss.str());
            ASKAPASSERT(false);
        }
    }
    // info should have only one sub record
    ASKAPCHECK(numSubRecord == 1, "info record should have one and only one sub record");

    // check if the subrecord has a "Units" field name
    bool foundUnitsField = false;
    const casacore::RecordInterface& subRec = info.asRecord(subRecordFieldId);
    nFields = subRec.nfields();
    for(casacore::uInt f = 0; f < nFields; f++) {
        if ( subRec.name(f) == "Units" ) {
            foundUnitsField = true;
            break;
        }
    }
    ASKAPCHECK(foundUnitsField == true, "info's subrecord should contain a Units field name");

    // check the fields have the only datatypes supported
    for(casacore::uInt f = 0; f < nFields; f++) {
        casacore::String name = subRec.name(f);
        casacore::DataType type = subRec.dataType(f) ;
        if ( type != casacore::DataType::TpArrayDouble ||
             type != casacore::DataType::TpArrayString ||
             type != casacore::DataType::TpArrayFloat ||
             type != casacore::DataType::TpArrayInt ||
             type != casacore::DataType::TpArrayUInt ) {
             std::stringstream ss;
             ss << "field " << name << " has incorrect datatype. Supported datatype are: TpArrayDouble,  TpArrayString, TpAarrayFloat, TpArrayInt and TpArrayUInt.";
             ASKAPLOG_INFO_STR(FITSlogger,ss.str());
             ASKAPASSERT(false);
        }
    }
}

void FITSImageRW::getTableKeywords(const casacore::RecordInterface& info,
                                       std::map<std::string,TableKeywordInfo>& tableKeywords)
{
    tableKeywords.clear();

    casacore::uInt subRecordFieldId = 0;
    std::string tableName = "";
    const casacore::uInt nFields = info.nfields();
    for(casacore::uInt f = 0; f < nFields; f++) {
        casacore::DataType type = info.dataType(f);
        std::string name = info.name(f);
        std::string comment = info.comment(f);
        if ( type == casacore::DataType::TpRecord ) {
            subRecordFieldId = f;
            tableName = name;
        } else if ( type == casacore::DataType::TpDouble ) {
            double value = 0.0;
            info.get(f,value);
            auto t = std::make_tuple(name,std::to_string(value),comment);
            tableKeywords.emplace(std::make_pair(name,t));
        } else if ( type == casacore::DataType::TpFloat ) {
            float value = 0.0;
            info.get(f,value);
            auto t = std::make_tuple(name,std::to_string(value),comment);
            tableKeywords.emplace(std::make_pair(name,t));
        } else if ( type == casacore::DataType::TpInt ) {
            int value = 0;
            info.get(f,value);
            auto t = std::make_tuple(name,std::to_string(value),comment);
            tableKeywords.emplace(std::make_pair(name,t));
        } else if ( type == casacore::DataType::TpString ) {
            casacore::String value = "";
            info.get(f,value);
            auto t = std::make_tuple(name,value,comment);
            tableKeywords.emplace(std::make_pair(name,t));
        }
    }
}

void FITSImageRW::createTable(const casacore::RecordInterface &info)
{
    // find the sub record. it is the table we want to create
    casacore::uInt nFields = info.nfields();
    casacore::uInt subRecordFieldId = 0;
    std::string tableName = "";
    for(int f = 0; f < nFields; f++) {
        casacore::DataType type = info.dataType(f);
        if ( type == casacore::DataType::TpRecord ) {
            // subRec is a binary table. what should be here are columns info in the binary table
            subRecordFieldId = f;
            tableName = info.name(f);
       }
    }

    const casacore::RecordInterface& table = info.asRecord(subRecordFieldId);

    nFields = table.nfields(); // this is the number of columns in the table
    
    // the subRecord has nfield but we know one of them is a "Units" field
    // which is not part of the table columns
    const casacore::uInt numCol = nFields - 1;
    CPointerWrapper cPointerWrapper { numCol };

    // store number of rows per column
    std::vector<long>  rows;
    for(int f = 0; f < nFields; f++) {
        casacore::String name = table.name(f); // column name
        casacore::DataType type = table.dataType(f); // column datatype
        if ( name != "Units" ) {
            cPointerWrapper.itsTType[f] = new char[sizeof(char)*name.length()];
            std::fill_n(cPointerWrapper.itsTType[f],'\0',name.length());
            std::copy_n(name.data(),name.length(),cPointerWrapper.itsTType[f]);
            // these fields are transformed into fits binary table columns
            // so we have to work out the tfrom and ttype for them
            if ( type == casacore::DataType::TpArrayDouble ) {
                //std::cout << "column name: " << name << ", column type: TpArrayDouble" << std::endl;
                casacore::Array<double> doubleArr;
                table.get(f,doubleArr);
                rows.emplace_back(doubleArr.capacity());

                cPointerWrapper.itsTForm[f] = new char[sizeof(char)*3];
                std::fill_n(cPointerWrapper.itsTForm[f],3,'\0');
                std::copy_n("1D",2,cPointerWrapper.itsTForm[f]);
            } else if ( type == casacore::DataType::TpArrayString ) {
                casacore::Array<casacore::String> stringArr;
                table.get(f,stringArr);
                rows.emplace_back(stringArr.capacity());

                // assume that the column that has string value has max 20 character
                cPointerWrapper.itsTForm[f] = new char[sizeof(char)*5];
                std::fill_n(cPointerWrapper.itsTForm[f],'\0',5);
                std::copy_n("20a",3,cPointerWrapper.itsTForm[f]);
            } else if ( type == casacore::DataType::TpArrayFloat ) {
                casacore::Array<float> floatArr;
                table.get(f,floatArr);
                rows.emplace_back(floatArr.capacity());

                // assume that the column that has string value has max 20 character
                cPointerWrapper.itsTForm[f] = new char[sizeof(char)*3];
                std::fill_n(cPointerWrapper.itsTForm[f],'\0',3);
                std::copy_n("1E",2,cPointerWrapper.itsTForm[f]);
            } else if ( type == casacore::DataType::TpArrayInt ) {
                casacore::Array<int> intArr;
                table.get(f,intArr);
                rows.emplace_back(intArr.capacity());

                // assume that the column that has string value has max 20 character
                cPointerWrapper.itsTForm[f] = new char[sizeof(char)*3];
                std::fill_n(cPointerWrapper.itsTForm[f],'\0',3);
                std::copy_n("1I",2,cPointerWrapper.itsTForm[f]);
            }
        } else {
            // this is a Units field. We expects an array of strings i.e the first unit
            // is the unit for column 1 and so on
            if ( type == casacore::DataType::TpArrayString ) {
                casacore::Array<casacore::String> stringArr;
                table.get(f,stringArr);
                auto numUnits = static_cast<int> (stringArr.capacity());
                std::vector<casacore::String> v = stringArr.tovector();
                cPointerWrapper.itsUnits = new char* [sizeof(char*) * (numUnits - 1)];
                for (int i = 0; i < numUnits; i++) {
                    cPointerWrapper.itsUnits[i] = new char[sizeof(char) * (v[i].length() + 1)];
                    std::fill_n(cPointerWrapper.itsUnits[i],'\0',v[i].length() +1);
                    std::copy_n(v[i].data(),v[i].length(),cPointerWrapper.itsUnits[i]);
                }
            }
        }
    }

    ASKAPCHECK(rows.empty() == false, "FITSImageRW::createTable does not contain any rows of data");

    fitsfile *fptr;
    int status = 0;
    std::vector<long>::iterator maxIter = std::max_element(rows.begin(),rows.end());
    long maxRow = *maxIter;
    
    if (fits_open_file(&fptr, this->name.c_str(), READWRITE, &status))
        printerror(status);

    auto t = cPointerWrapper.itsTType;
    auto f = cPointerWrapper.itsTForm;
    auto u = cPointerWrapper.itsUnits;
    if ( fits_create_tbl( fptr, BINARY_TBL, 0, nFields - 1, t, f, u, tableName.c_str(), &status) )
        printerror(status);

    // write table keywords
    std::map<std::string,TableKeywordInfo> tableKeywords;
    getTableKeywords(info,tableKeywords);
    writeTableKeywords(fptr,tableKeywords);

    // write the table columns
    writeTableColumns(fptr,table);
}

void FITSImageRW::writeTableColumns(fitsfile *fptr, const casacore::RecordInterface &table)
{
    auto nFields = table.nfields();
    

    int status = 0;
    long firstrow = 1;
    long firstelem = 1;
    for(int f = 0; f < nFields; f++) {
        // Write the actual data to the binary table rows and columns
        casacore::String name = table.name(f);
        // Ignore the "Units"field 
        if ( name == "Units" ) continue;

        casacore::DataType type = table.dataType(f);
        if ( type == casacore::DataType::TpArrayDouble ) {
            casacore::Array<double> doubleArr;
            table.get(f,doubleArr);
            long nrows = doubleArr.capacity();
            fits_write_col(fptr, TDOUBLE, f+1, firstrow, firstelem, nrows, doubleArr.data(), &status);
        } else if ( type == casacore::DataType::TpArrayFloat ) {
            casacore::Array<float> floatArr;
            table.get(f,floatArr);
            long nrows = floatArr.capacity();
            fits_write_col(fptr, TFLOAT, f+1, firstrow, firstelem, nrows, floatArr.data(), &status);
        } else if ( type == casacore::DataType::TpArrayInt ) {
            casacore::Array<float> intArr;
            table.get(f,intArr);
            long nrows = intArr.capacity();
            fits_write_col(fptr, TINT, f+1, firstrow, firstelem, nrows, intArr.data(), &status);
        } else if ( type == casacore::DataType::TpArrayString ) {
            casacore::Array<casacore::String> stringArr;
            table.get(f,stringArr);
            long nrows = stringArr.capacity();
            std::vector<casacore::String> v = stringArr.tovector();
            long frow = 1;
            long felem = 1;
            // write the data to the binary table column one cell at a time
            for (long i = 0; i < nrows; i++) {
                boost::shared_array<char> ptr {new char[sizeof(char) * (v[i].length() + 1)]};
                std::fill_n(ptr.get(),v[i].length() + 1,'\0');
                std::copy_n(v[i].data(),v[i].length(),ptr.get());
                char* bptr[] = { ptr.get() };
                if ( fits_write_col(fptr, TSTRING, f+1, frow, firstelem, 1, bptr, &status) ) {
                    printerror(status);
                }
                frow += 1;
            }
        }
    }
}

void FITSImageRW::writeTableKeywords(fitsfile* fptr, std::map<std::string,TableKeywordInfo>& tableKeywords)
{
    int status = 0;
    for (const auto& kw : tableKeywords) {
        std::string name;
        std::string value;
        std::string comment;
        std::tie(name,value,comment) = kw.second; // kw.second is a tuple of kw name, value and comment
        if ( fits_update_key(fptr, TSTRING, name.c_str(), const_cast<char *>(value.c_str()), comment.c_str(), &status) )
            printerror( status );
    }
}

void FITSImageRW::setInfo(const casacore::RecordInterface &info)
{
    setInfoValidityCheck(info);
    createTable(info);
}
