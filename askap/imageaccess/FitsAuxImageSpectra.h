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

//#include <askap/imageaccess/AuxImageSpectra.h>
#include <askap/imageaccess/Utils.h>
#include <casacore/casa/Containers/RecordInterface.h>
#include <fitsio.h>

namespace askap {
namespace accessors {
//class FitsAuxImageSpectra : public AuxImageSpectra {
class FitsAuxImageSpectra {
    public :
        using SpectrumT = std::vector<float>;
        using ArrayOfSpectrumT = std::vector<SpectrumT>;

        FitsAuxImageSpectra(const std::string& fitsFileName,
                            const casacore::RecordInterface &tableInfo,
                            const int nChannels, const int nrows);
        //void add(const std::string& id, const std::vector<float>& spectrum) // override;
        void add(const std::string& id, const SpectrumT& spectrum, 
                 const unsigned int startingRow);
        ~FitsAuxImageSpectra() {}
        static void PrintError(int status);
    private :
        void create(const casacore::RecordInterface &tableInfo,
                    const int nChannels, const int nrows);
        
        constexpr int spectrumHDU () { return 2; }

        fitsfile* itsFitsPtr;
        int itsStatus;
        std::string itsName;
        
};
} // accessors
} // askap
#endif
