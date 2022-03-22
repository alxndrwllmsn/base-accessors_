/// @file FitsImageAccessParallel.cc
/// @brief Access FITS image using parallel I/O
/// @details This class implements the IImageAccess interface for FITS images.
/// This class adds parallel I/O operations in cases where that is possible.
/// At the moment it can deal with 3d images and 4d images with a degenerate
/// 3rd or 4th axis.
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
/// Foundation, Inc.,  59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Mark Wieringa <mark.wieringa@csiro.au>
///

#include <askap_accessors.h>

#include <askap/askap/AskapLogging.h>
#include <casacore/casa/OS/CanonicalConversion.h>

#include <askap/imageaccess/FitsImageAccessParallel.h>

#include <fitsio.h>
#include <askap/imageaccess/FITSImageRW.h> // for printerror#
ASKAP_LOGGER(logger, ".fitsImageAccessParallel");

using namespace askap;
using namespace askap::accessors;

/// @brief constructor
/// @param[in] comms, MPI communicator
/// @param[in] axis, image axis to distribute over (i.e., for cube: 0,1,2 gives yz,xz,xy planes)
FitsImageAccessParallel::FitsImageAccessParallel(askapparallel::AskapParallel &comms, uint axis):
    itsComms(comms), itsAxis(axis), itsParallel(-1)
{
    ASKAPLOG_INFO_STR(logger, "Creating parallel FITS accessor with data distributed over axis " << axis);
}

// reading methods

/// @brief read full image distributed by rank
/// @param[in] name image name
/// @return array with pixels
casacore::Array<float> FitsImageAccessParallel::read(const std::string &name) const
{
    if (canDoParallelIO(name)) return read_all(name, itsAxis);
    else return FitsImageAccess::read(name);

}

/// @brief read part of the image
/// @param[in] name image name
/// @param[in] blc bottom left corner of the selection
/// @param[in] trc top right corner of the selection
/// @return array with pixels for the selection only
/// @details The read operation will only be parallel when reading one entire
/// plane along axes perpendicular to the distribution axis. The ranks should
/// specify consecutive planes in rank order for this to work correctly.
casacore::Array<float> FitsImageAccessParallel::read(const std::string &name, const casacore::IPosition &blc,
                                                     const casacore::IPosition &trc) const
{
    bool parallel = canDoParallelIO(name);
    int section = blctrcTosection(blc,trc);

    if (section>=0 && parallel) {
        int nsection = itsShape(itsAxis) / itsComms.nProcs();
        return read_all(name, itsAxis, nsection, section);
    } else {
        return FitsImageAccess::read(name, blc, trc);
    }
}

/// @brief read part of the image - collective MPI read
/// @param[in] name image name
/// @param[in] iax, axis to distribute over: use 1 for 2D images,  0, 1 or 2 for x, y, z, i.e., yz planes, xz planes, xy planes
/// @return array with pixels for the section of the image read
casacore::Array<float> FitsImageAccessParallel::read_all(const std::string &name,
    int iax, int nsub, int sub) const
{
    std::string fullname = name;
    if (name.rfind(".fits") == std::string::npos) {
        fullname = name + ".fits";
    }
    ASKAPASSERT(iax>=0);
    ASKAPLOG_INFO_STR(logger, "Reading the FITS image " << name << " distributed over axis " << iax);

    MPI_Datatype filetype;
    MPI_Offset offset;
    casa::IPosition bufshape;
    setFileAccess(fullname, bufshape, offset, filetype, iax, nsub, sub);
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, fullname.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, offset, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);
    const MPI_Offset bufsize = bufshape.product();  // local number to read
    casa::Float* buf = new casa::Float[bufsize];
    // Collective read of the whole cube
    MPI_Status status;
    MPI_File_read_all(fh, buf, bufsize, MPI_FLOAT, &status);
    MPI_File_close(&fh);
    // Take care of endianness
    casa::Array<casa::Float> buffer(bufshape);
    casa::CanonicalConversion::toLocal(buffer.data(), buf, bufsize);
    delete [] buf;
    if (nsub > 1) ASKAPLOG_INFO_STR(logger, " - returning section " << sub << ", an array with shape " << buffer.shape());
    return buffer;
}

/// @brief write full image - across ranks
/// @param[in] name image name
/// @return array with pixels
void FitsImageAccessParallel::write(const std::string &name, const casacore::Array<float> &arr)
{
    if (canDoParallelIO(name)) write_all(name, arr, itsAxis);
    else FitsImageAccess::write(name, arr);
}

/// @brief write a slice of an image
/// @param[in] name image name
/// @param[in] arr array with pixels
/// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
/// @details The write operation will only be parallel when writing one entire
/// plane along axes perpendicular to the distribution axis. The ranks should
/// specify consecutive planes in rank order for this to work correctly.
void FitsImageAccessParallel::write(const std::string &name, const casacore::Array<float> &arr,
                   const casacore::IPosition &where)
{
    bool parallel = canDoParallelIO(name);
    casacore::IPosition tlc = where;
    // Deal with 2d input array, but will only work correctly if axis>1
    for (uint i=0; i<arr.shape().nelements(); i++) tlc(i) += arr.shape()(i) - 1;
    int section = blctrcTosection(where, tlc);

    if (section>=0 && parallel) {
        int nsection = itsShape(itsAxis) / itsComms.nProcs();
        write_all(name, arr, itsAxis, nsection, section);
    } else {
        FitsImageAccess::write(name, arr, where);
    }
}

/// @brief write a slice of an image and mask
/// @param[in] name image name
/// @param[in] arr array with pixels
/// @param[in] mask array with mask
/// @param[in] where bottom left corner where to put the slice to (trc is deduced from the array shape)
/// @details The write operation will only be parallel when writing one entire
/// plane along axes perpendicular to the distribution axis. The ranks should
/// specify consecutive planes in rank order for this to work correctly.
void FitsImageAccessParallel::write(const std::string &name, const casacore::Array<float> &arr,
                   const casacore::Array<bool> &mask, const casacore::IPosition &where)
{
   bool parallel = canDoParallelIO(name);
   casacore::IPosition tlc = where;
   // Deal with 2d input array, but will only work correctly if axis>1
   for (uint i=0; i<arr.shape().nelements(); i++) tlc(i) += arr.shape()(i) - 1;
   int section = blctrcTosection(where, tlc);

   if (section>=0 && parallel) {
       int nsection = itsShape(itsAxis) / itsComms.nProcs();
       casacore::Array<float> arrmasked;
       arrmasked = arr;
       for(size_t i=0;i<arr.size();i++){
           if(!mask.data()[i]){
               casacore::setNaN(arrmasked.data()[i]);
           }
       }
       write_all(name, arrmasked, itsAxis, nsection, section);
   } else {
       FitsImageAccess::write(name, arr, mask, where);
   }
}

/// @brief write an image - collective MPI write.
/// @details Note that the fits header must be written to disk before calling this.
/// @param[in] name image name
/// @param[in] arr array with pixels. Array dimension iax * #ranks must match the corresponding image dimension
/// @param[in] iax, axis to distribute over: 0, 1 or 2 for x, y, z, i.e., yz planes, xz planes, xy planes
void FitsImageAccessParallel::write_all(const std::string &name,
    const casacore::Array<float> &arr, int iax, int nsub, int sub) const
{
    ASKAPLOG_INFO_STR(logger, "Writing array with the shape " << arr.shape() << " into a FITS image " <<
                      name << " distributed over axis " << iax);
    std::string fullname = name;
    if (name.rfind(".fits") == std::string::npos) {
      fullname = name + ".fits";
    }
    ASKAPASSERT(iax>=0);

    MPI_Datatype filetype;
    MPI_Offset offset;
    casa::IPosition bufshape;
    setFileAccess(fullname, bufshape, offset, filetype, iax, nsub, sub);

    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, fullname.c_str(), MPI_MODE_APPEND|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, offset, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);
    const MPI_Offset bufsize = bufshape.product();
    casa::Float* buf = new casa::Float[bufsize];
    if (arr.contiguousStorage()) {
        casa::CanonicalConversion::fromLocal(buf, arr.data(), bufsize);
    } else {
        casa::Bool deleteIt;
        const casa::Float *storage = arr.getStorage(deleteIt);
        casa::CanonicalConversion::fromLocal(buf, storage, bufsize);
        arr.freeStorage(storage, deleteIt);
    }
    MPI_Status status;
    MPI_File_write_all(fh, buf, bufsize, MPI_FLOAT, &status);
    MPI_File_close(&fh);
    delete [] buf;

    // Add fits padding to make file size multiple of 2880
    if (itsComms.isMaster() && sub == nsub-1) {
        fits_padding(fullname);
    }

    // All wait for padding to be written
    itsComms.barrier();

}

bool FitsImageAccessParallel::canDoParallelIO(const std::string &name) const
{
    if (name != itsName) {
        // we keep some values cached so cast away const
        FitsImageAccessParallel *This = (FitsImageAccessParallel *) this;
        This->itsName = name;
        casa::IPosition imageShape;
        casa::Long headersize;
        std::string fullname = name;
        if (name.rfind(".fits") == std::string::npos) {
            fullname = name + ".fits";
        }
        decode_header(fullname, imageShape, headersize);
        int ndim = imageShape.size();
        This->itsShape.resize(ndim);
        This->itsShape = imageShape;
        ASKAPCHECK(itsAxis < ndim, "imageaccess.axis needs to be less than number of image axes");
        This->itsParallel = ( itsShape(itsAxis) % itsComms.nProcs() == 0  );
        if (ndim>3) This->itsParallel &= itsShape(2)==1 || itsShape(3)==1;
    }
    return itsParallel;
}

/// @brief turn blc/trc into a section of the cube to read
/// @param[in] blc bottom left corner of the selection
/// @param[in] trc top right corner of the selection
/// @return section number, or -1 if input invalid
int FitsImageAccessParallel::blctrcTosection(const casacore::IPosition & blc, const casacore::IPosition & trc) const
{
    // check we're reading a full plane
    bool ok = true;
    for (uint i=0; i<blc.size(); i++ ) {
        if (i != itsAxis) {
            ok = ok && blc(i) == 0 && trc(i) == (itsShape(i)-1);
        } else {
            ok = ok && blc(i) == trc(i);
        }
    }
    if (ok) return blc(itsAxis) / itsComms.nProcs();
    return -1;
}


void FitsImageAccessParallel::copy_header(const casa::String &infile, const casa::String& outfile) const
{
    using namespace std;
    // get header size
    casa::IPosition shape;
    casa::Long headersize;
    std::string fullinfile = infile;
    if (fullinfile.rfind(".fits") == std::string::npos) {
        fullinfile += ".fits";
    }
    std::string fulloutfile = outfile;
    if (fulloutfile.rfind(".fits") == std::string::npos) {
        fulloutfile += ".fits";
    }
    ASKAPLOG_INFO_STR(logger,"copy_header: "<<fullinfile<<", "<<fulloutfile);
    decode_header(fullinfile, shape, headersize);
    // create the new output file and copy header
    //streampos size;
    char * header;
    ifstream file (fullinfile, ios::in|ios::binary);
    if (file.is_open())
    {
        header = new char [headersize];
        file.read (header, headersize);
        file.close();
        ofstream ofile (fulloutfile, ios::out|ios::binary|ios::trunc);
        if (ofile.is_open()) {
            ofile.write(header,headersize);
            ofile.close();
        }
        delete[] header;
    }
}

void FitsImageAccessParallel::copy_header_with_historykw(const casa::String &infile, 
                                                         const casa::String& outfile,
                                                         const std::vector<std::string>& historyLines) const
{
    using namespace std;

    ASKAPCHECK(!historyLines.empty(), "FitsImageAccessParallel::copy_header_historykw historyLines argument is empty");

    const unsigned long KEYWORD_SIZE = 80; // number of bytes per keyword
    const unsigned long KEYWORD_NAME_SIZE = 8; // number of bytes per keyword name
    char* fitsHistoryLinesBuffer = nullptr;
    std::size_t fitsHistoryLinesBufferSize = -1;
    // put the keywords in the vector (this is what the user input) to a
    // buffer in a format that fits expected.
    if ( ! historyLines.empty() ) {
        // allocate the buffer for the HISTORY keywords in the historyLines vector where each keyword
        // is 80 bytes
        fitsHistoryLinesBuffer = new char[historyLines.size() * KEYWORD_SIZE * sizeof(char)];
        fitsHistoryLinesBufferSize = historyLines.size() * KEYWORD_SIZE * sizeof(char);
        std::memset(fitsHistoryLinesBuffer,0,historyLines.size() * KEYWORD_SIZE * sizeof(char));
        // Copy the history keyword to the buffer so that it conforms to FITS keyword header format.
        unsigned long offset = 0;
        for (const auto& line : historyLines) {
            std::memcpy(fitsHistoryLinesBuffer+offset,"HISTORY ",KEYWORD_NAME_SIZE);
            offset = offset + 8;
            std::memcpy(fitsHistoryLinesBuffer+offset,line.c_str(),line.size()); 
            offset = offset + (KEYWORD_SIZE - KEYWORD_NAME_SIZE);
        }
    }
    //std::vector<std::array<unsigned char,80>> FitsHistoryLines;
    // get header size
    casa::IPosition shape;
    casa::Long headersize;
    std::string fullinfile = infile;
    if (fullinfile.rfind(".fits") == std::string::npos) {
        fullinfile += ".fits";
    }
    std::string fulloutfile = outfile;
    if (fulloutfile.rfind(".fits") == std::string::npos) {
        fulloutfile += ".fits";
    }

    ASKAPLOG_INFO_STR(logger,"copy_header_with_historykw: "<<fullinfile<<", "<<fulloutfile);

    decode_header(fullinfile, shape, headersize);
    ifstream file (fullinfile, ios::in|ios::binary);
    if (file.is_open())
    {
        // create the new output file and copy header
        char * header;
        header = new char [headersize];
        file.read (header, headersize);
        file.close();

        // kwPtr points to the end of the keyword section
        char* kwPtr = header + headersize - 1;

        // We only want to copy to the last keyword before the END keyword
        long spaceAfterEndKW = 0; // how many spaces after the END keyword
        while ( *kwPtr == ' ' ) {
            spaceAfterEndKW = spaceAfterEndKW + 1;
            kwPtr = kwPtr - 1;
        }

        // Found the END keyword. kwPtr points to 'D'
        // 'END' keyword is 80 bytes so we have to add 3 to
        // spaceAfterEndKW to get to the end of the last keyword before the
        // END keyword
        spaceAfterEndKW = spaceAfterEndKW + 3;

        ofstream ofile (fulloutfile, ios::out|ios::binary|ios::trunc);
        if (ofile.is_open()) {
            char endKW[KEYWORD_SIZE];
            std::memset(endKW,' ',KEYWORD_SIZE);
            std::memcpy(endKW,"END",3);

            // copy the keywords of the input file to the output file
            // minus the END keyword
            ofile.write(header,headersize - spaceAfterEndKW);
            // copy the HISTORY keyword to the output file
            ofile.write(fitsHistoryLinesBuffer,fitsHistoryLinesBufferSize);
            // write END keyword
            ofile.write(endKW,KEYWORD_SIZE);
            ofile.close();
            // now the padding to make it a multiple of 2880 bytes.
            //fits_padding(outfile);
            fits_padding(fulloutfile);
        }
        delete[] fitsHistoryLinesBuffer;
        delete[] header;
    }
}

void FitsImageAccessParallel::setFileAccess(const casa::String& name,
    casa::IPosition& bufshape, MPI_Offset& offset, MPI_Datatype&  filetype, int iax,
    int nsub, int sub) const
{
    std::string fullname = name;
    if (fullname.rfind(".fits") == std::string::npos) {
        fullname += ".fits";
    }
    ASKAPLOG_INFO_STR(logger,"setFileAccess: name - " << name
                        << ", fullname - " << fullname);
    // get header and data size, get image dimensions
    casa::IPosition imageShape;
    casa::Long headersize;
    decode_header(fullname, imageShape, headersize);
    int nz = imageShape(2);
    if (imageShape.size()>3) nz *= imageShape(3);
    // Now work out the file access pattern and start offset
    MPI_Status status;
    const int myrank = itsComms.rank();
    const int numprocs = itsComms.nProcs();
    int nplane = imageShape(iax) / numprocs / nsub;
    ASKAPASSERT(imageShape(iax) == nsub * nplane * numprocs);
    bufshape = imageShape;
    bufshape(iax) = nplane;
    MPI_Offset blocksize = nplane;
    for (int i=1; i<=iax; i++) blocksize *= bufshape(i-1);   // number of contiguous floats

    // # blocks, blocklength, stride (distance between blocks)
    if (iax == 0) {
        MPI_Type_vector(imageShape(2)*nz, blocksize, imageShape(0), MPI_FLOAT, &filetype);
    } else if (iax == 1) {
        MPI_Type_vector(nz, blocksize, imageShape(0)*imageShape(1), MPI_FLOAT, &filetype);
    } else {
        MPI_Type_vector(1, blocksize, blocksize, MPI_FLOAT, &filetype);
    }
    MPI_Type_commit(&filetype);
    offset = headersize + (myrank + sub * numprocs) * blocksize * sizeof(float);
}


void FitsImageAccessParallel::decode_header(const casa::String& infile, casa::IPosition& imageShape,
                    casa::Long& headersize) const
{
    fitsfile *infptr;  // FITS file pointers
    int status = 0;  // CFITSIO status value MUST be initialized to zero!

    std::string fullinfile = infile;
    if (fullinfile.rfind(".fits") == std::string::npos) {
        fullinfile += ".fits";
    }

    fits_open_file(&infptr, fullinfile.c_str(), READONLY, &status); // open input image
    if (status) {
        fits_report_error(stderr, status); // print error message
        return;
    }
    LONGLONG headstart, datastart, dataend;
    fits_get_hduaddrll (infptr, &headstart, &datastart, &dataend, &status);
    ASKAPLOG_INFO_STR(logger,"header starts at: "<<headstart<<" data start: "<<datastart<<" end: "<<dataend);
    headersize = datastart;
    int naxis;
    long naxes[4];
    fits_get_img_dim(infptr, &naxis, &status);  // read dimensions
    //ASKAPLOG_INFO_STR(logger,"Input image #dimensions: " << naxis);
    fits_get_img_size(infptr, 4, naxes, &status);
    imageShape.resize(0);
    ASKAPCHECK("naxis==3||naxis==4","FITS image must have 3 or 4 axes");
    if (naxis==3) imageShape = casacore::IPosition(naxis,naxes[0],naxes[1],naxes[2]);
    if (naxis==4) imageShape = casacore::IPosition(naxis,naxes[0],naxes[1],naxes[2],naxes[3]);
    fits_close_file(infptr, &status);
}

void FitsImageAccessParallel::fits_padding(const casa::String& filename) const
{
    using namespace std;

    std::string fullinfile = filename;
    if (fullinfile.rfind(".fits") == std::string::npos) {
        fullinfile += ".fits";
    }

    ASKAPLOG_INFO_STR(logger,"fits_padding: filename - " << filename
                        << ", fullinfile - " << fullinfile);
    std::ifstream infile(fullinfile, ios::binary | ios::ate);
    const size_t file_size = infile.tellg();
    const size_t padding = 2880 - (file_size % 2880);
    infile.close();
    if (padding != 2880) {
        ofstream ofile;
        ofile.open(fullinfile, ios::binary | ios::app);
        char * buf = new char[padding];
        for (char* bufp = &buf[padding-1]; bufp >= buf; bufp--) *bufp = 0;
        ofile.write(buf,padding);
        ofile.close();
        delete [] buf;
    }
    ASKAPLOG_INFO_STR(logger,"master added "<< padding % 2880 << " bytes of FITS padding to file of size "<<file_size);
}
