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

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IConstDataSharedIter iter) : MemBufferDataAccessor(*iter), MetaDataAccessor(*iter), itsAccessorIndex(0), canReOrder(true)
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
    
    itsVisBuffer.push_back(iter->visibility().copy());
    append(accBuffer);
  }
}

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IDataSharedIter iter) : MemBufferDataAccessor(*iter), MetaDataAccessor(*iter), itsAccessorIndex(0), canReOrder(true)
{
  // When instantiated from an iterator - we can do a lot in the constructor
  for (iter.init();iter.hasMore();iter.next())
  {
    // iterating over each time step
    // buffer-accessor, used as a replacement for proper buffers held in the subtable
    // effectively, an array with the same shape as the visibility cube is held by this class
    accessors::MemBufferDataAccessor accBuffer(*iter);
    // putting the input visibilities into a cube
    
    itsVisBuffer.push_back(iter->visibility().copy());
    append(accBuffer);
  }
}

/// @brief construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
/// @details This constructor does nothing. It is expected that accessors would be appended manually.
/// This allows appending in any order.

MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const IConstDataAccessor &acc) :
MemBufferDataAccessor(acc), MetaDataAccessor(acc), itsAccessorIndex(0), canReOrder(false) {}


MemBufferDataAccessorStackable::MemBufferDataAccessorStackable(const MemBufferDataAccessorStackable &other) : MemBufferDataAccessor(other), MetaDataAccessor(other), itsAccessorIndex(0), canReOrder(false) {
  // Constructing this way makes it difficult to reorder and keep the original visibilities
  
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
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data. In the case where reordering is possible return the cube
/// as stored by the Stack. THis allows semantics like iter->visibility() to return a vis-cube
///  in the same way as for the other iterators.
///
///  If you need access to the buffer for this particular instance of the memBuffer then use
///  the rw interface as this provides access to that buffer. This permits semantics like
///       accessor->rwVisibility() - iter->visibility() to produce non-trivial output. Say in the case
///       where the buffer is filled with model visibilities.
///
const casacore::Cube<casacore::Complex>& MemBufferDataAccessorStackable::visibility() const
{
 
  if (canReOrder) // use the internal buffer as we may have re-ordered
  {
    return itsVisBuffer[itsAccessorIndex];
  }
  else // just use the accessor
  {
    return getConstAccessor(itsAccessorIndex).visibility();
  }
  
}

  /// @brief Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
///
/// @details This method provides direct access to the internal buffer of the viscube for this accessor
/// The const method above is actually interfacing with a backing store for the stack.
///
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
casacore::Cube<casacore::Complex>& MemBufferDataAccessorStackable::rwVisibility()
{  
  return getAccessor(itsAccessorIndex).rwVisibility();
}
/// @brief synchronise the backing vis store held by the stack. With the internal buffer held by the accessor.

void MemBufferDataAccessorStackable::sync( ) {
  getAccessor(itsAccessorIndex).rwVisibility() = itsVisBuffer[itsAccessorIndex];
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

  if (canReOrder == false) {
    ASKAPTHROW(AskapError, "Attempting to reorder a Stack that cannot do that");
  }
  
  if (opt == OrderByOptions::REVERSE ) {
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> > newUVWStack;
    std::vector< casacore::Cube<casacore::Complex> > newVisBuffer;
    std::vector<MemBufferDataAccessor> newAccessorStack;
  
  // first just test by reversing the order
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> >::reverse_iterator uvw;
    std::vector<MemBufferDataAccessor>::reverse_iterator acc;
    std::vector< casacore::Cube<casacore::Complex> >::reverse_iterator vis;
    
    for (uvw = itsUVWStack.rbegin(),acc=itsAccessorStack.rbegin(),vis=itsVisBuffer.rbegin() ; uvw < itsUVWStack.rend(); uvw++,acc++,vis++) {
      newUVWStack.push_back(*uvw);
      newAccessorStack.push_back(*acc);
      newVisBuffer.push_back(*vis);
    }
    itsAccessorStack.swap(newAccessorStack);
    itsUVWStack.swap(newUVWStack);
    itsVisBuffer.swap(newVisBuffer);
  }
  else if (opt==OrderByOptions::W_ORDER) {
    
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> > newUVWStack;
    std::vector<casacore::Vector<casacore::RigidVector<casacore::Double, 3>> >::iterator uvw;
    
    std::vector< casacore::Cube<casacore::Complex> > newVisBuffer;
    std::vector< casacore::Cube<casacore::Complex> >::iterator vis;
    
    std::vector<MemBufferDataAccessor>::iterator acc;
    
    casacore::RigidVector<casacore::Double, 3>  uvw_vector;
    
    
    // we need to sort through the UVW vectors to get. Now the question is
    // are they already rotated the phase centre
    // lets assume they are ...
    
    // logic will be extract the
   
    for (uvw = itsUVWStack.begin(),acc=itsAccessorStack.begin(),vis=itsVisBuffer.begin() ; uvw < itsUVWStack.end(); uvw++,acc++,vis++) {
  
      // ok simple insertion sort goes here ....

      casacore::Vector<casacore::RigidVector<casacore::Double, 3>> currentUVW = *uvw;
      casacore::Vector<casacore::RigidVector<casacore::Double, 3>> sortedUVW;
      
      // could probably do this in place

      sortedUVW = (*uvw).copy();

      MemBufferDataAccessor currentACC = *acc;
     
      // visibility stack
      
      for (int start_index = 0; start_index < currentUVW.size(); start_index++) {
        casacore::RigidVector<casacore::Double, 3> min_uvw = sortedUVW(start_index);

        for (int current_index = start_index; current_index<currentUVW.size(); current_index++) {
          uvw_vector = sortedUVW(current_index);

          if (uvw_vector(2) < min_uvw(2)) {

            sortedUVW(current_index) = min_uvw;
            sortedUVW(start_index) = uvw_vector;
            // Swap the local copy of the vis ....
            for (int channel=0;channel < currentACC.nChannel();channel++) {
              for (int pol=0;pol< currentACC.nPol();pol++) {
                  casacore::Complex current = currentACC.rwVisibility().at(current_index,channel,pol);
                  casacore::Complex start = currentACC.rwVisibility().at(start_index,channel,pol);
                  currentACC.rwVisibility().at(current_index,channel,pol) = start;
                  currentACC.rwVisibility().at(start_index,channel,pol) = current;
                
                  current = (*vis).at(current_index,channel,pol);
                  start = (*vis).at(start_index,channel,pol);
                  (*vis).at(current_index,channel,pol) = start;
                  (*vis).at(start_index,channel,pol) = current;
                
               }
            }
 
            min_uvw =  uvw_vector;

          }
        }
      }
      newUVWStack.push_back(sortedUVW);
    }
    
    itsUVWStack.swap(newUVWStack);
    
  }
}
  
  

