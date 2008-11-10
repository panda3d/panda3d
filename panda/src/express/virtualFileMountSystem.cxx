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
  if (file.is_text()) {
    pathname.set_text();
  } else {
    pathname.set_binary();
  }
  pifstream *stream = new pifstream;
  if (!pathname.open_read(*stream)) {
    // Couldn't open the file for some reason.
    close_read_file(stream);
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
get_file_size(const Filename &, istream *stream) const {
  // First, save the original stream position.
  streampos orig = stream->tellg();

  // Seek to the end and get the stream position there.
  stream->seekg(0, ios::end);
  streampos size = stream->tellg();

  // Then return to the original point.
  stream->seekg(orig, ios::beg);

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
