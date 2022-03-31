/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeAndroid.cxx
 * @author rdb
 * @date 2013-01-11
 */

#include "pnmFileTypeAndroid.h"

#ifdef ANDROID

#include "config_pnmimagetypes.h"

/**
 *
 */
PNMFileTypeAndroid::
PNMFileTypeAndroid(CompressFormat format) : _format(format) {
}

/**
 * Returns a few words describing the file type.
 */
std::string PNMFileTypeAndroid::
get_name() const {
  return "Android Bitmap";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeAndroid::
get_num_extensions() const {
  switch (_format) {
  case CF_jpeg:
    return 3;
  case CF_png:
    return 1;
  case CF_webp:
    return 1;
  default:
    return 0;
  }
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
std::string PNMFileTypeAndroid::
get_extension(int n) const {
  static const char *const jpeg_extensions[] = {"jpg", "jpeg", "jpe"};
  switch (_format) {
  case CF_jpeg:
    return jpeg_extensions[n];
  case CF_png:
    return "png";
  case CF_webp:
    return "webp";
  default:
    return 0;
  }
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeAndroid::
has_magic_number() const {
  return false;
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeAndroid::
make_reader(std::istream *file, bool owns_file, const std::string &magic_number) {
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeAndroid::
make_writer(std::ostream *file, bool owns_file) {
  return new Writer(this, file, owns_file, _format);
}

#endif  // ANDROID
