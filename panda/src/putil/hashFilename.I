// Filename: hashFilename.I
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


////////////////////////////////////////////////////////////////////
//     Function: HashFilename::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE HashFilename::
HashFilename(const string &filename_pattern) :
  Filename(filename_pattern)
{
  locate_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE HashFilename::
HashFilename(const HashFilename &copy) :
  Filename(copy),
  _hash_start(copy._hash_start),
  _hash_end(copy._hash_end)
{
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashFilename::
operator = (const HashFilename &copy) {
  Filename::operator = (copy);
  _hash_start = copy._hash_start;
  _hash_end = copy._hash_end;
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashFilename::
operator = (const Filename &copy) {
  Filename::operator = (copy);
  locate_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE HashFilename::
~HashFilename() {
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::has_hash
//       Access: Published
//  Description: Returns true if the filename pattern did include a
//               sequence of hash marks, false otherwise.  If this is
//               true, then get_filename_index() will return a
//               different filename each time.
////////////////////////////////////////////////////////////////////
INLINE bool HashFilename::
has_hash() const {
  return (_hash_start != _hash_end);
}

////////////////////////////////////////////////////////////////////
//     Function: HashFilename::get_hash_to_end
//       Access: Published
//  Description: Returns the part of the filename beginning at the
//               hash sequence (if any), and continuing to the end of
//               the filename.
////////////////////////////////////////////////////////////////////
INLINE string HashFilename::
get_hash_to_end() const {
  return _filename.substr(_hash_start);
}
