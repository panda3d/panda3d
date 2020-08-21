/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmReader.I
 * @author drose
 * @date 2000-06-16
 */

/**
 *
 */
INLINE PNMReader::
PNMReader(PNMFileType *type, std::istream *file, bool owns_file) :
  _type(type),
  _owns_file(owns_file),
  _file(file),
  _is_valid(true),
  _has_read_size(false)
{
}

/**
 * Instructs the reader to attempt to scale the image to the indicated size
 * while reading it.  The reader may or may not follow this suggestion, or may
 * follow it only partially (e.g.  by reading a file which is slightly reduced
 * in size, but not the precise size requested).
 */
void PNMReader::
set_read_size(int x_size, int y_size) {
  _read_x_size = x_size;
  _read_y_size = y_size;
  _has_read_size = true;
}

/**
 * Returns a pointer to the PNMFileType object that created this PNMReader.
 */
INLINE PNMFileType *PNMReader::
get_type() const {
  return _type;
}

/**
 * Returns true if the PNMReader can be used to read data, false if something
 * is wrong.
 */
INLINE bool PNMReader::
is_valid() const {
  return _is_valid;
}
