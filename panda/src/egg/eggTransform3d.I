// Filename: eggTransform3d.I
// Created by:  drose (21Jun02)
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
//     Function: EggTransform3d::Component::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggTransform3d::Component::
Component(EggTransform3d::ComponentType type, double number) :
  _type(type),
  _number(number)
{
  _vector = (LVector3d *)NULL;
  _matrix = (LMatrix4d *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Component::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggTransform3d::Component::
Component(const EggTransform3d::Component &copy) :
  _type(copy._type),
  _number(copy._number)
{
  _vector = (LVector3d *)NULL;
  _matrix = (LMatrix4d *)NULL;
  if (copy._vector != (LVector3d *)NULL) {
    _vector = new LVector3d(*copy._vector);
  }
  if (copy._matrix != (LMatrix4d *)NULL) {
    _matrix = new LMatrix4d(*copy._matrix);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Component::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggTransform3d::Component::
operator = (const EggTransform3d::Component &copy) {
  _type = copy._type;
  _number = copy._number;
  if (_vector != (LVector3d *)NULL) {
    delete _vector;
    _vector = (LVector3d *)NULL;
  }
  if (_matrix != (LMatrix4d *)NULL) {
    delete _matrix;
    _matrix = (LMatrix4d *)NULL;
  }
  if (copy._vector != (LVector3d *)NULL) {
    _vector = new LVector3d(*copy._vector);
  }
  if (copy._matrix != (LMatrix4d *)NULL) {
    _matrix = new LMatrix4d(*copy._matrix);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Component::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggTransform3d::Component::
~Component() {
  if (_vector != (LVector3d *)NULL) {
    delete _vector;
  }
  if (_matrix != (LMatrix4d *)NULL) {
    delete _matrix;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::clear_transform
//       Access: Public
//  Description: Resets the transform to empty, identity.
////////////////////////////////////////////////////////////////////
INLINE void EggTransform3d::
clear_transform() {
  internal_clear_transform();
  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_matrix
//       Access: Public
//  Description: Appends an arbitrary 4x4 matrix to the current
//               transform.
////////////////////////////////////////////////////////////////////
INLINE void EggTransform3d::
add_matrix(const LMatrix4d &mat) {
  internal_add_matrix(mat);
  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::has_transform
//       Access: Public
//  Description: Returns true if the transform is nonempty, false if
//               it is empty (no transform components have been
//               added).
////////////////////////////////////////////////////////////////////
INLINE bool EggTransform3d::
has_transform() const {
  return !_components.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::set_transform
//       Access: Public
//  Description: Sets the overall transform as a 4x4 matrix.  This
//               completely replaces whatever componentwise transform
//               may have been defined.
////////////////////////////////////////////////////////////////////
INLINE void EggTransform3d::
set_transform(const LMatrix4d &mat) {
  internal_set_transform(mat);
  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_transform
//       Access: Public
//  Description: Returns the overall transform as a 4x4 matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggTransform3d::
get_transform() const {
  return _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::transform_is_identity
//       Access: Public
//  Description: Returns true if the described transform is identity,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggTransform3d::
transform_is_identity() const {
  return _components.empty() ||
    _transform.almost_equal(LMatrix4d::ident_mat(), 0.0001);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_num_components
//       Access: Public
//  Description: Returns the number of components that make up the
//               transform.
////////////////////////////////////////////////////////////////////
INLINE int EggTransform3d::
get_num_components() const {
  return _components.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_component_type
//       Access: Public
//  Description: Returns the type of the nth component.
////////////////////////////////////////////////////////////////////
INLINE EggTransform3d::ComponentType EggTransform3d::
get_component_type(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), CT_invalid);
  return _components[n]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_component_number
//       Access: Public
//  Description: Returns the solitary number associated with the nth
//               component.  In the case of a rotation, this is the
//               angle in degrees to rotate; in the case of uniform
//               scale, this is the amount of the scale.  Other types
//               do not use this property.
////////////////////////////////////////////////////////////////////
INLINE double EggTransform3d::
get_component_number(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), 0.0);
  return _components[n]._number;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_component_vector
//       Access: Public
//  Description: Returns the 3-component vector associated with the
//               nth component.  This may be the translate vector,
//               rotate axis, or non-uniform scale.  It is an error to
//               call this if the component type does not use a vector
//               property.
////////////////////////////////////////////////////////////////////
INLINE const LVector3d &EggTransform3d::
get_component_vector(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LVector3d::zero());
  nassertr(_components[n]._vector != (LVector3d *)NULL, LVector3d::zero());
  return *_components[n]._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::get_component_matrix
//       Access: Public
//  Description: Returns the 4x4 matrix associated with the nth
//               component.  It is an error to call this if the
//               component type is not CT_matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &EggTransform3d::
get_component_matrix(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LMatrix4d::ident_mat());
  nassertr(_components[n]._matrix != (LMatrix4d *)NULL, LMatrix4d::ident_mat());
  return *_components[n]._matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::internal_set_transform
//       Access: Protected
//  Description: Sets the overall transform without calling
//               transform_changed().
////////////////////////////////////////////////////////////////////
INLINE void EggTransform3d::
internal_set_transform(const LMatrix4d &mat) {
  internal_clear_transform();
  internal_add_matrix(mat);
}
