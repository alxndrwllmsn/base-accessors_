/// @file
/// @brief helper interface used in conjunction with MemCalSolutionAccessor
/// @details All classes which know how to fill buffers of MemCalSolutionAccessor are
/// supposed to be derived from this interface. This approach allows to have the
/// solution source responsible for the actual reading while retaining a generic accessor
/// which can be reused with various implementations of the solution source
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_ACCESSORS_I_CAL_SOLUTION_FILLER_H
#define ASKAP_ACCESSORS_I_CAL_SOLUTION_FILLER_H

#include <utility>

#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/BasicSL/Complex.h>

namespace askap {

namespace accessors {

/// @brief helper interface used in conjunction with MemCalSolutionAccessor
/// @details All classes which know how to fill buffers of MemCalSolutionAccessor are
/// supposed to be derived from this interface. This approach allows to have the
/// solution source responsible for the actual reading while retaining a generic accessor
/// which can be reused with various implementations of the solution source
/// @ingroup calibaccess
struct ICalSolutionFiller {

  /// @brief virtual destructor to keep the compiler happy
  virtual ~ICalSolutionFiller() {};

  /// @brief gains filler
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (to be resized to 2 x nAnt x nBeam)
  virtual void fillGains(std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &gains) const = 0;

  /// @brief leakage filler
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (to be resized to 2 x nAnt x nBeam)
  virtual void fillLeakages(std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &leakages) const = 0;

  /// @brief bandpass filler
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (to be resized to (2*nChan) x nAnt x nBeam)
  virtual void fillBandpasses(std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &bp) const = 0;

  /// @brief bpleakage filler
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (to be resized to (2*nChan) x nAnt x nBeam)
  virtual void fillBPLeakages(std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &bpleakages) const = 0;

  /// @brief ionosphere filler
  /// @details
  /// @param[in] pair of cubes with ionospheric parameters and validity flags (to be resized to 1 x nParam x nDir)
  virtual void fillIonoParams(std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &params) const = 0;

  /// @brief gains writer
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeGains(const std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &gains) const = 0;

  /// @brief leakage writer
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeLeakages(const std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &leakages) const = 0;

  /// @brief bandpass writer
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (should be (2*nChan) x nAnt x nBeam)
  virtual void writeBandpasses(const std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &bp) const = 0;

  /// @brief bpleakage writer
  /// @details
  /// @param[in] bpleakages pair of cubes with bpleakages and validity flags (should be (2*nChan) x nAnt x nBeam)
  virtual void writeBPLeakages(const std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &bpleakages) const = 0;

  /// @brief ionospheric parameters writer
  /// @details
  /// @param[in] pair of cubes with ionospheric parameters and validity flags (should be 1 x nParam x nDir)
  virtual void writeIonoParams(const std::pair<casacore::Cube<casacore::Complex>, casacore::Cube<casacore::Bool> > &params) const = 0;

  // the following methods can be overriden to provide information that a particular solution doesn't exist at all
  // (and therefore reading should always return a default value). This allows to use read-only fillers without
  // giving a maximum number of antennas, beams and spectral channels. By default, these methods return that all
  // types of solutions exist

  /// @brief check for gain solution
  /// @return true, if there is no gain solution, false otherwise
  virtual bool noGain() const { return false; }

  /// @brief check for leakage solution
  /// @return true, if there is no leakage solution, false otherwise
  virtual bool noLeakage() const { return false; }

  /// @brief check for bandpass solution
  /// @return true, if there is no bandpass solution, false otherwise
  virtual bool noBandpass() const { return false; }

  /// @brief check for bpleakage solution
  /// @return true, if there is no bandpass leakage solution, false otherwise
  virtual bool noBPLeakage() const { return false; }

  /// @brief check for ionospheric solution
  /// @return true, if there is no ionospheric solution, false otherwise
  virtual bool noIonosphere() const { return false; }

  /// @brief flush the underlying data
  virtual bool flush() {return false;}

};


} // namespace accessors

} // namespace askap

#endif
