// Filename: eggComment.I
// Created by:  drose (20Jan99)
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
//     Function: EggComment::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggComment::
EggComment(const string &node_name, const string &comment)
  : EggNode(node_name), _comment(comment) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggComment::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggComment::
EggComment(const EggComment &copy) : EggNode(copy), _comment(copy._comment) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggComment::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggComment &EggComment::
operator = (const string &comment) {
  _comment = comment;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggComment::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggComment &EggComment::
operator = (const EggComment &copy) {
  _comment = copy._comment;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggComment::String typecast operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggComment::
operator const string & () const {
  return _comment;
}


////////////////////////////////////////////////////////////////////
//     Function: EggComment::set_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggComment::
set_comment(const string &comment) {
  _comment = comment;
}


////////////////////////////////////////////////////////////////////
//     Function: EggComment::set_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string EggComment::
get_comment() const {
  return _comment;
}

