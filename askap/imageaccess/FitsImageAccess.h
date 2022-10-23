/// @file FitsImageAccess.h
/// @brief Access FITS image
/// @details This class implements IImageAccess interface for FITS image
///
///
/// @copyright (c) 2016 CSIRO
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
/// @author Stephen Ord <stephen.ord@csiro.au
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ASKAP_ACCESSORS_FITS_IMAGE_ACCESS_H
#define ASKAP_ACCESSORS_FITS_IMAGE_ACCESS_H

#include <boost/shared_ptr.hpp>

#include <askap/imageaccess/IImageAccess.h>
#include <askap/imageaccess/FITSImageRW.h>

namespace askap {
namespace accessors {

/// @brief Access casa image
/// @details This class implements IImageAccess interface for FITS images
/// Internally we are dealing with casa images. We only need the FITS format
/// for efficient output.
/// It therefore makes sense to heavily inherit from the CASA conversion
/// classes.
/// @ingroup imageaccess

struct FitsImageAccess : public IImageAccess<> {

    public:

        /// @brief connect accessor to an existing image
        /// @details Instantiates the private FITSImageRW shared pointer.
        /// @param[in] name image name
        void connect(const std::string &name) const;

        /// @brief use fast allocation of file
        /// @details calculates file size and writes a zero at the end
        /// @param[in] fast If true use fast allocation, default uses CFITSIO
        inline void useFastAlloc(bool fast = false) { itsFastAlloc = fast;}

        //////////////////
        // Reading methods
        //////////////////

        /// @brief obtain the shape
        /// @param[in] name image name
        /// @return full shape of the given image
        virtual casacore::IPosition shape(const std::string &name) const override;

        /// @brief read full image
        /// @param[in] name image name
        /// @return array with pixels
        virtual casacore::Array<float> read(const std::string &name) const override;

        /// @brief read part of the image
        /// @param[in] name image name
        /// @param[in] blc bottom left corner of the selection
        /// @param[in] trc top right corner of the selection
        /// @return array with pixels for the selection only
        virtual casacore::Array<float> read(const std::string &name, const casacore::IPosition &blc,
                                        const casacore::IPosition &trc) const override;
    
    /// @brief Determine whether an image has a mask
    /// @param[in] nam image name
    /// @return True if image has a mask, False if not.
    virtual bool isMasked(const std::string &name) const override;

    /// @brief read the mask for the full image
    /// @param[in] name image name
    /// @return bool array with mask values - 1=good, 0=bad
    virtual casacore::LogicalArray readMask(const std::string &name) const override;

    /// @brief read the mask for part of the image
    /// @param[in] name image name
    /// @param[in] blc bottom left corner of the selection
    /// @param[in] trc top right corner of the selection
    /// @return bool array with mask values - 1=good, 0=bad
    virtual casacore::LogicalArray readMask(const std::string &name, const casacore::IPosition &blc,
                                    const casacore::IPosition &trc) const override;

        /// @brief obtain coordinate system info
        /// @param[in] name image name
        /// @return coordinate system object
        virtual casacore::CoordinateSystem coordSys(const std::string &name) const override;

        /// @brief obtain coordinate system info for part of an image
        /// @param[in] name image name
        /// @return coordinate system object
        virtual casacore::CoordinateSystem coordSysSlice(const std::string &name, const casacore::IPosition &blc,
                const casacore::IPosition &trc) const override;

        /// @brief obtain beam info
        /// @param[in] name image name
        /// @return beam info vector
        virtual casacore::Vector<casacore::Quantum<double> > beamInfo(const std::string &name) const override;

        /// @brief obtain beam info
        /// @param[in] name image name
        /// @return beam info list
        virtual BeamList beamList(const std::string &name) const override;

        /// @brief obtain pixel units
        /// @param[in] name image name
        /// @return units string
        virtual std::string getUnits(const std::string &name) const override;

        /// @brief this method gets the data in the FITS binary table and puts in the casacore::Record.
        /// @param[in] name - name of the FITS file
        /// param[in] info - casacore::Record to contain the data of the FITS table. The info record has
        ///                  sub record(s) which stores the FITS table columns' data and the table keywords
        ///                  are held in the info's (Record)  names, values, and comments.
        virtual void getInfo(const std::string &name, const std::string& tableName, casacore::Record &info) override;

        /// @brief Get a particular keyword from the image metadata (A.K.A header)
        /// @details This reads a given keyword to the image metadata.
        /// @param[in] name Image name
        /// @param[in] keyword The name of the metadata keyword
        /// @return pair of strings - keyword value and comment
        virtual std::pair<std::string, std::string> getMetadataKeyword(const std::string &name, const std::string &keyword) const override;

        //////////////////
        // Writing methods
        //////////////////

        /// @brief create a new image
        /// @details A call to this method should preceed any write calls. The actual
        /// image may be created only upon the first write call. Details depend on the
        /// implementation.
        /// @param[in] name image name
        /// @param[in] shape full shape of the image
        /// @param[in] csys coordinate system of the full image
        virtual void create(const std::string &name, const casacore::IPosition &shape,
                            const casacore::CoordinateSystem &csys) override;

        /// @brief write full image
        /// @param[in] name image name
        /// @param[in] arr array with pixels
        virtual void write(const std::string &name, const casacore::Array<float> &arr) override;

        /// @brief write a slice of an image
        /// @param[in] name image name
        /// @param[in] arr array with pixels
        /// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
        virtual void write(const std::string &name, const casacore::Array<float> &arr,
                           const casacore::IPosition &where) override;

        /// @brief write full image and mask
        /// @param[in] name image name
        /// @param[in] mask array with mask
        /// @param[in] arr array with pixels
        virtual void write(const std::string &name, const casacore::Array<float> &arr,
                           const casacore::Array<bool> &mask);

        /// @brief write a slice of an image and mask
        /// @param[in] name image name
        /// @param[in] arr array with pixels
        /// @param[in] mask array with mask
        /// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
        virtual void write(const std::string &name, const casacore::Array<float> &arr,
                           const casacore::Array<bool> &mask, const casacore::IPosition &where) override;

        /// @brief write a slice of an image mask
        /// @param[in] name image name
        /// @param[in] arr array with pixels
        /// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
        virtual void writeMask(const std::string &name, const casacore::Array<bool> &mask,
                               const casacore::IPosition &where) override;

        /// @brief write a slice of an image mask
        /// @param[in] name image name
        /// @param[in] arr array with pixels

        virtual void writeMask(const std::string &name, const casacore::Array<bool> &mask) override;
        /// @brief set brightness units of the image
        /// @details
        /// @param[in] name image name
        /// @param[in] units string describing brightness units of the image (e.g. "Jy/beam")
        virtual void setUnits(const std::string &name, const std::string &units) override;

        /// @brief set restoring beam info
        /// @details For the restored image we want to carry size and orientation of the restoring beam
        /// with the image. This method allows to assign this info.
        /// @param[in] name image name
        /// @param[in] maj major axis in radians
        /// @param[in] min minor axis in radians
        /// @param[in] pa position angle in radians
        virtual void setBeamInfo(const std::string &name, double maj, double min, double pa) override;

        /// @brief set restoring beam info for all channels
        /// @details For the restored image we want to carry size and orientation of the restoring beam
        /// with the image. This method allows to assign this info.
        /// @param[in] name image name
        /// @param[in] beamlist The list of beams
        virtual void setBeamInfo(const std::string &name, const BeamList & beamlist) override;

        /// @brief apply mask to image
        /// @details Deteails depend upon the implemenation - CASA images will have the pixel mask assigned
        /// but FITS images will have it applied to the pixels ... which is an irreversible process
        /// @param[in] name image name
        virtual void makeDefaultMask(const std::string &name) override;

        /// @brief Set a particular keyword for the metadata (A.K.A header)
        /// @details This adds a given keyword to the image metadata.
        /// @param[in] name Image name
        /// @param[in] keyword The name of the metadata keyword
        /// @param[in] value The value for the keyword, in string format
        /// @param[in] desc A description of the keyword
        virtual void setMetadataKeyword(const std::string &name, const std::string &keyword,
                                        const std::string value, const std::string &desc = "") override;

        /// @brief Set the keywords for the metadata (A.K.A header)
        /// @details This adds keywords to the image metadata.
        /// @param[in] name Image name
        /// @param[in] keywords A parset with keyword entries (KEYWORD = ["keyword value","keyword description","STRING"])
        virtual void setMetadataKeywords(const std::string &name, const LOFAR::ParameterSet &keywords) override;

        /// @brief Add HISTORY messages to the image metadata
        /// @details Adds a list of strings detailing the history of the image
        /// @param[in] name Image name
        /// @param[in] historyLines History comments to add
        virtual void addHistory(const std::string &name, const std::vector<std::string> &historyLines) override;

        /// @brief Write what is in the info object to FITS binary table.
        /// @param[in] name - name of the FITS file
        /// @param[in] info - In this case the info object is an instance of casacore::Record class.
        /// It is required that the info object confirms to the following requirements :
        /// (1) It is a top level record. The fields in the top level record can only have
        ///     the following types : String (TpString), Float (TpFloat) , Double (TpDouble),
        ///     Integer (TpInt) and Record (TpRecord). Fields that are not of type Record (TpRecord)
        ///     are treated as FITS keywords in the binary table.
        /// (2) It (info object) has one and ony one sub record (i.e it has a field of type TpRecord).
        ///     This record contains information which is inserted into a FITS binary table data.
        ///     The format of this record is as follows:
        ///        Each field in the record represents a column in the FITS binary table.
        ///     (ii)  The data types of the fields are : array of string (TpArrayString), array of float
        ///           (TpArrayFloat), array of double (TpArrayDouble), array of integer (TpArrayInt)
        ///     (iii) Each element in the array in (ii) denotes a value of a cell in the FITS binary column.
        ///           e.g: a field whose name is "col1" and has an array of 5 integer numbers is written to
        ///           FITS binary table as a table that has a column name "col1" which contains 5 rows where
        ///           the first cell of the first row has the value of array[0] and so on.
        ///     (iv)  The name of the fields becomes the columns' name in the FITS binary table.
        ///     (v)   Lastly, there is a field in this record that has a name called "Unit" of type TpArrayString.
        ///           It is used to specified the units of the table columns. e.g: the first element of this array
        ///           indicates the unit of the first column in the table and so on.
        virtual void setInfo(const std::string &name, const casacore::RecordInterface &info) override;

    private:
        mutable boost::shared_ptr<FITSImageRW> itsFITSImage;
        bool itsFastAlloc;

};


} // namespace accessors
} // namespace askap

#endif
