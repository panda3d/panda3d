/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramOutputFile.I
 * @author drose
 * @date 2000-10-27
 */

/**
 *
 */
INLINE DatagramOutputFile::
DatagramOutputFile() {
  _error = false;
  _wrote_first_datagram = false;
  _out = nullptr;
  _owns_out = false;
}

/**
 *
 */
INLINE DatagramOutputFile::
~DatagramOutputFile() {
  close();
}

/**
 * Opens the indicated filename for writing.  Returns true on success, false
 * on failure.
 */
INLINE bool DatagramOutputFile::
open(const Filename &filename) {
  return open(new FileReference(filename));
}

/**
 * Returns the ostream represented by the output file.
 */
INLINE std::ostream &DatagramOutputFile::
get_stream() {
  static std::ofstream null_stream;
  nassertr(_out != nullptr, null_stream);
  return *_out;
}
