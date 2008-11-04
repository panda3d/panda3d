// Filename: virtualFileMountMultifile.cxx
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

#include "virtualFileMountMultifile.h"
#include "virtualFileSystem.h"

TypeHandle VirtualFileMountMultifile::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMountMultifile::
~VirtualFileMountMultifile() {
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::has_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
has_file(const Filename &file) const {
  return (file.empty() ||
          _multifile->find_subfile(file) >= 0 ||
          _multifile->has_directory(file));
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::is_directory
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a directory.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
is_directory(const Filename &file) const {
  return (file.empty() || _multifile->has_directory(file));
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::is_regular_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a regular file.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
is_regular_file(const Filename &file) const {
  return (_multifile->find_subfile(file) >= 0);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::open_read_file
//       Access: Public, Virtual
//  Description: Opens the file for reading, if it exists.  Returns a
//               newly allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFileMountMultifile::
open_read_file(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return NULL;
  }
  return _multifile->open_read_subfile(subfile_index);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the already-open file.  Pass in the stream that
//               was returned by open_read_file(); some
//               implementations may require this stream to determine
//               the size.
////////////////////////////////////////////////////////////////////
off_t VirtualFileMountMultifile::
get_file_size(const Filename &file, istream *) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_length(subfile_index);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the file before it has been opened.
////////////////////////////////////////////////////////////////////
off_t VirtualFileMountMultifile::
get_file_size(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_length(subfile_index);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::get_timestamp
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
time_t VirtualFileMountMultifile::
get_timestamp(const Filename &file) const {
  int subfile_index = _multifile->find_subfile(file);
  if (subfile_index < 0) {
    return 0;
  }
  return _multifile->get_subfile_timestamp(subfile_index);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::scan_directory
//       Access: Public, Virtual
//  Description: Fills the given vector up with the list of filenames
//               that are local to this directory, if the filename is
//               a directory.  Returns true if successful, or false if
//               the file is not a directory or cannot be read.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
scan_directory(vector_string &contents, const Filename &dir) const {
  return _multifile->scan_directory(contents, dir);
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMountMultifile::
output(ostream &out) const {
  out << _multifile->get_multifile_name();
}
