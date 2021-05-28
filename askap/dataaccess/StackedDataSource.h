/// @file  StackedDataSource.h
/// @brief The class describing the visibility data source that is the instantiated by
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


#ifndef ASKAP_ACCESSORS_STACKED_DATA_SOURCE_H
#define ASKAP_ACCESSORS_STACKED_DATA_SOURCE_H

#include <stdio.h>

#include <askap/dataaccess/IDataSource.h>
#include <askap/dataaccess/MemBufferDataAccessorStackable.h>

namespace askap {

namespace accessors {

    class StackedDataSource : virtual public IDataSource
    {
    public:
      
      StackedDataSource(MemBufferDataAccessorStackable &theStack);
      
      /// get a read/write iterator over the whole dataset represented
      /// by this DataSource object. Default data conversion policies
      /// will be used, see IDataConverter.h for default values.
      /// Default implementation is via the most general
      /// createIterator(...) call, override it in
      /// derived classes, if a (bit) higher performance is required
      ///
      /// @return a shared pointer to DataIterator object
      ///
      /// The method acts as a factory by creating a new DataIterator.
      /// The lifetime of this iterator is the same as the lifetime of
      /// the DataSource object. Therefore, it can be reused multiple times,
      /// if necessary.
      
      virtual boost::shared_ptr<IDataIterator>
                      createIterator() const;
      
      virtual boost::shared_ptr<IConstDataIterator> createConstIterator() const;

      virtual boost::shared_ptr<IConstDataIterator> createConstIterator(const
            IDataSelectorConstPtr &sel, const IDataConverterConstPtr &conv) const;
      
      virtual boost::shared_ptr<IDataIterator> createIterator(const
            IDataSelectorConstPtr &sel, const IDataConverterConstPtr &conv) const;
      
      
      virtual IDataConverterPtr createConverter() const;
      virtual IDataSelectorPtr createSelector() const;
      
    private:
      
      MemBufferDataAccessorStackable &itsStack;
      
  };
}
}
#endif /* ASKAP_ACCESSORS_STACKED_DATA_SOURCE_H */


