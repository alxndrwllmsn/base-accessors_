#ifndef IMAGES_ADIOSIMAGE_H
#define IMAGES_ADIOSIMAGE_H

//# Includes
#include <casacore/casa/aips.h>
#include <casacore/images/Images/PagedImage.h>
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/images/Images/ImageAttrHandlerCasa.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/casa/Utilities/DataType.h>
#include <casacore/tables/Tables/TableRecord.h>

//# Forward Declarations
#include <casacore/casa/iosfwd.h>

namespace askap {

namespace accessors {

template <class T> class ADIOSImage : public casacore::ImageInterface<T>
{
public:
  ADIOSImage (const casacore::TiledShape& mapShape,
	      const casacore::CoordinateSystem& coordinateInfo,
	      const casacore::String& nameOfNewFile,
	      casacore::uInt rowNumber = 0);

  explicit ADIOSImage(const casacore::String &filename, 
                      casacore::MaskSpecifier spec = casacore::MaskSpecifier(),
                      casacore::uInt rowNumber = 0);

  ADIOSImage (const ADIOSImage<T>& other);


  casacore::Table& table()
    { return tab_p; }

  static casacore::String className()
    { return "PagedImage"; }

  virtual casacore::String imageType() const;

  casacore::Array<T> makeArrayColumn (const casacore::TiledShape& shape, casacore::uInt rowNumber);

  virtual casacore::Bool setUnits (const casacore::Unit& newUnits);

  virtual casacore::Bool setImageInfo(const casacore::ImageInfo& info);

  virtual casacore::Bool setMiscInfo (const casacore::RecordInterface& newInfo);

  virtual casacore::Bool setCoordinateInfo (const casacore::CoordinateSystem& coords);

  virtual casacore::IPosition shape() const;

  virtual casacore::String name (casacore::Bool stripPath=casacore::False) const;

  virtual casacore::Bool ok() const;

  virtual casacore::Bool doGetSlice (casacore::Array<T>& buffer, const casacore::Slicer& theSlice);

  virtual void doPutSlice (const casacore::Array<T>& sourceBuffer, const casacore::IPosition& where, const casacore::IPosition& stride);

  virtual const casacore::LatticeRegion* getRegionPtr() const;

  virtual casacore::ImageInterface<T>* cloneII() const;

  virtual void resize (const casacore::TiledShape& newShape);

  //rewritten PagedImage private functions
  void attach_logtable();
  void setTableType();
  void restoreAll (const casacore::TableRecord& rec);
  void applyMaskSpecifier (const casacore::MaskSpecifier&);
  void restoreImageInfo (const casacore::TableRecord& rec);
  void restoreUnits (const casacore::TableRecord& rec);
  void restoreMiscInfo (const casacore::TableRecord& rec);
  void applyMask (const casacore::String& maskName);
  static casacore::Table& getTable (void* imagePtr, casacore::Bool writable);

private:
  casacore::Array<T> map_p;
  casacore::Table tab_p;
  casacore::LatticeRegion* regionPtr_p;
  casacore::uInt row_p;


public:
  using casacore::ImageInterface<T>::setMiscInfoMember;
  using casacore::ImageInterface<T>::setUnitMember;
  using casacore::ImageInterface<T>::imageInfo;
  using casacore::ImageInterface<T>::setLogMember;
  using casacore::ImageInterface<T>::setCoordsMember;
  using casacore::ImageInterface<T>::setImageInfoMember;
  using casacore::ImageInterface<T>::getDefaultMask;
  using casacore::ImageInterface<T>::hasRegion;
  using casacore::ImageInterface<T>::getImageRegionPtr;
  using casacore::ImageInterface<T>::coordinates;

  using casacore::Lattice<T>::ndim;







};

}
}

#include <askap/imageaccess/ADIOSImage.tcc>

#endif