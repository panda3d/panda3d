// Filename: virtualFileSystem.I
// Created by:  drose (03Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::exists
//       Access: Published
//  Description: Convenience function; returns true if the named file
//               exists.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileSystem::
exists(const Filename &filename) const {
  return get_file(filename) != (VirtualFile *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::is_directory
//       Access: Published
//  Description: Convenience function; returns true if the named file
//               exists and is a directory.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileSystem::
is_directory(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename);
  return (file != (VirtualFile *)NULL && file->is_directory());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::is_regular_file
//       Access: Published
//  Description: Convenience function; returns true if the named file
//               exists and is a regular file.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileSystem::
is_regular_file(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename);
  return (file != (VirtualFile *)NULL && file->is_regular_file());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::ls
//       Access: Published
//  Description: Convenience function; lists the files within the
//               indicated directory.  This accepts a string instead
//               of a Filename purely for programmer convenience at
//               the Python prompt.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFileSystem::
ls(const string &filename) const {
  PT(VirtualFile) file = get_file(filename);
  if (file == (VirtualFile *)NULL) {
    express_cat.info()
      << "Not found: " << filename << "\n";
  } else {
    file->ls();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::ls_all
//       Access: Published
//  Description: Convenience function; lists the files within the
//               indicated directory, and all files below,
//               recursively.  This accepts a string instead of a
//               Filename purely for programmer convenience at the
//               Python prompt.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFileSystem::
ls_all(const string &filename) const {
  PT(VirtualFile) file = get_file(filename);
  if (file == (VirtualFile *)NULL) {
    express_cat.info()
      << "Not found: " << filename << "\n";
  } else {
    file->ls_all();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::read_file
//       Access: Published
//  Description: Convenience function; returns the entire contents of
//               the indicated file as a string.
////////////////////////////////////////////////////////////////////
INLINE string VirtualFileSystem::
read_file(const Filename &filename) const {
  string result;
  bool okflag = read_file(filename, result);
  nassertr(okflag, string());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::open_read_file
//       Access: Published
//  Description: Convenience function; returns a newly allocated
//               istream if the file exists and can be read, or NULL
//               otherwise.  Does not return an invalid istream.
////////////////////////////////////////////////////////////////////
INLINE istream *VirtualFileSystem::
open_read_file(const Filename &filename) const {
  PT(VirtualFile) file = get_file(filename);
  if (file == (VirtualFile *)NULL) {
    return NULL;
  }
  istream *str = file->open_read_file();
  if (str != (istream *)NULL && str->fail()) {
    delete str;
    str = (istream *)NULL;
  }
  return str;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::read_file
//       Access: Public
//  Description: Convenience function; fills the string up with the
//               data from the indicated file, if it exists and can be
//               read.  Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileSystem::
read_file(const Filename &filename, string &result) const {
  PT(VirtualFile) file = get_file(filename);
  return (file != (VirtualFile *)NULL && file->read_file(result));
}
