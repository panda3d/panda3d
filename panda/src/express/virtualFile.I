// Filename: virtualFile.I
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


////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFile::
VirtualFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::get_original_filename
//       Access: Published
//  Description: Returns the original filename as it was used to
//               locate this VirtualFile.  This is usually, but not
//               always, the same string returned by get_filename().
////////////////////////////////////////////////////////////////////
INLINE const Filename &VirtualFile::
get_original_filename() const {
  return _original_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public
//  Description: Returns the entire contents of the file as a string.
////////////////////////////////////////////////////////////////////
INLINE string VirtualFile::
read_file(bool auto_unwrap) const {
  string result;
  read_file(result, auto_unwrap);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::write_file
//       Access: Public
//  Description: Writes the entire contents of the file as a string,
//               if it is writable.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFile::
write_file(const string &data, bool auto_wrap) {
  return write_file((const unsigned char *)data.data(), data.size(), auto_wrap);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::set_original_filename
//       Access: Public
//  Description: Stores the original filename that was used to locate
//               this VirtualFile.  This is normally called only by
//               the VirtualFileSystem, as it creates each
//               VirtualFile.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFile::
set_original_filename(const Filename &filename) {
  _original_filename = filename;
}


INLINE ostream &
operator << (ostream &out, const VirtualFile &file) {
  file.output(out);
  return out;
}

