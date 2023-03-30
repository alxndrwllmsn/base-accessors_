/// @file Utils.h
/// @brief Utilities class
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

#ifndef ASKAP_ACCESSORS_UTILS_H
#define ASKAP_ACCESSORS_UTILS_H
namespace askap {
namespace accessors {
    /// @brief this structure wraps the c pointers required by cfitsio library to ensure
    ///        memory used is properly freed.
    struct CPointerWrapper
    {
        explicit CPointerWrapper(unsigned int numColumns)
        : itsNumColumns(numColumns),
          itsTType  { new char* [sizeof(char*) * numColumns] },
          itsTForm { new char* [sizeof(char*) * numColumns] },
          itsUnits { new char* [sizeof(char*) * numColumns] }
        {
            for ( unsigned int i = 0; i < itsNumColumns; i++ ) {
                itsTType[i] = nullptr;
                itsTForm[i] = nullptr;
                itsUnits[i] = nullptr;
            }
        }

        ~CPointerWrapper()
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

        unsigned int itsNumColumns;
        char** itsTType;
        char** itsTForm;
        char** itsUnits;
    };

} // accessors
} // askap
#endif
