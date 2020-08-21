/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmWriter.I
 * @author drose
 * @date 2000-06-16
 */

/**
 *
 */
INLINE PNMWriter::
PNMWriter(PNMFileType *type, std::ostream *file, bool owns_file) :
  _type(type),
  _owns_file(owns_file),
  _file(file),
  _is_valid(true)
{
}

/**
 * Returns a pointer to the PNMFileType object that created this PNMWriter.
 */
INLINE PNMFileType *PNMWriter::
get_type() const {
  return _type;
}

/**
 *
 */
INLINE void PNMWriter::
set_color_type(ColorType type) {
  set_num_channels((int)type);
}

/**
 *
 */
INLINE void PNMWriter::
set_num_channels(int num_channels) {
  nassertv(num_channels >= 1 && num_channels <= 4);
  _num_channels = num_channels;
}

/**
 *
 */
INLINE void PNMWriter::
set_maxval(xelval maxval) {
  _maxval = maxval;
}

/**
 *
 */
INLINE void PNMWriter::
set_x_size(int x_size) {
  nassertv(x_size >= 0);
  _x_size = x_size;
}

/**
 *
 */
INLINE void PNMWriter::
set_y_size(int y_size) {
  nassertv(y_size >= 0);
  _y_size = y_size;
}

/**
 * Initializes all the data in the header (x_size, y_size, num_channels, etc.)
 * to the same values indicated in the given header.  This should be done
 * before writing anything to the file.
 */
INLINE void PNMWriter::
copy_header_from(const PNMImageHeader &header) {
  PNMImageHeader::operator = (header);
}

/**
 * Returns true if the PNMWriter can be used to write data, false if something
 * is wrong.
 */
INLINE bool PNMWriter::
is_valid() const {
  return _is_valid;
}
