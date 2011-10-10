// Filename: userVertexTransform.I
// Created by:  drose (24Mar05)
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
set_matrix(const LMatrix4 &matrix) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_matrix = matrix;
  mark_modified(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UserVertexTransform::CData::
CData() :
  _matrix(LMatrix4::ident_mat())
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
