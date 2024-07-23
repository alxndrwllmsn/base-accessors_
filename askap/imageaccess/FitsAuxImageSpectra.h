/// @file FitsAuxImageSpectra.h
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

#ifndef ASKAP_ACCESSORS_FITS_AUX_IMAGE_SPECTRA_H
#define ASKAP_ACCESSORS_FITS_AUX_IMAGE_SPECTRA_H

#include <askap/imageaccess/FitsImageAccess.h>
#include <askap/imageaccess/Utils.h>
#include <casacore/casa/Containers/RecordInterface.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/coordinates/Coordinates.h>
#include <casacore/tables/Tables.h>
#include <fitsio.h>
#include <map>
namespace askap {
namespace accessors {
///
/// @brief - This class has a FITS binary used to store various 1D spectra of detected components.
/// @details - The current implementation is each spectra of a detected component is stored as an
///            image in the FITS file, resulting in a large number of FITS files to be generated.
///            Moreover, on Setonix, there is a limit on the number of files in a given directory.
///            As a result, there needs to be a new scheme to store these spectra and this class is
///            designed to overcome such restrictions by keeping these spctra in a FITS binary table.
///            Note: This class does not support parallel IO and hence, in the MPI environment, it is
///            assumed that there is only one rank that does all the IO.
class FitsAuxImageSpectra {
    public :
        using SpectrumT = casacore::Vector<float>;
        using ArrayOfSpectrumT = casacore::Matrix<float>;

        /// @brief create a FITS image spectra table object
        /// @param[in] - fitsFileName - name of the fits file to be created
        /// @param[in] - nChannels - the number of channels.
        ///              This determines the size of the array
        ///              of the spectrum.
        /// @param[in] - nrows - how many rows to be created.
        ///              0 => create an empty table
        // @param[in] - coord - CoordinateSystem object which is used to obtain
        ///             keywords to be written to FITS file header.
        /// @param[in] - a table record. @TODO
        FitsAuxImageSpectra(const std::string& fitsFileName,
                            const int nChannels, const int nrows,
                            const casacore::CoordinateSystem& coord,
                            const casacore::RecordInterface &tableInfo = casacore::Record());

        /// @brief - add an id and a spectrum to the table
        /// @param[in] - id - a string id (must be unique)
        /// @param[in] - spectrum - the spectrum of a component
        void add(const std::string& id, const SpectrumT& spectrum);

        /// @brief - add an array of spectra to the table
        /// @param[in] - ids - an array of identifiers
        /// @param[in] - arrayOfSpectrums - a casacore matrix of spectra
        ///              to be added to the table.
        void add(const std::vector<std::string>& ids, const ArrayOfSpectrumT& arrayOfSpectrums);

        /// @brief - get a spectrum from a table
        /// @param[in] - row - row number in the table
        /// @param[out] - spectrum - the extracted spectrum from the table
        void get(const long row, SpectrumT& spectrum);

        /// @brief - get a spectrum from a table
        /// @param[in] - id - unique id in the id column
        /// @param[out] - spectrum - the extracted spectrum from the table
        void get(const std::string& id, SpectrumT& spectrum);

        /// @brief - destructor
        ~FitsAuxImageSpectra() {}
        static void PrintError(int status);
    private :
        /// @brief - helper method
        void create(const casacore::RecordInterface &tableInfo,
                    const int nChannels, const int nrows);

        /// @brief 0 helper method. Return the spectrum binary table
        ///        of interest.
        int spectrumHDU() const { return 2; }

        /// This map can be HUGE if the table contains millions of rows.
        /// So dont know if it is a good idea to keep it in memory. See
        /// how it goes
        std::map<std::string,long> itsId2RowMap;

        /// store the last inserted row
        long itsCurrentRow;

        fitsfile* itsFitsPtr;
        int itsStatus;

        /// name of the fits file
        std::string itsName;
        // name without .fits extension
        std::string itsBasename;

        /// how many channels in the array ie spectrum size
        int itsNChannels;

        /// image access object. used to create the keywords in the
        /// primary header ie the coordinate system.
        std::unique_ptr<IImageAccess<float>> itsIA;
};
} // accessors
} // askap
#endif
