/// @file  StackedDataIterator.h
/// @brief The class describing the iterator through the source that holds a
///  MemBufferDataAccessorStackable
///
/// @details In order to seamlessly integrate the stack of visibilites we need a data source
///  and iterator. It is possible that the arbitary ordering possible using this class will mean that
///  only a restricted use case actually works. But even a reordering of visibilities within a time step
///  may prove worthwhile.
///
///
/// @copyright (c) 2021 CSIRO
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
/// @author Stephen Ord <stephen.ord@csiro.au>


#ifndef ASKAP_ACCESSORS_STACKED_DATA_ITERATOR_H
#define ASKAP_ACCESSORS_STACKED_DATA_ITERATOR_H

#include <stdio.h>

#include <askap/dataaccess/IDataIterator.h>
#include <askap/dataaccess/MemBufferDataAccessorStackable.h>

namespace askap {

namespace accessors {

    class StackedDataIterator : virtual public IDataIterator
    {
    public:
      // @brief default constructor
      StackedDataIterator(MemBufferDataAccessorStackable &stack);
          
     
    /// @brief operator* delivers a reference to data accessor (current chunk)
    /// @details
    /// @return a reference to the current chunk
    /// @note
    /// constness of the return type is changed to allow read/write
    /// operations.
    ///
    virtual IDataAccessor& operator*() const;

    /// @brief Switch the output of operator* and operator-> to one of 
    /// the buffers.
    /// @details This is meant to be done to provide the same 
    /// interface for a buffer access as exists for the original 
    /// visibilities (e.g. it->visibility() to get the cube).
    /// It can be used for an easy substitution of the original 
    /// visibilities to ones stored in a buffer, when the iterator is
    /// passed as a parameter to mathematical algorithms.   
    /// The operator* and operator-> will refer to the chosen buffer
    /// until a new buffer is selected or the chooseOriginal() method
    /// is executed to revert operators to their default meaning
    /// (to refer to the primary visibility data).  
    /// @param[in] bufferID  the name of the buffer to choose
    ///
    virtual void chooseBuffer(const std::string &bufferID);

    /// Switch the output of operator* and operator-> to the original
    /// state (present after the iterator is just constructed) 
    /// where they point to the primary visibility data. This method
    /// is indended to cancel the results of chooseBuffer(std::string)
    ///

    virtual void chooseOriginal();
  
    /// @brief obtain any associated buffer for read/write access.
    /// @details The buffer is identified by its bufferID. The method 
    /// ignores a chooseBuffer/chooseOriginal setting.  
    /// @param[in] bufferID the name of the buffer requested
    /// @return a reference to writable data accessor to the
    ///         buffer requested
    virtual IDataAccessor& buffer(const std::string &bufferID) const;    

    /// Restart the iteration from the beginning
    void init();
	
    /// advance the iterator one step further 
    /// @return True if there are more data (so constructions like 
    ///         while(it.next()) {} are possible)
    virtual casacore::Bool next();


    virtual casacore::Bool hasMore() const noexcept ;


    /// populate the cube with the data stored in the given buffer  
    /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
    ///            cube to fill with the complex visibility data
    /// @param[in] name a name of the buffer to work with
    virtual void readBuffer(casacore::Cube<casacore::Complex> &vis,
                          const std::string &name) const;
  

    /// write the cube back to the given buffer  
    /// @param[in] vis a reference to the nRow x nChannel x nPol buffer
    ///            cube to fill with the complex visibility data
    /// @param[in] name a name of the buffer to work with
    virtual void writeBuffer(const casacore::Cube<casacore::Complex> &vis,
                           const std::string &name) const;
    private:
      
      MemBufferDataAccessorStackable &itsStack;
    
    };
  }
}
#endif /* ASKAP_ACCESSORS_STACKED_DATA_ITERATOR_H */


