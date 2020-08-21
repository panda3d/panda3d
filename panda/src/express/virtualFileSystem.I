/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileSystem.I
 * @author drose
 * @date 2002-08-03
 */

/**
 * Convenience function; returns true if the named file exists.
 */
INLINE bool VirtualFileSystem::
exists(const Filename &filename) const {
  return get_file(filename, true) != nullptr;
}

/**
 * Convenience function; returns true if the named file exists and is a
 * directory.
 */
INLINE bool VirtualFileSystem::
is_directory(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename, true);
  return (file != nullptr && file->is_directory());
}

/**
 * Convenience function; returns true if the named file exists and is a
 * regular file.
 */
INLINE bool VirtualFileSystem::
is_regular_file(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename, true);
  return (file != nullptr && file->is_regular_file());
}

/**
 * If the file represents a directory (that is, is_directory() returns true),
 * this returns the list of files within the directory at the current time.
 * Returns NULL if the file is not a directory or if the directory cannot be
 * read.
 */
INLINE PT(VirtualFileList) VirtualFileSystem::
scan_directory(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename, true);
  if (file == nullptr) {
    return nullptr;
  }

  return file->scan_directory();
}

/**
 * Convenience function; lists the files within the indicated directory.
 */
INLINE void VirtualFileSystem::
ls(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename, true);
  if (file == nullptr) {
    express_cat.info()
      << "Not found: " << filename << "\n";
  } else {
    file->ls();
  }
}

/**
 * Convenience function; lists the files within the indicated directory, and
 * all files below, recursively.
 */
INLINE void VirtualFileSystem::
ls_all(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename, true);
  if (file == nullptr) {
    express_cat.info()
      << "Not found: " << filename << "\n";
  } else {
    file->ls_all();
  }
}

/**
 * Convenience function; returns the entire contents of the indicated file as
 * a string.
 *
 * If auto_unwrap is true, an explicitly-named .pz/.gz file is automatically
 * decompressed and the decompressed contents are returned.  This is different
 * than vfs-implicit-pz, which will automatically decompress a file if the
 * extension .pz is *not* given.
 */
INLINE std::string VirtualFileSystem::
read_file(const Filename &filename, bool auto_unwrap) const {
  std::string result;
  bool okflag = read_file(filename, result, auto_unwrap);
  nassertr(okflag, std::string());
  return result;
}

/**
 * Convenience function; writes the entire contents of the indicated file as a
 * string.
 *
 * If auto_wrap is true, an explicitly-named .pz file is automatically
 * compressed while writing.
 */
INLINE bool VirtualFileSystem::
write_file(const Filename &filename, const std::string &data, bool auto_wrap) {
  return write_file(filename, (const unsigned char *)data.data(), data.size(), auto_wrap);
}

/**
 * Convenience function; fills the string up with the data from the indicated
 * file, if it exists and can be read.  Returns true on success, false
 * otherwise.
 *
 * If auto_unwrap is true, an explicitly-named .pz/.gz file is automatically
 * decompressed and the decompressed contents are returned.  This is different
 * than vfs-implicit-pz, which will automatically decompress a file if the
 * extension .pz is *not* given.
 */
INLINE bool VirtualFileSystem::
read_file(const Filename &filename, std::string &result, bool auto_unwrap) const {
  PT(VirtualFile) file = get_file(filename, false);
  return (file != nullptr && file->read_file(result, auto_unwrap));
}

/**
 * Convenience function; fills the pvector up with the data from the indicated
 * file, if it exists and can be read.  Returns true on success, false
 * otherwise.
 *
 * If auto_unwrap is true, an explicitly-named .pz/.gz file is automatically
 * decompressed and the decompressed contents are returned.  This is different
 * than vfs-implicit-pz, which will automatically decompress a file if the
 * extension .pz is *not* given.
 */
INLINE bool VirtualFileSystem::
read_file(const Filename &filename, vector_uchar &result, bool auto_unwrap) const {
  PT(VirtualFile) file = get_file(filename, false);
  return (file != nullptr && file->read_file(result, auto_unwrap));
}

/**
 * Convenience function; writes the entire contents of the indicated file as a
 * block of data.
 *
 * If auto_wrap is true, an explicitly-named .pz file is automatically
 * compressed while writing.
 */
INLINE bool VirtualFileSystem::
write_file(const Filename &filename, const unsigned char *data, size_t data_size, bool auto_wrap) {
  PT(VirtualFile) file = create_file(filename);
  return (file != nullptr && file->write_file(data, data_size, auto_wrap));
}
