#ifndef IMAGES_ADIOSIMAGE
#define IMAGES_ADIOSIMAGE

#include <askap/imageaccess/ADIOSImage.h>
#include <casacore/images/Regions/ImageRegion.h>
#include <casacore/images/Regions/RegionHandlerTable.h>
#include <casacore/images/Images/ImageInfo.h>
#include <casacore/lattices/Lattices/ArrayLattice.h>
#include <casacore/lattices/Lattices/LatticeNavigator.h>
#include <casacore/lattices/Lattices/LatticeStepper.h>
#include <casacore/lattices/Lattices/LatticeIterator.h>
#include <casacore/lattices/Lattices/PagedArrIter.h>
#include <casacore/lattices/LEL/LatticeExprNode.h>
#include <casacore/lattices/LEL/LatticeExpr.h>
#include <casacore/lattices/LRegions/LatticeRegion.h>
#include <casacore/casa/Logging/LogIO.h>
#include <casacore/casa/Logging/LogMessage.h>

#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayLogical.h>
#include <casacore/casa/Arrays/LogiArray.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/Path.h>
#include <casacore/casa/Arrays/IPosition.h>
#include <casacore/casa/Arrays/Slicer.h>
#include <casacore/tables/Tables/SetupNewTab.h>
#include <casacore/tables/Tables/TableLock.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/TableDesc.h>
#include <casacore/tables/Tables/TableRecord.h>
#include <casacore/tables/Tables/TableInfo.h>
#include <casacore/tables/Tables/TableColumn.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/Assert.h>
#include <casacore/casa/Quanta/UnitMap.h>

#include <casacore/casa/iostream.h>
#include <casacore/casa/sstream.h>

template <class T> 
ADIOSImage<T>::ADIOSImage (const casacore::TiledShape& shape, 
			   const casacore::CoordinateSystem& coordinateInfo, 
			   const casacore::String& filename, 
			   casacore::uInt rowNumber)
{
  casacore::SetupNewTable newtab (filename, TableDesc(), Table::New);
  casacore::Table tab(newtab);
  tab_p = tab;
  map_p = makeArrayColumn(shape, rowNumber);
  attach_logtable();
  AlwaysAssert(setCoordinateInfo(coordinateInfo), casacore::AipsError);
  setTableType();
}

template <class T>
ADIOSImage<T>::ADIOSImage (const casacore::String& filename, 
                          casacore::MaskSpecifier spec,
                          casacore::uInt rowNumber)
{
  tab_p = casacore::Table(filename);
  map_p = casacore::ArrayColumn<T>(tab, "map").get(rowNumber);
  attach_logtable();
  restoreAll (tab_p.keywordSet());
  applyMaskSpecifier (spec);
}

template <class T>
casacore::String ADIOSImage<T>::name () const
{
  return tab_p.tableName();
}

template <class T>
casacore::Array<T> ADIOSImage<T>::makeArrayColumn(const casacore::TiledShape& shape, casacore::uInt rowNumber)
{
  casacore::IPosition latShape = shape.shape();
  casacore::IPOsition tileShape = shape.tileShape();

  const casacore::uInt ndim = latShape.nelements();
  casacore::Bool newColumn = False;
  if (!tab_p.tableDesc().isColumn("map")) {
    newColumn = True;

    casacore::TableDesc description;
    description.addColumn(casacore::ArrayColumnDesc<T>("map",
                          casacore::String("version 4.0"),
                          ndim));
    casacore::Adios2StMan stman("",
                                {},
                                {{}},
                                {{{"Variable", "map"},{},{}}});
    tab_p.addColumn(description, stman);
  }
  casacore::ArrayColumn<T> array(tab_p, "map");
  array.attach(tab_p, "map");

  const casacore::IPosition emptyShape(ndim, 1);
  const casacore::uInt rows = tab_p.nrow();
  if (rows <= rowNumber) {
    tab_p.addRow (rowNumber-rows+1);
    for (casacore::uInt r = rows; r < rowNumber; r++) {
      array.setShape(r, emptyShape);
    }
  }
  if (newColumn) {
    for (casacore::uInt r = 0; r < rows; r++) {
      if (r != rowNumber) {
        array.setShape(r, emptyShape);
      }
    }
  }

  array.setShape(rowNumber, latShape);

  return array.get(rowNumber)
}

template <class T>
casacore::Bool ADIOSImage<T>::setUnits(const casacore::Unit& newUnits)
{
  setUnitMember (newUnits);
  casacore::Table& tab = table();
  if (! tab.isWritable()) {
    return false;
  }
  if (tab.keywordSet().isDefined("units")) {
    tab.rwKeywordSet().removeField("units");
  }
  tab.rwKeywordSet().define("units", newUnits.getName());
  return true;
}

template <class T>
casacore::Bool ADIOSImage<T>::setImageInfo(const casacore::ImageInfo& info)
{
  // Set imageinfo in base class.
  casacore::Bool ok = casacore::ImageInterface<T>::setImageInfo(info);
  if (ok) {
    // Make persistent in table keywords.
    casacore::Table& tab = table();
    if (tab.isWritable()) {
      // Delete existing one if there.
      if (tab.keywordSet().isDefined("imageinfo")) {
	tab.rwKeywordSet().removeField("imageinfo");
      }
      // Convert info to a record and save as keyword.
      casacore::TableRecord rec;
      casacore::String error;
      if (imageInfo().toRecord(error, rec)) {
         tab.rwKeywordSet().defineRecord("imageinfo", rec);
      } else {
        // Could not convert to record.
        casacore::LogIO os;
	os << casacore::LogIO::SEVERE << "Error saving ImageInfo in image " << name()
           << "; " << error << casacore::LogIO::POST;
	ok = false;
      }
    } else {
      // Table not writable.
      casacore::LogIO os;
      os << casacore::LogIO::SEVERE
         << "Image " << name() << " is not writable; not saving ImageInfo"
         << casacore::LogIO::POST;
    }
  }
  return ok;
}

template<class T> 
casacore::Bool ADIOSImage<T>::setMiscInfo (const casacore::RecordInterface& newInfo)
{
  setMiscInfoMember (newInfo);
  casacore::Table& tab = table();
  if (! tab.isWritable()) {
    return casacore::False;
  }
  if (tab.keywordSet().isDefined("miscinfo")) {
    tab.rwKeywordSet().removeField("miscinfo");
  }
  tab.rwKeywordSet().defineRecord("miscinfo", newInfo);
  return casacore::True;
}
#endif