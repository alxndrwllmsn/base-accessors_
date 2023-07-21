// #ifndef IMAGES_ADIOSIMAGE
// #define IMAGES_ADIOSIMAGE

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

  // destructor
  ~ADIOSImage();

  casacore::Table& table()
    { return tab_p; }

  virtual casacore::String name () const;

  casacore::Array<T> makeArrayColumn (const casacore::TiledShape& shape, casacore::uInt rowNumber);

  virtual casacore::Bool setUnits (const casacore::Unit& newUnits);

  virtual casacore::Bool setImageInfo(const casacore::ImageInfo& info);

  virtual casacore::Bool setMiscInfo (const casacore::RecordInterface& newInfo);
 // etc...
private:
  casacore::Array<T> map_p;
  casacore::Table tab_p;
};

// #endif