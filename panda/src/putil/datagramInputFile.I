/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramInputFile.I
 * @author drose
 * @date 2000-10-30
 */

/**
 *
 */
INLINE DatagramInputFile::
DatagramInputFile() {
  _error = false;
  _read_first_datagram = false;
  _in = nullptr;
  _owns_in = false;
  _timestamp = 0;
}

/**
 *
 */
INLINE DatagramInputFile::
~DatagramInputFile() {
  close();
}

/**
 * Opens the indicated filename for reading.  Returns true on success, false
 * on failure.
 */
INLINE bool DatagramInputFile::
open(const Filename &filename) {
  return open(new FileReference(filename));
}

/**
 * Returns the istream represented by the input file.
 */
INLINE std::istream &DatagramInputFile::
get_stream() {
  static std::ifstream null_stream;
  nassertr(_in != nullptr, null_stream);
  return *_in;
}
