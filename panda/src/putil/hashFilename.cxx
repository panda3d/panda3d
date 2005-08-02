// Filename: hashFilename.cxx
// Created by:  drose (02Aug05)
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

#include "hashFilename.h"

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::get_filename_index
//       Access: Published
//  Description: Returns a Filename, derived from the HashFilename,
//               with the sequence of hash marks (if any) replaced by
//               the indicated index number.  If the HashFilename does
//               not contain a sequence of hash marks, this quietly
//               returns the original filename.
////////////////////////////////////////////////////////////////////
Filename HashFilename::
get_filename_index(int index) const {
  Filename file(*this);

  if (_hash_end != _hash_start) {
    ostringstream strm;
    strm << _filename.substr(0, _hash_start) 
	 << setw(_hash_end - _hash_start) << setfill('0') << index
	 << _filename.substr(_hash_end);
    file.set_fullpath(strm.str());
  }

  return file;
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::set_hash_to_end
//       Access: Published
//  Description: Replaces the part of the filename from the beginning
//               of the has sequence to the end of the filename.
////////////////////////////////////////////////////////////////////
void HashFilename::
set_hash_to_end(const string &s) {
  _filename.replace(_hash_start, string::npos, s);

  locate_basename();
  locate_extension();
  locate_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::locate_hash
//       Access: Private
//  Description: Identifies the part of the filename that contains the
//               sequence of hash marks, if any.
////////////////////////////////////////////////////////////////////
void HashFilename::
locate_hash() {
  _hash_end = _filename.rfind('#');
  if (_hash_end == string::npos) {
    _hash_end = string::npos;
    _hash_start = string::npos;

  } else {
    _hash_start = _hash_end;
    ++_hash_end;
    while (_hash_start > 0 && _filename[_hash_start - 1] == '#') {
      --_hash_start;
    }
  }
}
