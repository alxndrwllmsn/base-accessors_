/// @file
/// @brief an adapter to a stack of MemDataBuffers
///
/// @details It has proven neccessary that a buffer of input data
/// is required - but as we need to access data in dimensions other than TIME
/// it is perhaps simpler to buffer more than one of these simple cubes.
/// Typically, the need for such class arises if one needs a buffering
/// of more than one iteration and the content of buffers <IS> required
/// to be preserved when the corresponding iterator advances. We are therefore stacking individual cubes.
//
///
/// @copyright (c) 2007 2021 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au> for the original classes
/// @author Stephen Ord <stephen.ord@csiro.au> for the stack.
///

// own includes
#include <askap/dataaccess/MemBufferDataAccessorStackable.h>
#include <askap/dataaccess/MemBufferDataAccessor.h>


#include <askap/askap/AskapLogging.h>
#include <askap/askap/AskapError.h>

#include <casacore/casa/Arrays/Vector.h>

using namespace askap;
using namespace askap::accessors;

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IConstDataSharedIter iter) : MemBufferDataAccessor(*iter), MetaDataAccessor(*iter), itsAccessorIndex(0)
{
  // When instantiated from an iterator - we can do a lot in the constructor
  for (iter.init();iter.hasMore();iter.next())
  {
    // iterating over each time step
    // buffer-accessor, used as a replacement for proper buffers held in the subtable
    // effectively, an array with the same shape as the visibility cube is held by this class
    accessors::MemBufferDataAccessor accBuffer(*iter);
    // putting the input visibilities into a cube
    // Normally this array is filled with a model ... but in this context we need the visibilities.
    // Perhaps we should store them separately.
    // @note this does not seem to be required ....
    accBuffer.rwVisibility() = iter->visibility().copy();
    append(accBuffer);
  }
}

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IDataSharedIter iter) : MemBufferDataAccessor(*iter), MetaDataAccessor(*iter), itsAccessorIndex(0)
{
  // When instantiated from an iterator - we can do a lot in the constructor
  for (iter.init();iter.hasMore();iter.next())
  {
    // iterating over each time step
    // buffer-accessor, used as a replacement for proper buffers held in the subtable
    // effectively, an array with the same shape as the visibility cube is held by this class
    accessors::MemBufferDataAccessor accBuffer(*iter);
    // putting the input visibilities into a cube
    accBuffer.rwVisibility() = iter->visibility().copy();
    append(accBuffer);
  }
}

/// @brief construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
/// @details This constructor does nothing. It is expected that accessors would be appended manually.
/// This allows appending in any order.

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IConstDataAccessor &acc) :
MemBufferDataAccessor(acc), MetaDataAccessor(acc), itsAccessorIndex(0) {}


MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const MemBufferDataAccessorStackable &other) : MemBufferDataAccessor(other), MetaDataAccessor(other), itsAccessorIndex(0) {
  for (int i=0; i < other.numAcc(); i++) {
    MemBufferDataAccessor accBuffer(other.getConstAccessor(i));
    append(accBuffer);
  }
}
const MemBufferDataAccessor& MemBufferDataAccessorStackable::getConstAccessor() const
{
  return getConstAccessor(itsAccessorIndex);
}
const MemBufferDataAccessor& MemBufferDataAccessorStackable::getConstAccessor(int index) const {
  return dynamic_cast < const MemBufferDataAccessor & > (itsAccessorStack[index]);
}
MemBufferDataAccessor& MemBufferDataAccessorStackable::getAccessor(int index) {
  return dynamic_cast < MemBufferDataAccessor & > (itsAccessorStack[index]);
}
MemBufferDataAccessor& MemBufferDataAccessorStackable::getAccessor() 
{
  return getAccessor(itsAccessorIndex);
}
/// Read-only visibilities (a cube is nStacks x nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casacore::Cube<casacore::Complex>& MemBufferDataAccessorStackable::visibility() const
{
  return getConstAccessor(itsAccessorIndex).visibility();
}

  /// @brief Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
casacore::Cube<casacore::Complex>& MemBufferDataAccessorStackable::rwVisibility()
{
  return getAccessor(itsAccessorIndex).rwVisibility();
}

/// @brief Append MemBufferAccessor to the stack
/// @details As we are using underlying vector storage the time steps
/// should be added to the back of the vector.
void MemBufferDataAccessorStackable::append( MemBufferDataAccessor acc) {
  itsUVWStack.push_back(acc.uvw().copy());
  itsAccessorStack.push_back(acc);
}

MemBufferDataAccessorStackable & MemBufferDataAccessorStackable::operator=(const MemBufferDataAccessorStackable &other) {
  ASKAPTHROW(AskapError,"MemBufferDataAccessor::operator= not yet implemented");
}
/// @brief Access the UVW array for this Index.
/// @return The actual vector ... should this be a reference ...

casacore::Vector<casacore::RigidVector<casacore::Double, 3> > MemBufferDataAccessorStackable::uvw()

{
  return itsUVWStack[itsAccessorIndex];
}
void MemBufferDataAccessorStackable::orderBy( OrderByOptions opt = OrderByOptions::DEFAULT) {

  if (opt == OrderByOptions::REVERSE ) {
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> > newUVWStack;
    std::vector<MemBufferDataAccessor> newAccessorStack;
  
  // first just test by reversing the order
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> >::reverse_iterator uvw;
    std::vector<MemBufferDataAccessor>::reverse_iterator acc;
  
    for (uvw = itsUVWStack.rbegin(),acc=itsAccessorStack.rbegin() ; uvw < itsUVWStack.rend(); uvw++,acc++) {
      newUVWStack.push_back(*uvw);
      newAccessorStack.push_back(*acc);
    }
    itsAccessorStack.swap(newAccessorStack);
    itsUVWStack.swap(newUVWStack);
  }
}
    
  
  

