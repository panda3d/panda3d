// Filename: matrixLens.I
// Created by:  drose (12Dec01)
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
//     Function: MatrixLens::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MatrixLens::
MatrixLens() :
  _user_mat(LMatrix4f::ident_mat())
{
  // The default film size for a MatrixLens is 2, which makes the
  // default range for both X and Y be [-1, 1].  This also,
  // incidentally, makes the film_mat be identity.
  set_film_size(2.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixLens::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MatrixLens::
MatrixLens(const MatrixLens &copy) : 
  Lens(copy),
  _user_mat(copy._user_mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixLens::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MatrixLens::
operator = (const MatrixLens &copy) {
  Lens::operator = (copy);
  _user_mat = copy.get_user_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixLens::set_user_mat
//       Access: Published
//  Description: Explicitly specifies the projection matrix.  This
//               matrix should convert X and Y to the range
//               [-film_size/2, film_size/2], where (-fs/2,-fs/2) is
//               the lower left corner of the screen and (fs/2, fs/2)
//               is the upper right.  Z should go to the range [-1,
//               1], where -1 is the far plane and 1 is the near
//               plane.  Note that this is a left-handed Y-up
//               coordinate system.
//
//               The default film_size for a MatrixLens is 2, so the
//               default range is [-1, 1] for both X and Y.  This is
//               consistent with the GL conventions for projection
//               matrices.
////////////////////////////////////////////////////////////////////
INLINE void MatrixLens::
set_user_mat(const LMatrix4f &user_mat) {
  _user_mat = user_mat;
  adjust_comp_flags(CF_mat, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixLens::get_user_mat
//       Access: Published
//  Description: Returns the explicit projection matrix as set by the
//               user.  This does not include transforms on the lens
//               or film (e.g. a film offset or view hpr).
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &MatrixLens::
get_user_mat() const {
  return _user_mat;
}
