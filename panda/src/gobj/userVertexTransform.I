// Filename: userVertexTransform.I
// Created by:  drose (24Mar05)
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
//     Function: UserVertexTransform::get_name
//       Access: Published
//  Description: Returns the name passed to the constructor.
//               Completely arbitrary.
////////////////////////////////////////////////////////////////////
INLINE const string &UserVertexTransform::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::set_matrix
//       Access: Published
//  Description: Stores the indicated matrix.
////////////////////////////////////////////////////////////////////
INLINE void UserVertexTransform::
set_matrix(const LMatrix4f &matrix) {
  CDWriter cdata(_cycler);
  cdata->_matrix = matrix;
  mark_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UserVertexTransform::CData::
CData() :
  _matrix(LMatrix4f::ident_mat())
{
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UserVertexTransform::CData::
CData(const UserVertexTransform::CData &copy) :
  _matrix(copy._matrix)
{
}
