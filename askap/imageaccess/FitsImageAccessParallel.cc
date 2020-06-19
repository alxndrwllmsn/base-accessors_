/// @file FitsImageAccess.cc
/// @brief Access FITS image
/// @details This class implements IImageAccess interface for FITS image
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
/// Foundation, Inc.,  59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Mark Wieringa <mark.wieringa@csiro.au>
///

#include <askap_accessors.h>


#include <askap/askap/AskapLogging.h>
#include <casacore/casa/OS/CanonicalConversion.h>

#include <askap/imageaccess/FitsImageAccessParallel.h>

#include <fitsio.h>

ASKAP_LOGGER(logger, ".fitsImageAccessParallel");

using namespace askap;
using namespace askap::accessors;

// reading methods

/// @brief read part of the image - collective MPI read
/// @param[in] comms, MPI communicator
/// @param[in] name image name
/// @param[in] iax, axis to distribute over: use 1 for 2D images,  0, 1 or 2 for x, y, z, i.e., yz planes, xz planes, xy planes
/// @return array with pixels for the section of the image read
casacore::Array<float> FitsImageAccessParallel::read_all(askapparallel::AskapParallel& comms, const std::string &name,
    int iax) const
{
    std::string fullname = name;
    if (name.rfind(".fits") == std::string::npos) {
        fullname = name + ".fits";
    }
    ASKAPASSERT(iax>=0 && iax <=2);
    ASKAPLOG_INFO_STR(logger, "Reading the FITS image " << name << " distributed over axis " << iax);

    MPI_Datatype filetype;
    MPI_Offset offset;
    casa::IPosition bufshape;
    setFileAccess(comms, fullname, bufshape, offset, filetype, iax);
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, fullname.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, offset, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);
    const MPI::Offset bufsize = bufshape.product();  // local number to read
    casa::Float* buf = new casa::Float[bufsize];
    // Collective read of the whole cube
    MPI_Status status;
    MPI_File_read_all(fh, buf, bufsize, MPI_FLOAT, &status);
    MPI_File_close(&fh);
    // Take care of endianness
    casa::Array<casa::Float> buffer(bufshape);
    casa::CanonicalConversion::toLocal(buffer.data(), buf, bufsize);
    delete [] buf;
    return buffer;
}

/// @brief write an image - collective MPI write.
/// @details Note that the fits header must be written to disk before calling this.
/// @param[in] comms, MPI communicator
/// @param[in] name image name
/// @param[in] arr array with pixels. Array dimension iax * #ranks must match the corresponding image dimension
/// @param[in] iax, axis to distribute over: 0, 1 or 2 for x, y, z, i.e., yz planes, xz planes, xy planes
void FitsImageAccessParallel::write_all(askap::askapparallel::AskapParallel& comms, const std::string &name,
    const casacore::Array<float> &arr, int iax) const
{
    ASKAPLOG_INFO_STR(logger, "Writing array with the shape " << arr.shape() << " into a FITS image " <<
                      name << " distributed over axis " << iax);
    std::string fullname = name;
    if (name.rfind(".fits") == std::string::npos) {
      fullname = name + ".fits";
    }
    ASKAPASSERT(iax>=0 && iax<=2);

    MPI_Datatype filetype;
    MPI_Offset offset;
    casa::IPosition bufshape;
    setFileAccess(comms, fullname, bufshape, offset, filetype, iax);

    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, fullname.c_str(), MPI_MODE_APPEND|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, offset, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);
    const MPI::Offset bufsize = bufshape.product();
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
    if (comms.isMaster()) {
        fits_padding(fullname);
    }

    // All wait for padding to be written
    comms.barrier();

}


void FitsImageAccessParallel::copy_header(const casa::String &infile, const casa::String& outfile) const
{
    using namespace std;
    // get header size
    casa::Int nx,ny,nz;
    casa::Long headersize;
    decode_header(infile, nx, ny, nz, headersize);
    // create the new output file and copy header
    //streampos size;
    char * header;
    ifstream file (infile, ios::in|ios::binary);
    if (file.is_open())
    {
        header = new char [headersize];
        file.read (header, headersize);
        file.close();
        ofstream ofile (outfile, ios::out|ios::binary|ios::trunc);
        if (ofile.is_open()) {
            ofile.write(header,headersize);
            ofile.close();
        }
        delete[] header;
    }
}

void FitsImageAccessParallel::setFileAccess(askapparallel::AskapParallel& comms, const casa::String& name,
    casa::IPosition& bufshape, MPI_Offset& offset, MPI_Datatype&  filetype, casacore::Int iax) const
{
    // get header and data size, get image dimensions
    casa::Int nx,ny,nz;
    casa::Long headersize;
    decode_header(name, nx, ny, nz, headersize);
    casacore::IPosition cubeshape(3,nx,ny,nz);
    if (nz == 1) ASKAPASSERT(iax < 2);

    // Now work out the file access pattern and start offset
    MPI_Status status;
    const int myrank = comms.rank();
    const int numprocs = comms.nProcs();
    int nplane = cubeshape(iax) / numprocs;
    ASKAPASSERT(cubeshape(iax) == nplane * numprocs);
    bufshape = cubeshape;
    bufshape(iax) = nplane;
    MPI::Offset blocksize = nplane;
    for (int i=1; i<=iax; i++) blocksize *= bufshape(i-1);   // number of contiguous floats

    // # blocks, blocklength, stride (distance between blocks)
    if (iax == 0) {
        MPI_Type_vector(ny*nz, blocksize, nx, MPI_FLOAT, &filetype);
    } else if (iax == 1) {
        MPI_Type_vector(nz, blocksize, nx*ny, MPI_FLOAT, &filetype);
    } else {
        MPI_Type_vector(1, blocksize, blocksize, MPI_FLOAT, &filetype);
    }
    MPI_Type_commit(&filetype);
    offset = headersize + myrank * blocksize * sizeof(float);
}


void FitsImageAccessParallel::decode_header(const casa::String& infile, casa::Int& nx, casa::Int& ny,
                    casa::Int& nz, casa::Long& headersize) const
{
    fitsfile *infptr, *outfptr;  // FITS file pointers
    int status = 0;  // CFITSIO status value MUST be initialized to zero!

    fits_open_file(&infptr, infile.c_str(), READONLY, &status); // open input image
    if (status) {
        fits_report_error(stderr, status); // print error message
        return;
    }
    LONGLONG headstart, datastart, dataend;
    fits_get_hduaddrll (infptr, &headstart, &datastart, &dataend, &status);
    //ASKAPLOG_INFO_STR(logger,"header starts at: "<<headstart<<" data start: "<<datastart<<" end: "<<dataend);
    headersize = datastart;
    int naxis;
    long naxes[4];
    fits_get_img_dim(infptr, &naxis, &status);  // read dimensions
    //ASKAPLOG_INFO_STR(logger,"Input image #dimensions: " << naxis);
    fits_get_img_size(infptr, 4, naxes, &status);
    nx = naxes[0];
    ny = naxes[1];
    nz = naxes[2];
    if (naxis >3) {
        if (nz==1) nz = naxes[3];
    }
    fits_close_file(infptr, &status);
}

void FitsImageAccessParallel::fits_padding(const casa::String& filename) const
{
    using namespace std;
    std::ifstream infile(filename, ios::binary | ios::ate);
    const size_t file_size = infile.tellg();
    const size_t padding = 2880 - (file_size % 2880);
    infile.close();
    if (padding != 2880) {
        ofstream ofile;
        ofile.open(filename, ios::binary | ios::app);
        char * buf = new char[padding];
        for (char* bufp = &buf[padding-1]; bufp >= buf; bufp--) *bufp = 0;
        ofile.write(buf,padding);
        ofile.close();
        delete [] buf;
    }
    ASKAPLOG_INFO_STR(logger,"master added "<< padding % 2880 << " bytes of FITS padding to file of size "<<file_size);
}
