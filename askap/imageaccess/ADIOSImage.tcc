#ifndef IMAGES_ADIOSIMAGE_TCC
#define IMAGES_ADIOSIMAGE_TCC

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
#include <casacore/tables/DataMan/Adios2StMan.h>

#include <casacore/casa/iostream.h>
#include <casacore/casa/sstream.h>

template <class T> 
ADIOSImage<T>::ADIOSImage (const casacore::TiledShape& shape, 
			   const casacore::CoordinateSystem& coordinateInfo, 
			   const casacore::String& filename, 
			   casacore::uInt rowNumber)
{
  casacore::SetupNewTable newtab (filename, casacore::TableDesc(), casacore::Table::New);
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
  map_p = casacore::ArrayColumn<T>(tab_p, "map").get(rowNumber);
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
  casacore::IPosition tileShape = shape.tileShape();

  const casacore::uInt ndim = latShape.nelements();
  casacore::Bool newColumn = casacore::False;
  if (!tab_p.tableDesc().isColumn("map")) {
    newColumn = casacore::True;

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

  return array.get(rowNumber);
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
	ok = casacore::False;
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

template<class T> 
void ADIOSImage<T>::attach_logtable()
{
  // Open logtable as readonly if main table is not writable.
  casacore::Table& tab = table();
  setLogMember (casacore::LoggerHolder (name() + "/logtable", tab.isWritable()));
  // Insert the keyword if possible and if it does not exist yet.
  if (tab.isWritable()  &&  ! tab.keywordSet().isDefined ("logtable")) {
    tab.rwKeywordSet().defineTable("logtable", Table(name() + "/logtable"));
  }
}

template<class T> 
void ADIOSImage<T>::setTableType()
{
  casacore::TableInfo& info(table().tableInfo());
  const casacore::String reqdType = info.type(casacore::TableInfo::PAGEDIMAGE);
  if (info.type() != reqdType) {
    info.setType(reqdType);
  }
  const casacore::String reqdSubType = info.subType(casacore::TableInfo::PAGEDIMAGE);
  if (info.subType() != reqdSubType) {
    info.setSubType(reqdSubType);
  }
}

template <class T>
void ADIOSImage<T>::restoreAll (const casacore::TableRecord& rec)
{
  // Restore the coordinates.
  casacore::CoordinateSystem* restoredCoords = casacore::CoordinateSystem::restore(rec, "coords");
  AlwaysAssert(restoredCoords != 0, casacore::AipsError);
  setCoordsMember (*restoredCoords);
  delete restoredCoords;
  // Restore the image info.
  restoreImageInfo (rec);
  // Restore the units.
  restoreUnits (rec);
  // Restore the miscinfo.
  restoreMiscInfo (rec);
}

template<class T>
void ADIOSImage<T>::applyMaskSpecifier (const casacore::MaskSpecifier& spec)
{
  // Use default mask if told to do so.
  // If it does not exist, use no mask.
  casacore::String name = spec.name();
  if (spec.useDefault()) {
    name = getDefaultMask();
    if (! hasRegion (name, casacore::RegionHandler::Masks)) {
      name = casacore::String();
    }
  }
  applyMask (name);
}

template<class T>
void ADIOSImage<T>::restoreImageInfo (const casacore::TableRecord& rec)
{
  if (rec.isDefined("imageinfo")) {
    casacore::String error;
    casacore::ImageInfo info;
    casacore::Bool ok = info.fromRecord (error, rec.asRecord("imageinfo"));
    if (ok) {
      setImageInfoMember (info);
    } else {
      casacore::LogIO os;
      os << casacore::LogIO::WARN << "Failed to restore the ImageInfo in image " << name()
         << "; " << error << casacore::LogIO::POST;
    }
  }
}

template<class T> 
void ADIOSImage<T>::restoreUnits (const casacore::TableRecord& rec)
{
  casacore::Unit retval;
  casacore::String unitName;
  if (rec.isDefined("units")) {
    if (rec.dataType("units") != casacore::TpString) {
      casacore::LogIO os;
      os << casacore::LogOrigin("ADIOSImage<T>", "units()", WHERE)
	 << "'units' keyword in image table is not a string! Units not restored." 
         << casacore::LogIO::SEVERE << casacore::LogIO::POST;
    } else {
      rec.get("units", unitName);
    }
  }
  if (! unitName.empty()) {
    // OK, non-empty unit, see if it's valid, if not try some known things to
    // make a valid unit out of it.
    if (! casacore::UnitVal::check(unitName)) {
      // Beam and Pixel are the most common undefined units
      casacore::UnitMap::putUser("Pixel",casacore::UnitVal(1.0),"Pixel unit");
      casacore::UnitMap::putUser("Beam",casacore::UnitVal(1.0),"Beam area");
    }
    if (! casacore::UnitVal::check(unitName)) {
      // OK, maybe we need FITS
      casacore::UnitMap::addFITS();
    }
    if (!casacore::UnitVal::check(unitName)) {
      // I give up!
      casacore::LogIO os;
      casacore::UnitMap::putUser(unitName, casacore::UnitVal(1.0, casacore::UnitDim::Dnon), unitName);
      os << casacore::LogIO::WARN << "FITS unit \"" << unitName
         << "\" unknown to CASA - will treat it as non-dimensional."
	 << casacore::LogIO::POST;
      retval.setName(unitName);
      retval.setValue(casacore::UnitVal(1.0, casacore::UnitDim::Dnon));
    } else {
      retval = casacore::Unit(unitName);
    }
  }
  setUnitMember (retval);
}

template<class T> 
void ADIOSImage<T>::restoreMiscInfo (const casacore::TableRecord& rec)
{
  if (rec.isDefined("miscinfo")  &&
      rec.dataType("miscinfo") == casacore::TpRecord) {
    setMiscInfoMember (rec.asRecord ("miscinfo"));
  }
}

template<class T>
void ADIOSImage<T>::applyMask (const casacore::String& maskName)
{
  // No region if no mask name is given.
  if (maskName.empty()) {
    delete regionPtr_p;
    regionPtr_p = 0;
    return;
  }
  // Reconstruct the ImageRegion object.
  // Turn the region into lattice coordinates.
  casacore::ImageRegion* regPtr = getImageRegionPtr (maskName, casacore::RegionHandler::Masks);
  casacore::LatticeRegion* latReg = new casacore::LatticeRegion
                          (regPtr->toLatticeRegion (coordinates(), shape()));
  delete regPtr;
  // The mask has to cover the entire image.
  if (latReg->shape() != shape()) {
    delete latReg;
    throw (casacore::AipsError ("PagedImage::setDefaultMask - region " + maskName +
		      " does not cover the full image"));
  }
  // Replace current by new mask.
  delete regionPtr_p;
  regionPtr_p = latReg;
}
#endif