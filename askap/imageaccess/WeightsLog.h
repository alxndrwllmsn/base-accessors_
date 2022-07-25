/// @file WeightsLog.h
///
/// Class to log the imaging weights of individual channels of a spectral cube
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
#ifndef ASKAP_ACCESSORS_WEIGHTS_LOG_H
#define ASKAP_ACCESSORS_WEIGHTS_LOG_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/askapparallel/AskapParallel.h>
#include <casacore/casa/Arrays/Vector.h>
#include <Common/ParameterSet.h>

namespace askap {
namespace accessors {

/// @brief Class to handle writing & reading of channel-level
/// weights information for a spectral cube.
/// @details This class wraps up the functionality required to
/// create and access the weights log files. The class also
/// provides the ability to straightforwardly read the weights
/// log to extract the channel-level weights information.

class WeightsLog {
    public:
        WeightsLog();
        WeightsLog(const LOFAR::ParameterSet &parset);
        WeightsLog(const std::string &filename);
        virtual ~WeightsLog() {};

        /// Set the name of the beam log file
        void setFilename(const std::string& filename) {itsFilename = filename;};

        /// @return the file name of the beam log file
        std::string filename() const {return itsFilename;};

        /// @brief Write the weights information to the weights log
        /// @details The weights information for each channel is written
        /// to the weights log. The log is in ASCII format, with each
        /// line having columns: number | weight. Each column is
        /// separated by a single space. The first line is a comment
        /// line (starting with a '#') that indicates what each column
        /// contains.
        void write() const;

        /// @brief Read the weights information from a weights log
        /// @details The weights log file is opened and each channel's
        /// weights information is read and stored in the vector of weights
        /// values. The list of channel image names is also filled. If
        /// the weights log can not be opened, both vectors are cleared
        /// and an error message is written to the log.
        void read();

        /// @brief Gather channels from different ranks onto a single,
        /// nominated rank, combining the lists of channel information
        /// @details Each rank (other than the nominated one) sends
        /// the channel and weight information to the nominated
        /// rank. The weightslists are aggregated on that rank ready for
        /// writing, ignoring any channels that have zero weights.
        void gather(askapparallel::AskapParallel &comms, int rankToGather, bool includeMaster);


        /// @brief Return the weights information
        std::map<unsigned int, float> weightslist() const {return itsWeightsList;};

        /// @brief Return the weights information
        std::map<unsigned int, float>& weightslist() {return itsWeightsList;};

        /// @brief Return the weights for a given channel.
        /// @details Returns the weights stored for the requested channel. If
        /// the weights list does not have an entry for that channel,
        /// zero is returned.
        float weight(const unsigned int channel) const;

        /// @brief Return the weights as a record that can be written to an image
        /// @return Record with channel and weight vectors
        casacore::Record toRecord() const;

    protected:
        /// @brief Return true if weightslist is valid
        bool valid() const;

        /// @brief The disk file to be read from / written to
        std::string itsFilename;

        /// @brief The list of beam information. Each element of the map is a float,
        /// referenced by the channel number.
        std::map<unsigned int, float> itsWeightsList;

};

}
}

#endif
