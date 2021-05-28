//
//  StackedDataSource.cpp
//  accessors
//
//  Created by Stephen Ord on 27/5/21.
//

#include <askap/dataaccess/StackedDataSource.h>
#include <askap/dataaccess/StackedDataIterator.h>
#include <askap/dataaccess/IDataSource.h>


using namespace askap;
using namespace askap::accessors;

StackedDataSource::StackedDataSource(MemBufferDataAccessorStackable &theStack) : itsStack(theStack)
{}

boost::shared_ptr<IDataIterator> StackedDataSource::createIterator() const
{
  return boost::shared_ptr<IDataIterator>(new StackedDataIterator(itsStack));
}
boost::shared_ptr<IConstDataIterator> StackedDataSource::createConstIterator() const
{
  return boost::shared_ptr<IConstDataIterator>(new StackedDataIterator(itsStack));
}
boost::shared_ptr<IConstDataIterator> StackedDataSource::createConstIterator(const
      IDataSelectorConstPtr &sel, const IDataConverterConstPtr &conv) const
{
  return boost::shared_ptr<IConstDataIterator>(new StackedDataIterator(itsStack));
}

boost::shared_ptr<IDataIterator> StackedDataSource::createIterator(const IDataSelectorConstPtr &sel, const IDataConverterConstPtr &conv) const
{
  return boost::shared_ptr<IDataIterator>(new StackedDataIterator(itsStack));
}


IDataConverterPtr StackedDataSource::createConverter() const
{
  ASKAPTHROW(AskapError,"StackedDataSource::createConverter() not yet implemented");
}
IDataSelectorPtr StackedDataSource::createSelector() const
{
  ASKAPTHROW(AskapError,"StackedDataSource::createSelector() not yet implemented");
}
