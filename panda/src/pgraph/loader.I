// Filename: loader.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
Results(const Loader::Results &copy) :
  _files(copy._files)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
operator = (const Loader::Results &copy) {
  _files = copy._files;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
~Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::clear
//       Access: Published
//  Description: Removes all the files from the list.
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
clear() {
  _files.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_num_files
//       Access: Published
//  Description: Returns the number of files on the result list.
////////////////////////////////////////////////////////////////////
INLINE int Loader::Results::
get_num_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_file
//       Access: Published
//  Description: Returns the nth file on the result list.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Loader::Results::
get_file(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), _files[0]._path);
  return _files[n]._path;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_file_type
//       Access: Published
//  Description: Returns the file type of the nth file on the result
//               list.
////////////////////////////////////////////////////////////////////
INLINE LoaderFileType *Loader::Results::
get_file_type(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), NULL);
  return _files[n]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::add_file
//       Access: Published
//  Description: Adds a new file to the result list.
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
add_file(const Filename &file, LoaderFileType *type) {
  ConsiderFile cf;
  cf._path = file;
  cf._type = type;
  _files.push_back(cf);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_sync
//       Access: Published
//  Description: Loads the file immediately, waiting for it to
//               complete.
//
//               If search is true, the file is searched for along the
//               model path; otherwise, only the exact filename is
//               loaded.
////////////////////////////////////////////////////////////////////
INLINE PT(PandaNode) Loader::
load_sync(const Filename &filename, bool search) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return load_file(filename, search);
}
