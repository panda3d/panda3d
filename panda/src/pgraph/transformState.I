// Filename: transformState.I
// Created by:  drose (25Feb02)
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
//     Function: TransformState::Composition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformState::Composition::
Composition() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Composition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformState::Composition::
Composition(const TransformState::Composition &copy) :
  _result(copy._result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos(const LVecBase3f &pos) {
  return make_pos_hpr_scale(pos, 
                            LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_hpr
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_hpr(const LVecBase3f &hpr) {
  return make_pos_hpr_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                            hpr, 
                            LVecBase3f(1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_hpr
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_hpr(const LVecBase3f &pos, const LVecBase3f &hpr) {
  return make_pos_hpr_scale(pos, hpr,
                            LVecBase3f(1.0, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale(float scale) {
  return make_pos_hpr_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(scale, scale, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale(const LVecBase3f &scale) {
  return make_pos_hpr_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(0.0f, 0.0f, 0.0f),
                            scale);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_identity
//       Access: Published
//  Description: Returns true if the transform represents the identity
//               matrix, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_identity() const {
  return ((_flags & F_is_identity) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_invalid
//       Access: Published
//  Description: Returns true if the transform represents an invalid
//               matrix, for instance the result of inverting a
//               singular matrix, or false if the transform is valid.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_invalid() const {
  return ((_flags & F_is_invalid) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_singular
//       Access: Published
//  Description: Returns true if the transform represents a singular
//               transform (that is, it has a zero scale, and it
//               cannot be inverted), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_singular() const {
  check_singular();
  return ((_flags & F_is_singular) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_components
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
//               If this returns true, you may safely call get_hpr()
//               and get_scale() to retrieve the components.  (You
//               may always safely call get_pos() whether this returns
//               true or false.)
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_components() const {
  check_components();
  return ((_flags & F_has_components) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::components_given
//       Access: Published
//  Description: Returns true if the transform was specified
//               componentwise, or false if it was specified with a
//               general 4x4 matrix.  If this is true, the components
//               returned by get_pos(), get_hpr(), and get_scale()
//               will be exactly those that were set; otherwise, these
//               functions will return computed values.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
components_given() const {
  return ((_flags & F_components_given) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_pos
//       Access: Published
//  Description: Returns true if the transform's pos component can be
//               extracted out separately.  This is generally always
//               true, unless the transform is invalid
//               (i.e. is_invalid() returns true).
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_pos() const {
  return !is_invalid();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_hpr
//       Access: Published
//  Description: Returns true if the transform's rotation component
//               can be extracted out separately and described as a
//               set of Euler angles.  This is generally true only
//               when has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_hpr() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_scale
//       Access: Published
//  Description: Returns true if the transform's scale component
//               can be extracted out separately.  This is generally
//               true only when has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_scale() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_mat
//       Access: Published
//  Description: Returns true if the transform can be described as a
//               matrix.  This is generally always true, unless
//               is_invalid() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_mat() const {
  return !is_invalid();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_pos
//       Access: Published
//  Description: Returns the pos component of the transform.  It is an
//               error to call this if has_pos() returned false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_pos() const {
  check_components();
  nassertr(has_pos(), _pos);
  return _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_hpr
//       Access: Published
//  Description: Returns the hpr component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_hpr() const {
  check_components();
  nassertr(has_hpr(), _hpr);
  return _hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_scale
//       Access: Published
//  Description: Returns the scale component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_scale() const {
  check_components();
  nassertr(has_scale(), _scale);
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the transform.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &TransformState::
get_mat() const {
  nassertr(has_mat(), LMatrix4f::ident_mat());
  check_mat();
  return _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_singular
//       Access: Private
//  Description: Ensures that we know whether the matrix is singular.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_singular() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_singular_known) == 0) {
    ((TransformState *)this)->calc_singular();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_components
//       Access: Private
//  Description: Ensures that we know the components of the transform
//               (or that we know they cannot be derived).
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_components() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_components_known) == 0) {
    ((TransformState *)this)->calc_components();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_mat
//       Access: Private
//  Description: Ensures that we know the overall matrix.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_mat() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_mat_known) == 0) {
    ((TransformState *)this)->calc_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_destructing
//       Access: Private
//  Description: This function should only be called from the
//               destructor; it indicates that this TransformState
//               object is beginning destruction.  It is only used as
//               a sanity check, and is only meaningful when NDEBUG is
//               not defined.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
set_destructing() {
#ifndef NDEBUG
  _flags |= F_is_destructing;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_destructing
//       Access: Private
//  Description: Returns true if the TransformState object is
//               currently within its destructor
//               (i.e. set_destructing() has been called).  This is
//               only used as a sanity check, and is only meaningful
//               when NDEBUG is not defined.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_destructing() const {
#ifndef NDEBUG
  return (_flags & F_is_destructing) != 0;
#else
  return false;
#endif
}
