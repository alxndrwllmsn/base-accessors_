/// @file
/// @brief a container class for the adapter to most methods of IConstDataAccessor
///
/// @details There is a use case for buffering the data from the accessors. 
/// This is a container for multiple MemBufferDataAccessor.
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


#ifndef ASKAP_ACCESSORS_MEM_BUFFER_DATA_ACCESSOR_STACK_H
#define ASKAP_ACCESSORS_MEM_BUFFER_DATA_ACCESSOR_STACK_H

// own includes
#include <askap/dataaccess/MemBufferDataAccessor.h>
#include <askap/dataaccess/SharedIter.h>
#include <askap/dataaccess/IDataIterator.h>

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
class MemBufferDataAccessorStackable : virtual public MemBufferDataAccessor
{
public:
  enum class OrderByOptions {
     /// Default order - Cubes ordered by TIME - Internal Cube ordering default VIS ordering
     /// buffers if available
     DEFAULT = 0,
     /// REVERSE - just reverse the order of the Cubes in TIME. Generally used for testing
     REVERSE = 1,
     /// Order is by increasing W. (UVW rotated to tangent point) The dimensionality of the cubes is
     /// maintained. But the order is strictly increasing W
     W_ORDER = 2
     
  };
  
  /// @brief construct an object linked with the given const accessor
  /// @param[in] acc a reference to the associated accessor
  explicit MemBufferDataAccessorStackable(const IConstDataAccessor &acc);
  
  /// @brief construct an object linked with the given Const iterator
  /// @param[in] iter the associated iterator
  /// @details These constructors iterate through the data themselves and stack the
  /// the accessors. Some elements of the accessors are stored by reference so need to
  /// be instantiated with new copies.
  
  explicit MemBufferDataAccessorStackable(const IConstDataSharedIter iter);
  
  /// @brief construct an object linked with the given iterator
   /// @param[in] iter the associated iterator
   /// @details Should be identical to the const version.
  ///  These constructors iterate through the data themselves and stack the
   /// the accessors. Some elements of the accessors are stored by reference so need to
   /// be instantiated with new copies.
  
  explicit MemBufferDataAccessorStackable(const IDataSharedIter iter);
  
  /// @brief copy contructor
  /// @details only real issue is to run through the stack and copy the contents.
  MemBufferDataAccessorStackable(const MemBufferDataAccessorStackable &other);
  
  /// @brief copy/assignment operator
  
  MemBufferDataAccessorStackable & operator=(const MemBufferDataAccessorStackable &other);
  
  /// @brief append operator
  /// @details Simply adds the accessor to an internal stack. At the same time it should copy any elements that may be lost
  /// due to reference storage into local copies.
  void append(MemBufferDataAccessor other);
  
  /// @brief get an accessor from the stack
  /// @param[in] index the index into the stack.
  
  const MemBufferDataAccessor& getConstAccessor(int index) const;
  /// @brief get an accessor from the stack
  const MemBufferDataAccessor& getConstAccessor() const;
  /// @brief get an accessor from the stack
  /// @param[in] index the index into the stack.

  MemBufferDataAccessor& getAccessor(int index);
  /// @brief get an accessor from the stack
  MemBufferDataAccessor& getAccessor();
  
  /// @brief howmany accessors do we have
  
  size_t numAcc() const {
    return itsAccessorStack.size();
  }
  
  void setAccessorIndex(int index) {
    if (index < 0 || index > numAcc()) {
      ASKAPTHROW(AskapError,"Requested index out of range");
    }
    itsAccessorIndex = index;
  }
  int getAccessorIndex() {
    return itsAccessorIndex;
  }
  /// @brief This is to provide a common interface to other adapters.
  const casacore::Cube<casacore::Complex>& visibility() const;
  
  /// THis is to provide a common interface to other adapters
  casacore::Cube<casacore::Complex>& rwVisibility();
  
  /// This is essentially overriding the MetaDataAccessor method of the same name
  
  casacore::Vector<casacore::RigidVector<casacore::Double, 3> > uvw();
 
  
  void orderBy( OrderByOptions );

private:
 
  int itsAccessorIndex;
  
  mutable casacore::Cube<casacore::Complex> itsBuffer;
  
  std::vector<MemBufferDataAccessor> itsAccessorStack;
  
  std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> > itsUVWStack;
  
  
  
};

} // namespace accessors

} // namespace askap


#endif // #ifndef MEM_BUFFER_DATA_ACCESSOR_STACK_H
