/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountSystem.cxx
 * @author drose
 * @date 2002-08-03
 */

#include "virtualFileMountSystem.h"
#include "virtualFileSystem.h"

using std::iostream;
using std::istream;
using std::ostream;
using std::streampos;
using std::streamsize;
using std::string;

TypeHandle VirtualFileMountSystem::_type_handle;


/**
 * Returns true if the indicated file exists within the mount system.
 */
bool VirtualFileMountSystem::
has_file(const Filename &file) const {
  Filename pathname(_physical_filename, file);
#ifdef WIN32
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    Filename case_pathname = pathname;
    if (!case_pathname.make_true_case()) {
      return false;
    }
    if (case_pathname != pathname) {
      express_cat.warning()
        << "Filename is incorrect case: " << pathname
        << " instead of " << case_pathname << "\n";
      return false;
    }
  }
#endif  // WIN32
  return pathname.exists();
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success (or if the file already exists), or
 * false if it cannot be created.
 */
bool VirtualFileMountSystem::
create_file(const Filename &file) {
  Filename pathname(_physical_filename, file);
  pathname.set_binary();
  std::ofstream stream;
  return pathname.open_write(stream, false);
}

/**
 * Attempts to delete the indicated file or directory within the mount.  This
 * can remove a single file or an empty directory.  It will not remove a
 * nonempty directory.  Returns true on success, false on failure.
 */
bool VirtualFileMountSystem::
delete_file(const Filename &file) {
  Filename pathname(_physical_filename, file);
  return pathname.unlink() || pathname.rmdir();
}

/**
 * Attempts to rename the contents of the indicated file to the indicated
 * file.  Both filenames will be within the mount.  Returns true on success,
 * false on failure.  If this returns false, this will be attempted again with
 * a copy-and-delete operation.
 */
bool VirtualFileMountSystem::
rename_file(const Filename &orig_filename, const Filename &new_filename) {
  Filename orig_pathname(_physical_filename, orig_filename);
  Filename new_pathname(_physical_filename, new_filename);
  return orig_pathname.rename_to(new_pathname);
}

/**
 * Attempts to copy the contents of the indicated file to the indicated file.
 * Both filenames will be within the mount.  Returns true on success, false on
 * failure.  If this returns false, the copy will be performed by explicit
 * read-and-write operations.
 */
bool VirtualFileMountSystem::
copy_file(const Filename &orig_filename, const Filename &new_filename) {
  Filename orig_pathname(_physical_filename, orig_filename);
  Filename new_pathname(_physical_filename, new_filename);
  return orig_pathname.copy_to(new_pathname);
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success, or false if it cannot be created.
 * If the directory already existed prior to this call, may return either true
 * or false.
 */
bool VirtualFileMountSystem::
make_directory(const Filename &file) {
  Filename pathname(_physical_filename, file);
  return pathname.mkdir();
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * directory.
 */
bool VirtualFileMountSystem::
is_directory(const Filename &file) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  return pathname.is_directory();
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * regular file.
 */
bool VirtualFileMountSystem::
is_regular_file(const Filename &file) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  return pathname.is_regular_file();
}

/**
 * Returns true if the named file or directory may be written to, false
 * otherwise.
 */
bool VirtualFileMountSystem::
is_writable(const Filename &file) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  return pathname.is_writable();
}

/**
 * Opens the file for reading, if it exists.  Returns a newly allocated
 * istream on success (which you should eventually delete when you are done
 * reading). Returns NULL on failure.
 */
istream *VirtualFileMountSystem::
open_read_file(const Filename &file) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return nullptr;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pifstream *stream = new pifstream;
  if (!pathname.open_read(*stream)) {
    // Couldn't open the file for some reason.
    close_read_file(stream);
    return nullptr;
  }

  return stream;
}

/**
 * Opens the file for writing.  Returns a newly allocated ostream on success
 * (which you should eventually delete when you are done writing). Returns
 * NULL on failure.
 */
ostream *VirtualFileMountSystem::
open_write_file(const Filename &file, bool truncate) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return nullptr;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pofstream *stream = new pofstream;
  if (!pathname.open_write(*stream, truncate)) {
    // Couldn't open the file for some reason.
    close_write_file(stream);
    return nullptr;
  }

  return stream;
}

/**
 * Works like open_write_file(), but the file is opened in append mode.  Like
 * open_write_file, the returned pointer should eventually be passed to
 * close_write_file().
 */
ostream *VirtualFileMountSystem::
open_append_file(const Filename &file) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return nullptr;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pofstream *stream = new pofstream;
  if (!pathname.open_append(*stream)) {
    // Couldn't open the file for some reason.
    close_write_file(stream);
    return nullptr;
  }

  return stream;
}

/**
 * Opens the file for writing.  Returns a newly allocated iostream on success
 * (which you should eventually delete when you are done writing). Returns
 * NULL on failure.
 */
iostream *VirtualFileMountSystem::
open_read_write_file(const Filename &file, bool truncate) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return nullptr;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pfstream *stream = new pfstream;
  if (!pathname.open_read_write(*stream, truncate)) {
    // Couldn't open the file for some reason.
    close_read_write_file(stream);
    return nullptr;
  }

  return stream;
}

/**
 * Works like open_read_write_file(), but the file is opened in append mode.
 * Like open_read_write_file, the returned pointer should eventually be passed
 * to close_read_write_file().
 */
iostream *VirtualFileMountSystem::
open_read_append_file(const Filename &file) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return nullptr;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pfstream *stream = new pfstream;
  if (!pathname.open_read_append(*stream)) {
    // Couldn't open the file for some reason.
    close_read_write_file(stream);
    return nullptr;
  }

  return stream;
}

/**
 * Returns the current size on disk (or wherever it is) of the already-open
 * file.  Pass in the stream that was returned by open_read_file(); some
 * implementations may require this stream to determine the size.
 */
streamsize VirtualFileMountSystem::
get_file_size(const Filename &file, istream *stream) const {
  // First, save the original stream position.
  streampos orig = stream->tellg();

  // Seek to the end and get the stream position there.
  stream->seekg(0, std::ios::end);
  if (stream->fail()) {
    // Seeking not supported.
    stream->clear();
    return get_file_size(file);
  }
  streampos size = stream->tellg();

  // Then return to the original point.
  stream->seekg(orig, std::ios::beg);

  // Make sure there are no error flags set as a result of the seek.
  stream->clear();

  return size;
}

/**
 * Returns the current size on disk (or wherever it is) of the file before it
 * has been opened.
 */
streamsize VirtualFileMountSystem::
get_file_size(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.get_file_size();
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
time_t VirtualFileMountSystem::
get_timestamp(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.get_timestamp();
}

/**
 * Populates the SubfileInfo structure with the data representing where the
 * file actually resides on disk, if this is knowable.  Returns true if the
 * file might reside on disk, and the info is populated, or false if it does
 * not (or it is not known where the file resides), in which case the info is
 * meaningless.
 */
bool VirtualFileMountSystem::
get_system_info(const Filename &file, SubfileInfo &info) {
  Filename pathname(_physical_filename, file);
  info = SubfileInfo(pathname, 0, pathname.get_file_size());
  return true;
}

/**
 * Fills the given vector up with the list of filenames that are local to this
 * directory, if the filename is a directory.  Returns true if successful, or
 * false if the file is not a directory or cannot be read.
 */
bool VirtualFileMountSystem::
scan_directory(vector_string &contents, const Filename &dir) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(dir)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, dir);
  return pathname.scan_directory(contents);
}


/**
 * See Filename::atomic_compare_and_exchange_contents().
 */
bool VirtualFileMountSystem::
atomic_compare_and_exchange_contents(const Filename &file, string &orig_contents,
                                     const string &old_contents,
                                     const string &new_contents) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  return pathname.atomic_compare_and_exchange_contents(orig_contents, old_contents, new_contents);
}

/**
 * See Filename::atomic_read_contents().
 */
bool VirtualFileMountSystem::
atomic_read_contents(const Filename &file, string &contents) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return false;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  return pathname.atomic_read_contents(contents);
}

/**
 *
 */
void VirtualFileMountSystem::
output(ostream &out) const {
  out << get_physical_filename();
}
