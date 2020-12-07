/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zipArchive.I
 * @author rdb
 * @date 2019-10-23
 */

/**
 * Returns the filename of the ZipArchive, if it is available.
 */
INLINE const Filename &ZipArchive::
get_filename() const {
  return _filename;
}

/**
 * Replaces the filename of the ZipArchive.  This is primarily used for
 * documentation purposes only; changing this name does not open the indicated
 * file.  See open_read() or open_write() for that.
 */
INLINE void ZipArchive::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 * Returns true if the ZipArchive has been opened for read mode and there have
 * been no errors, and individual Subfile contents may be extracted.
 */
INLINE bool ZipArchive::
is_read_valid() const {
  return (_read != nullptr);
}

/**
 * Returns true if the ZipArchive has been opened for write mode and there have
 * been no errors, and Subfiles may be added or removed from the ZipArchive.
 */
INLINE bool ZipArchive::
is_write_valid() const {
  return (_write != nullptr && !_write->fail());
}

/**
 * Returns true if the ZipArchive index is suboptimal and should be repacked.
 * Call repack() to achieve this.  It is not done automatically.
 */
INLINE bool ZipArchive::
needs_repack() const {
  return _needs_repack;
}

/**
 * Sets the flag indicating whether timestamps should be recorded within the
 * ZipArchive or not.  The default is true, indicating the ZipArchive will
 * record timestamps for each subfile that is added.
 *
 * If this is false, the ZipArchive will not record timestamps internally.  In
 * this case, the return value from get_subfile_timestamp() will be zero.
 *
 * You may want to set this false to minimize the bitwise difference between
 * independently-generated ZipArchives.
 */
INLINE void ZipArchive::
set_record_timestamp(bool flag) {
  _record_timestamp = flag;
}

/**
 * Returns the flag indicating whether timestamps should be recorded within
 * the ZipArchive or not.  See set_record_timestamp().
 */
INLINE bool ZipArchive::
get_record_timestamp() const {
  return _record_timestamp;
}

/**
 * Removes the named subfile from the ZipArchive, if it exists; returns true if
 * successfully removed, or false if it did not exist in the first place.  The
 * file will not actually be removed from the disk until the next call to
 * flush().
 *
 * Note that this does not actually remove the data from the indicated
 * subfile; it simply removes it from the index.  The ZipArchive will not be
 * reduced in size after this operation, until the next call to repack().
 */
INLINE bool ZipArchive::
remove_subfile(const std::string &subfile_name) {
  int index = find_subfile(subfile_name);
  if (index >= 0) {
    remove_subfile(index);
    return true;
  }
  return false;
}

/**
 * Returns a vector_uchar that contains the entire contents of the indicated
 * subfile.
 */
INLINE vector_uchar ZipArchive::
read_subfile(int index) {
  vector_uchar result;
  read_subfile(index, result);
  return result;
}

/**
 * Returns the comment string that was at the end of the ZIP end-of-directory
 * record, if any.
 * See set_comment().
 */
INLINE const std::string &ZipArchive::
get_comment() const {
  return _comment;
}

/**
 * Compares two Subfiles for proper sorting within the index.
 */
INLINE bool ZipArchive::Subfile::
operator < (const ZipArchive::Subfile &other) const {
  return _name < other._name;
}

/**
 * Returns true if this Subfile is compressed.
 */
INLINE bool ZipArchive::Subfile::
is_compressed() const {
  return _compression_method != CM_store;
}

/**
 * Returns true if this Subfile is encrypted.
 */
INLINE bool ZipArchive::Subfile::
is_encrypted() const {
  return (_flags & SF_encrypted) != 0;
}
