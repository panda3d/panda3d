/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountMultifile.cxx
 * @author drose
 * @date 2002-08-03
 */

#include "virtualFileMountMultifile.h"
#include "virtualFileSystem.h"

TypeHandle VirtualFileMountMultifile::_type_handle;


/**
 *
 */
VirtualFileMountMultifile::
~VirtualFileMountMultifile() {
}


/**
 * Returns true if the indicated file exists within the mount system.
 */
bool VirtualFileMountMultifile::
has_file(const Filename &file) const {
  return (file.empty() ||
          _multifile->find_subfile(file) >= 0 ||
          _multifile->has_directory(file));
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * directory.
 */
bool VirtualFileMountMultifile::
is_directory(const Filename &file) const {
  return (file.empty() || _multifile->has_directory(file));
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * regular file.
 */
bool VirtualFileMountMultifile::
is_regular_file(const Filename &file) const {
  return (_multifile->find_subfile(file) >= 0);
}

/**
 * Fills up the indicated pvector with the contents of the file, if it is a
 * regular file.  Returns true on success, false otherwise.
 */
bool VirtualFileMountMultifile::
read_file(const Filename &file, bool do_uncompress,
          vector_uchar &result) const {
  if (do_uncompress) {
    // If the file is to be decompressed, we'd better just use the higher-
    // level implementation, which includes support for on-the-fly
    // decompression.
    return VirtualFileMount::read_file(file, do_uncompress, result);
  }

  // But if we're just reading a straight file, let the Multifile do the
  // reading, which avoids a few levels of buffer copies.

  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    express_cat.info()
      << "Unable to read " << file << "\n";
    return false;
  }

  return _multifile->read_subfile(subfile_index, result);
}

/**
 * Opens the file for reading, if it exists.  Returns a newly allocated
 * istream on success (which you should eventually delete when you are done
 * reading). Returns NULL on failure.
 */
std::istream *VirtualFileMountMultifile::
open_read_file(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return nullptr;
  }

  // The caller will eventually pass this pointer to
  // VirtualFileSystem::close_read_file(), not to
  // Multifile::close_read_subfile().  Fortunately, these two methods do the
  // same thing, so that doesn't matter.
  return _multifile->open_read_subfile(subfile_index);
}

/**
 * Returns the current size on disk (or wherever it is) of the already-open
 * file.  Pass in the stream that was returned by open_read_file(); some
 * implementations may require this stream to determine the size.
 */
std::streamsize VirtualFileMountMultifile::
get_file_size(const Filename &file, std::istream *) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_length(subfile_index);
}

/**
 * Returns the current size on disk (or wherever it is) of the file before it
 * has been opened.
 */
std::streamsize VirtualFileMountMultifile::
get_file_size(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_length(subfile_index);
}

/**
 * Returns a time_t value that represents the time the file was last modified,
 * to within whatever precision the operating system records this information
 * (on a Windows95 system, for instance, this may only be accurate to within 2
 * seconds).
 *
 * If the timestamp cannot be determined, either because it is not supported
 * by the operating system or because there is some error (such as file not
 * found), returns 0.
 */
time_t VirtualFileMountMultifile::
get_timestamp(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_timestamp(subfile_index);
}

/**
 * Populates the SubfileInfo structure with the data representing where the
 * file actually resides on disk, if this is knowable.  Returns true if the
 * file might reside on disk, and the info is populated, or false if it might
 * not (or it is not known where the file resides), in which case the info is
 * meaningless.
 */
bool VirtualFileMountMultifile::
get_system_info(const Filename &file, SubfileInfo &info) {
  Filename multifile_name = _multifile->get_multifile_name();
  if (multifile_name.empty()) {
    return false;
  }
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return false;
  }
  if (_multifile->is_subfile_compressed(subfile_index) ||
      _multifile->is_subfile_encrypted(subfile_index)) {
    return false;
  }

  std::streampos start = _multifile->get_subfile_internal_start(subfile_index);
  size_t length = _multifile->get_subfile_internal_length(subfile_index);

  info = SubfileInfo(multifile_name, start, length);
  return true;
}

/**
 * Fills the given vector up with the list of filenames that are local to this
 * directory, if the filename is a directory.  Returns true if successful, or
 * false if the file is not a directory or cannot be read.
 */
bool VirtualFileMountMultifile::
scan_directory(vector_string &contents, const Filename &dir) const {
  return _multifile->scan_directory(contents, dir);
}


/**
 *
 */
void VirtualFileMountMultifile::
output(std::ostream &out) const {
  out << _multifile->get_multifile_name();
}
