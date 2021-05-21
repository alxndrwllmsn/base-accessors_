/// @file
/// @brief a container class for the adapter to most methods of IConstDataAccessor
///
/// @details There is a use case for buffering the data from the accessors. 
/// This is a container for multiple MemBufferDataAccessor.
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


#ifndef ASKAP_ACCESSORS_MEM_BUFFER_DATA_ACCESSOR_STACK_H
#define ASKAP_ACCESSORS_MEM_BUFFER_DATA_ACCESSOR_STACK_H

// own includes
#include <askap/dataaccess/MemBufferDataAccessor.h>

#ifdef _OPENMP
//boost include
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#endif

namespace askap {
	
namespace accessors {

/// @brief an adapter to most methods of IConstDataAccessor
///
/// @details 
class MemBufferDataAccessorStackAble : virtual public MemBufferDataAccessor
{
public:
  
  /// construct an object linked with the given const accessor
  /// @param[in] acc a reference to the associated accessor
  explicit MemBufferDataAccessorStackAble(const IConstDataAccessor &acc);
   
  /// copy contructor
  MemBufferDataAccessorStackAble(const MemBufferDataAccessorStackAble &other);
  
  /// copy/assignment operator
  
  MemBufferDataAccessorStackAble & operator=(const MemBufferDataAccessorStackAble &other);
  
  /// append operator
  MemBufferDataAccessorStackAble * append(MemBufferDataAccessorStackAble &other);
  
  const casacore::Cube<casacore::Complex>& visibility() const;
  
  casacore::Cube<casacore::Complex>& rwVisibility();
  

private:
  
  #ifdef _OPENMP
  /// @brief synchronisation lock for resizing of the buffer
  mutable boost::mutex itsMutex;
  
  #endif
  void resizeBufferIfNeeded();
  
  mutable casacore::Cube<casacore::Complex> itsBuffer;
  
};

} // namespace accessors

} // namespace askap


#endif // #ifndef MEM_BUFFER_DATA_ACCESSOR_STACK_H
