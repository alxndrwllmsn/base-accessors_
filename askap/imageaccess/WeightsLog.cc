/// @file WeightsLog.h
///
/// Class to log the Weights of individual channels of a spectral cube
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
/// @author Mark Wieringa <Mark.Wieringa@csiro.au>
///

#include <askap/imageaccess/WeightsLog.h>

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/askap/AskapLogging.h>
#include <askap/askapparallel/AskapParallel.h>
#include <casacore/casa/Arrays/Vector.h>
#include <Common/ParameterSet.h>
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

// Local package includes
#include <askap/imageaccess/CasaImageAccess.h>
ASKAP_LOGGER(logger, ".WeightsLog");

namespace askap {
namespace accessors {

WeightsLog::WeightsLog(const LOFAR::ParameterSet &parset) :
    itsFilename(parset.getString("WeightsLog", ""))
{
}

WeightsLog::WeightsLog(const std::string &filename) :
    itsFilename(filename)
{
}

float WeightsLog::weight(const unsigned int channel) const
{
   std::map<unsigned int, float>::const_iterator it = itsWeightsList.find(channel);
   if (it != itsWeightsList.end()) {
       return it->second;
   } 
   ASKAPLOG_WARN_STR(logger, "WeightsList has no Weights recorded for channel " << channel << ", returning zero Weights");
   return 0.f;
}

void WeightsLog::write() const
{
    if (itsFilename != "") {

        if (valid()) {

          std::ofstream fout(itsFilename.c_str());
          fout << "#Channel Weight\n";

          for (std::map<unsigned int, float>::const_iterator weightsIt = itsWeightsList.begin(); weightsIt!= itsWeightsList.end(); ++weightsIt) {
              fout << weightsIt->first << " "
                   << weightsIt->second<< std::endl;
          }
        } else {
          ASKAPLOG_WARN_STR(logger,
                            "WeightsLog cannot write the log, as the weights are invalid");
        }

    } else {
        ASKAPLOG_WARN_STR(logger,
                          "WeightsLog cannot write the log, as no filename has been specified");
    }
}

casacore::Record WeightsLog::toRecord() const
{
    casacore::Record record;

    if (valid()) {

      const int n = itsWeightsList.size();
      // We want columns for channel and weight
      casacore::Vector<casacore::Int> colChan(n);
      casacore::Vector<casacore::Float> colWt(n);
      //std::map<unsigned int, float>::iterator wl = itsWeightsList.begin();
      int i = 0;
      for (auto wl = itsWeightsList.begin(); wl != itsWeightsList.end(); wl++, i++) {
          colChan[i] = wl->first;
          colWt[i] = wl->second;
      }
      casacore::Record subRecord;

      subRecord.define("CHAN",colChan);
      subRecord.define("WEIGHT",colWt);
      casacore::Vector<casacore::String> units(2);
      units(0) = "";
      units(1) = "";
      subRecord.define("Units",units);
      record.defineRecord("WEIGHTS",subRecord);
      record.define("NCHAN",n);
      record.setComment("NCHAN","Number of channels");
    } else {
      ASKAPLOG_WARN_STR(logger,
                        "WeightsLog cannot write the log, as the weights are invalid");
    }
    return record;
}



void WeightsLog::read()
{
    itsWeightsList.clear();

    if (itsFilename != "") {

        std::ifstream fin(itsFilename.c_str());
        ASKAPCHECK(fin.is_open(),"Weights log file " << itsFilename << " could not be opened.");

        unsigned int chan;
        float wt;
        std::string line, name;

        while (getline(fin, line),
                !fin.eof()) {
            if (line[0] != '#') {
                std::stringstream ss(line);
                ss >> chan >> wt;
                itsWeightsList[chan] = wt;
            }
        }
    }
}

void WeightsLog::gather(askapparallel::AskapParallel &comms, int rankToGather, bool includeMaster)
{

    ASKAPLOG_DEBUG_STR(logger, "Gathering the Weights info - on rank " << comms.rank() << " and gathering onto rank " << rankToGather);

    if (comms.isParallel()) {

        const int minrank = includeMaster ? 0 : 1;

        if (comms.rank() != rankToGather) {
            // If we are here, the current rank does not do the gathering.
            // Instead, send the data to the rank that is.

            ASKAPLOG_DEBUG_STR(logger, "Sending from rank " << comms.rank() <<" to rank " << rankToGather);
            // send to desired rank
            LOFAR::BlobString bs;
            bs.resize(0);
            LOFAR::BlobOBufString bob(bs);
            LOFAR::BlobOStream out(bob);
            out.putStart("gatherWeights", 1);
            const unsigned int size = itsWeightsList.size();
            out << size;
            if (itsWeightsList.size() > 0) {
                ASKAPLOG_DEBUG_STR(logger, "This has data, so sending Weights list of size " << size);
                for (std::map<unsigned int, float>::const_iterator weightsIt = itsWeightsList.begin(); weightsIt != itsWeightsList.end(); ++weightsIt) {
                    out << weightsIt->first
                        << weightsIt->second;
                }
            }
            out.putEnd();
            comms.sendBlob(bs, rankToGather);
        } else {

            // The rank on which we are gathering the data
            // Loop over all the others and read their Weights.
            for (int rank = minrank; rank < comms.nProcs(); rank++) {

                if (rank != comms.rank()) {
                    ASKAPLOG_DEBUG_STR(logger, "Preparing to receive Weightslist from rank " << rank);
                    LOFAR::BlobString bs;
                    bs.resize(0);
                    comms.receiveBlob(bs, rank);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    const int version = in.getStart("gatherWeights");
                    ASKAPASSERT(version == 1);
                    unsigned int size;
                    in >> size;
                    if (size > 0) {
                        ASKAPLOG_DEBUG_STR(logger, "Has data - about to receive " << size << " channels");
                        for(unsigned int i=0;i<size;i++){
                            unsigned int chan;
                            float wt;
                            in >> chan >> wt;
                            if (wt > 0.) {
                                itsWeightsList[chan] = wt;
                            }
                        }
                    }
                    else {
                        ASKAPLOG_DEBUG_STR(logger, "No data from rank " << rank);
                    }
                    in.getEnd();
                }

            }

        }

    }

}

bool WeightsLog::valid() const {
    bool valid = true;
    for (const auto& wl : itsWeightsList) {
        if (wl.second < 0) {
            valid = false;
            break;
        }
    }
    return valid;
}


}
}
