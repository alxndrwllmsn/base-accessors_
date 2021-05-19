/// @file
/// @brief an adapter to a stack of MemDataBuffers
///
/// @details It has proven neccessary that a buffer of input data
/// is required - but as we need to access data in dimensions other than TIME
/// it is perhaps simpler to buffer more than one of these simple cubes.
/// Typically, the need for such class arises if one needs a buffering
/// of more than one iteration and the content of buffers <IS> required
/// to be preserved when the corresponding iterator advances. We are therefore stacking individual cubes.
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
#include <askap/dataaccess/MemBufferDataAccessorStack.h>

using namespace askap;
using namespace askap::accessors;


/// construct an object linked with the given const accessor
/// @param[in] acc a reference to the associated accessor
MemBufferDataAccessorStack::MemBufferDataAccessorStack(const IConstDataAccessor &acc) :
      MetaDataAccessor(acc) {}
  
/// Read-only visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
const casacore::Cube<casacore::Complex>& MemBufferDataAccessorStack::visibility() const
{
  resizeBufferIfNeeded();
  return itsBuffer;
}
	
/// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
/// each element is a complex visibility)
///
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casacore::Cube<casacore::Complex>& MemBufferDataAccessorStack::rwVisibility()
{
  resizeBufferIfNeeded();
  return itsBuffer;
}

/// @brief a helper method to ensure the buffer has appropriate shape
void MemBufferDataAccessorStack::resizeBufferIfNeeded() const
{
  #ifdef _OPENMP
  boost::lock_guard<boost::mutex> lock(itsMutex);
  #endif
  
  const IConstDataAccessor &acc = getROAccessor();
  if (itsBuffer.nrow() != acc.nRow() || itsBuffer.ncolumn() != acc.nChannel() ||
                                        itsBuffer.nplane() != acc.nPol()) {
      itsBuffer.resize(acc.nRow(), acc.nChannel(), acc.nPol());
  }
}


