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

template <class T> class ADIOSImage : public casacore::PagedImage<T>
{
public:
  ADIOSImage (const casacore::TiledShape& mapShape,
	      const casacore::CoordinateSystem& coordinateInfo,
	      const casacore::String& nameOfNewFile,
	      casacore::uInt rowNumber = 0);

  explicit ADIOSImage(const casacore::String &filename, 
                      casacore::MaskSpecifier spec = casacore::MaskSpecifier(),
                      casacore::uInt rowNumber = 0);

  casacore::Table& table()
    { return tab_p; }

  virtual casacore::String name () const;

  casacore::Array<T> makeArrayColumn (const casacore::TiledShape& shape, casacore::uInt rowNumber);

  virtual casacore::Bool setUnits (const casacore::Unit& newUnits);

  virtual casacore::Bool setImageInfo(const casacore::ImageInfo& info);

  virtual casacore::Bool setMiscInfo (const casacore::RecordInterface& newInfo);

  //rewritten PagedImage private functions
  void attach_logtable();
  void setTableType();
  void restoreAll (const casacore::TableRecord& rec);
  void applyMaskSpecifier (const casacore::MaskSpecifier&);
  void restoreImageInfo (const casacore::TableRecord& rec);
  void restoreUnits (const casacore::TableRecord& rec);
  void restoreMiscInfo (const casacore::TableRecord& rec);
  void applyMask (const casacore::String& maskName);





 // etc...
private:
  casacore::Array<T> map_p;
  casacore::Table tab_p;

public:
  using casacore::PagedImage<T>::setCoordinateInfo;
  using casacore::PagedImage<T>::shape;

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







};

}
}

#include <askap/imageaccess/ADIOSImage.tcc>

#endif