// Filename: transformAttrib.I
// Created by:  drose (23Feb02)
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
//     Function: TransformAttrib::Constructor
//       Access: Private
//  Description: Use TransformAttrib::make() to construct a new
//               TransformAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TransformAttrib::
TransformAttrib() {
  _flags = F_is_identity | F_singular_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::is_identity
//       Access: Published
//  Description: Returns true if the transform represents the identity
//               matrix, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformAttrib::
is_identity() const {
  return ((_flags & F_is_identity) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::is_singular
//       Access: Published
//  Description: Returns true if the transform represents a singular
//               transform (that is, it has a zero scale, and it
//               cannot be inverted), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformAttrib::
is_singular() const {
  check_singular();
  return ((_flags & F_is_singular) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::has_components
//       Access: Published
//  Description: Returns true if the transform can be described by
//               separate pos, hpr, and scale components.  Most
//               transforms we use in everyday life can be so
//               described, but some kinds of transforms (for
//               instance, those involving a skew) cannot.
//
//               This is not related to whether the transform was
//               originally described componentwise.  Even a transform
//               that was constructed with a 4x4 may return true here
//               if the matrix is a simple affine matrix with no skew.
//
//               If this returns true, you may safely call get_pos(),
//               etc., to retrieve the components.
////////////////////////////////////////////////////////////////////
INLINE bool TransformAttrib::
has_components() const {
  check_components();
  return ((_flags & F_has_components) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::get_pos
//       Access: Published
//  Description: Returns the pos component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformAttrib::
get_pos() const {
  check_components();
  nassertr(has_components(), _pos);
  return _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::get_hpr
//       Access: Published
//  Description: Returns the hpr component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformAttrib::
get_hpr() const {
  check_components();
  nassertr(has_components(), _hpr);
  return _hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::get_scale
//       Access: Published
//  Description: Returns the scale component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformAttrib::
get_scale() const {
  check_components();
  nassertr(has_components(), _scale);
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the transform.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &TransformAttrib::
get_mat() const {
  check_mat();
  return _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::check_singular
//       Access: Private
//  Description: Ensures that we know whether the matrix is singular.
////////////////////////////////////////////////////////////////////
INLINE void TransformAttrib::
check_singular() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_singular_known) == 0) {
    ((TransformAttrib *)this)->calc_singular();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::check_components
//       Access: Private
//  Description: Ensures that we know the components of the transform
//               (or that we know they cannot be derived).
////////////////////////////////////////////////////////////////////
INLINE void TransformAttrib::
check_components() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_components_known) == 0) {
    ((TransformAttrib *)this)->calc_components();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::check_mat
//       Access: Private
//  Description: Ensures that we know the overall matrix.
////////////////////////////////////////////////////////////////////
INLINE void TransformAttrib::
check_mat() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_mat_known) == 0) {
    ((TransformAttrib *)this)->calc_mat();
  }
}
