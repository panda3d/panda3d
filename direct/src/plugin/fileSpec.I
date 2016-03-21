/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fileSpec.I
 * @author drose
 * @date 2009-06-29
 */

/**
 * Returns the relative path to this file on disk, within the package root
 * directory.
 */
inline const string &FileSpec::
get_filename() const {
  return _filename;
}

/**
 * Changes the relative path to this file on disk, within the package root
 * directory.
 */
inline void FileSpec::
set_filename(const string &filename) {
  _filename = filename;
}

/**
 * Returns the full path to this file on disk.
 */
inline string FileSpec::
get_pathname(const string &package_dir) const {
  return package_dir + "/" + _filename;
}

/**
 * Returns the expected size of this file on disk, in bytes.
 */
inline size_t FileSpec::
get_size() const {
  return _size;
}

/**
 * Returns the expected last-modify timestamp of this file on disk.
 */
inline time_t FileSpec::
get_timestamp() const {
  return _timestamp;
}

/**
 * Returns true if we have successfully read a hash value, false otherwise.
 */
inline bool FileSpec::
has_hash() const {
  return _got_hash;
}

/**
 * After a call to quick_verify() or full_verify(), this method *may* return a
 * pointer to a FileSpec that represents the actual data read on disk, or it
 * may return NULL.  If this returns a non-NULL value, you may use it to
 * extract the md5 hash of the existing file, thus saving the effort of
 * performing the hash twice.
 */
inline const FileSpec *FileSpec::
get_actual_file() const {
  return _actual_file;
}

/**
 * Returns the integer value corresponding to the indicated hex digit.
 * Returns -1 if it is not a hex digit.
 */
inline int FileSpec::
decode_hexdigit(char c) {
  if (isdigit(c)) {
    return c - '0';
  }
  c = tolower(c);
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  return -1;
}

/**
 * Returns the hex digit corresponding to the indicated integer value.
 */
inline char FileSpec::
encode_hexdigit(int c) {
  if (c >= 10) {
    return c - 10 + 'a';
  }
  return c + '0';
}
