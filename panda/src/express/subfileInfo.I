/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subfileInfo.I
 * @author drose
 * @date 2011-06-20
 */

/**
 *
 */
INLINE SubfileInfo::
SubfileInfo() :
  _start(0),
  _size(0)
{
}

/**
 *
 */
INLINE SubfileInfo::
SubfileInfo(const FileReference *file, std::streampos start, std::streamsize size) :
  _file(file),
  _start(start),
  _size(size)
{
}

/**
 *
 */
INLINE SubfileInfo::
SubfileInfo(const Filename &filename, std::streampos start, std::streamsize size) :
  _file(new FileReference(filename)),
  _start(start),
  _size(size)
{
}

/**
 *
 */
INLINE SubfileInfo::
SubfileInfo(const SubfileInfo &copy) :
  _file(copy._file),
  _start(copy._start),
  _size(copy._size)
{
}

/**
 *
 */
INLINE void SubfileInfo::
operator = (const SubfileInfo &copy) {
  _file = copy._file;
  _start = copy._start;
  _size = copy._size;
}

/**
 * Returns true if this SubfileInfo doesn't define any file, false if it has
 * real data.
 */
INLINE bool SubfileInfo::
is_empty() const {
  return _file == nullptr;
}

/**
 * Returns the FileReference that represents this file.
 */
INLINE const FileReference *SubfileInfo::
get_file() const {
  return _file;
}

/**
 * A shortcut to the filename.
 */
INLINE const Filename &SubfileInfo::
get_filename() const {
  if (_file != nullptr) {
    return _file->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

/**
 * Returns the offset within the file at which this file data begins.
 */
INLINE std::streampos SubfileInfo::
get_start() const {
  return _start;
}

/**
 * Returns the number of consecutive bytes, beginning at get_start(), that
 * correspond to this file data.
 */
INLINE std::streamsize SubfileInfo::
get_size() const {
  return _size;
}

INLINE std::ostream &
operator << (std::ostream &out, const SubfileInfo &info) {
  info.output(out);
  return out;
}
