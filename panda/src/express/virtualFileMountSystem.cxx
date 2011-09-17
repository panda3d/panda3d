// Filename: virtualFileMountSystem.cxx
// Created by:  drose (03Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "virtualFileMountSystem.h"

TypeHandle VirtualFileMountSystem::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::has_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::create_file
//       Access: Public, Virtual
//  Description: Attempts to create the indicated file within the
//               mount, if it does not already exist.  Returns true on
//               success (or if the file already exists), or false if
//               it cannot be created.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
create_file(const Filename &file) {
  Filename pathname(_physical_filename, file);
  pathname.set_binary();
  ofstream stream;
  return pathname.open_write(stream, false);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::make_directory
//       Access: Public, Virtual
//  Description: Attempts to create the indicated file within the
//               mount, if it does not already exist.  Returns true on
//               success, or false if it cannot be created.  If the
//               directory already existed prior to this call, may
//               return either true or false.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
make_directory(const Filename &file) {
  Filename pathname(_physical_filename, file);
  return pathname.mkdir();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::is_directory
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a directory.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::is_regular_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a regular file.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::is_writable
//       Access: Public, Virtual
//  Description: Returns true if the named file or directory may be
//               written to, false otherwise.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_read_file
//       Access: Public, Virtual
//  Description: Opens the file for reading, if it exists.  Returns a
//               newly allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFileMountSystem::
open_read_file(const Filename &file) const {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return NULL;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pifstream *stream = new pifstream;
  if (!pathname.open_read(*stream)) {
    // Couldn't open the file for some reason.
    close_read_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_write_file
//       Access: Published, Virtual
//  Description: Opens the file for writing.  Returns a newly
//               allocated ostream on success (which you should
//               eventually delete when you are done writing).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
ostream *VirtualFileMountSystem::
open_write_file(const Filename &file, bool truncate) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return NULL;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pofstream *stream = new pofstream;
  if (!pathname.open_write(*stream, truncate)) {
    // Couldn't open the file for some reason.
    close_write_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_append_file
//       Access: Published
//  Description: Works like open_write_file(), but the file is opened
//               in append mode.  Like open_write_file, the returned
//               pointer should eventually be passed to
//               close_write_file().
////////////////////////////////////////////////////////////////////
ostream *VirtualFileMountSystem::
open_append_file(const Filename &file) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return NULL;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pofstream *stream = new pofstream;
  if (!pathname.open_append(*stream)) {
    // Couldn't open the file for some reason.
    close_write_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_read_write_file
//       Access: Published, Virtual
//  Description: Opens the file for writing.  Returns a newly
//               allocated iostream on success (which you should
//               eventually delete when you are done writing).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
iostream *VirtualFileMountSystem::
open_read_write_file(const Filename &file, bool truncate) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return NULL;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pfstream *stream = new pfstream;
  if (!pathname.open_read_write(*stream, truncate)) {
    // Couldn't open the file for some reason.
    close_read_write_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_read_append_file
//       Access: Published, Virtual
//  Description: Works like open_read_write_file(), but the file is opened
//               in append mode.  Like open_read_write_file, the returned
//               pointer should eventually be passed to
//               close_read_write_file().
////////////////////////////////////////////////////////////////////
iostream *VirtualFileMountSystem::
open_read_append_file(const Filename &file) {
#ifdef WIN32
  // First ensure that the file exists to validate its case.
  if (VirtualFileSystem::get_global_ptr()->vfs_case_sensitive) {
    if (!has_file(file)) {
      return NULL;
    }
  }
#endif  // WIN32
  Filename pathname(_physical_filename, file);
  pfstream *stream = new pfstream;
  if (!pathname.open_read_append(*stream)) {
    // Couldn't open the file for some reason.
    close_read_write_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the already-open file.  Pass in the stream that
//               was returned by open_read_file(); some
//               implementations may require this stream to determine
//               the size.
////////////////////////////////////////////////////////////////////
off_t VirtualFileMountSystem::
get_file_size(const Filename &file, istream *stream) const {
  // First, save the original stream position.
  streampos orig = stream->tellg();

  // Seek to the end and get the stream position there.
  stream->seekg(0, ios::end);
  if (stream->fail()) {
    // Seeking not supported.
    stream->clear();
    return get_file_size(file);
  }
  streampos size = stream->tellg();

  // Then return to the original point.
  stream->seekg(orig, ios::beg);

  // Make sure there are no error flags set as a result of the seek.
  stream->clear();

  return size;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the file before it has been opened.
////////////////////////////////////////////////////////////////////
off_t VirtualFileMountSystem::
get_file_size(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.get_file_size();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::get_timestamp
//       Access: Published, Virtual
//  Description: Returns a time_t value that represents the time the
//               file was last modified, to within whatever precision
//               the operating system records this information (on a
//               Windows95 system, for instance, this may only be
//               accurate to within 2 seconds).
//
//               If the timestamp cannot be determined, either because
//               it is not supported by the operating system or
//               because there is some error (such as file not found),
//               returns 0.
////////////////////////////////////////////////////////////////////
time_t VirtualFileMountSystem::
get_timestamp(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.get_timestamp();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::get_system_info
//       Access: Public, Virtual
//  Description: Populates the SubfileInfo structure with the data
//               representing where the file actually resides on disk,
//               if this is knowable.  Returns true if the file might
//               reside on disk, and the info is populated, or false
//               if it does not (or it is not known where the file
//               resides), in which case the info is meaningless.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
get_system_info(const Filename &file, SubfileInfo &info) {
  Filename pathname(_physical_filename, file);
  info = SubfileInfo(pathname, 0, pathname.get_file_size());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::scan_directory
//       Access: Public, Virtual
//  Description: Fills the given vector up with the list of filenames
//               that are local to this directory, if the filename is
//               a directory.  Returns true if successful, or false if
//               the file is not a directory or cannot be read.
////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMountSystem::
output(ostream &out) const {
  out << get_physical_filename();
}
