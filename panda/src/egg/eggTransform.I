/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTransform.I
 * @author drose
 * @date 2002-06-21
 */

/**
 *
 */
INLINE EggTransform::Component::
Component(EggTransform::ComponentType type, double number) :
  _type(type),
  _number(number)
{
  _vec2 = nullptr;
  _vec3 = nullptr;
  _mat3 = nullptr;
  _mat4 = nullptr;
}

/**
 *
 */
INLINE EggTransform::Component::
Component(const EggTransform::Component &copy) :
  _type(copy._type),
  _number(copy._number)
{
  _vec2 = nullptr;
  _vec3 = nullptr;
  _mat3 = nullptr;
  _mat4 = nullptr;
  if (copy._vec2 != nullptr) {
    _vec2 = new LVector2d(*copy._vec2);
  }
  if (copy._vec3 != nullptr) {
    _vec3 = new LVector3d(*copy._vec3);
  }
  if (copy._mat3 != nullptr) {
    _mat3 = new LMatrix3d(*copy._mat3);
  }
  if (copy._mat4 != nullptr) {
    _mat4 = new LMatrix4d(*copy._mat4);
  }
}

/**
 *
 */
INLINE void EggTransform::Component::
operator = (const EggTransform::Component &copy) {
  _type = copy._type;
  _number = copy._number;
  if (_vec2 != nullptr) {
    delete _vec2;
    _vec2 = nullptr;
  }
  if (_vec3 != nullptr) {
    delete _vec3;
    _vec3 = nullptr;
  }
  if (_mat3 != nullptr) {
    delete _mat3;
    _mat3 = nullptr;
  }
  if (_mat4 != nullptr) {
    delete _mat4;
    _mat4 = nullptr;
  }
  if (copy._vec2 != nullptr) {
    _vec2 = new LVecBase2d(*copy._vec2);
  }
  if (copy._vec3 != nullptr) {
    _vec3 = new LVecBase3d(*copy._vec3);
  }
  if (copy._mat3 != nullptr) {
    _mat3 = new LMatrix3d(*copy._mat3);
  }
  if (copy._mat4 != nullptr) {
    _mat4 = new LMatrix4d(*copy._mat4);
  }
}

/**
 *
 */
INLINE EggTransform::Component::
~Component() {
  if (_vec2 != nullptr) {
    delete _vec2;
  }
  if (_vec3 != nullptr) {
    delete _vec3;
  }
  if (_mat3 != nullptr) {
    delete _mat3;
  }
  if (_mat4 != nullptr) {
    delete _mat4;
  }
}

/**
 * Resets the transform to empty, identity.
 */
INLINE void EggTransform::
clear_transform() {
  internal_clear_transform();
  transform_changed();
}

/**
 * Appends an arbitrary 3x3 matrix to the current transform.
 */
INLINE void EggTransform::
add_matrix3(const LMatrix3d &mat) {
  internal_add_matrix(mat);
  transform_changed();
}

/**
 * Appends an arbitrary 4x4 matrix to the current transform.
 */
INLINE void EggTransform::
add_matrix4(const LMatrix4d &mat) {
  internal_add_matrix(mat);
  transform_changed();
}

/**
 * Returns true if the transform is nonempty, false if it is empty (no
 * transform components have been added).  This is true for either a 2-d or a
 * 3-d transform.
 */
INLINE bool EggTransform::
has_transform() const {
  return !_components.empty();
}

/**
 * Returns true if the transform is specified as a 2-d transform, e.g.  with a
 * 3x3 matrix, or false if it is specified as a 3-d transform (with a 4x4
 * matrix), or not specified at all.
 *
 * Normally, EggTextures have a 2-d matrix (but occasionally they use a 3-d
 * matrix), and EggGroups always have a 3-d matrix.
 */
INLINE bool EggTransform::
has_transform2d() const {
  return has_transform() && _is_transform_2d;
}

/**
 * Sets the overall transform as a 3x3 matrix.  This completely replaces
 * whatever componentwise transform may have been defined.
 */
INLINE void EggTransform::
set_transform2d(const LMatrix3d &mat) {
  internal_set_transform(mat);
  transform_changed();
}

/**
 * Returns true if the transform is specified as a 3-d transform, e.g.  with a
 * 4x4 matrix, or false if it is specified as a 2-d transform (with a 2x2
 * matrix), or not specified at all.
 *
 * Normally, EggTextures have a 3-d matrix (but occasionally they use a 3-d
 * matrix), and EggGroups always have a 3-d matrix.
 */
INLINE bool EggTransform::
has_transform3d() const {
  return has_transform() && !_is_transform_2d;
}

/**
 * Sets the overall transform as a 4x4 matrix.  This completely replaces
 * whatever componentwise transform may have been defined.
 */
INLINE void EggTransform::
set_transform3d(const LMatrix4d &mat) {
  internal_set_transform(mat);
  transform_changed();
}

/**
 * Returns the overall transform as a 3x3 matrix.  It is an error to call this
 * if has_transform3d() is true.
 */
INLINE LMatrix3d EggTransform::
get_transform2d() const {
  nassertr(!has_transform3d(), LMatrix3d::ident_mat());
  const LMatrix4d &t = _transform;
  return LMatrix3d(t(0, 0), t(0, 1), t(0, 3),
                   t(1, 0), t(1, 1), t(1, 3),
                   t(3, 0), t(3, 1), t(3, 3));
}

/**
 * Returns the overall transform as a 4x4 matrix.  It is valid to call this
 * even if has_transform2d() is true; in this case, the 3x3 transform will be
 * expanded to a 4x4 matrix.
 */
INLINE const LMatrix4d &EggTransform::
get_transform3d() const {
  return _transform;
}

/**
 * Returns true if the described transform is identity, false otherwise.
 */
INLINE bool EggTransform::
transform_is_identity() const {
  return _components.empty() ||
    _transform.almost_equal(LMatrix4d::ident_mat(), 0.0001);
}

/**
 * Returns the number of components that make up the transform.
 */
INLINE int EggTransform::
get_num_components() const {
  return _components.size();
}

/**
 * Returns the type of the nth component.
 */
INLINE EggTransform::ComponentType EggTransform::
get_component_type(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), CT_invalid);
  return _components[n]._type;
}

/**
 * Returns the solitary number associated with the nth component.  In the case
 * of a rotation, this is the angle in degrees to rotate; in the case of
 * uniform scale, this is the amount of the scale.  Other types do not use
 * this property.
 */
INLINE double EggTransform::
get_component_number(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), 0.0);
  return _components[n]._number;
}

/**
 * Returns the 2-component vector associated with the nth component.  This may
 * be the translate vector, rotate axis, or non-uniform scale.  It is an error
 * to call this if the component type does not use a 2-d vector property.
 */
INLINE const LVecBase2d &EggTransform::
get_component_vec2(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LVector2d::zero());
  nassertr(_components[n]._vec2 != nullptr, LVector2d::zero());
  return *_components[n]._vec2;
}

/**
 * Returns the 3-component vector associated with the nth component.  This may
 * be the translate vector, rotate axis, or non-uniform scale.  It is an error
 * to call this if the component type does not use a 3-d vector property.
 */
INLINE const LVecBase3d &EggTransform::
get_component_vec3(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LVector3d::zero());
  nassertr(_components[n]._vec3 != nullptr, LVector3d::zero());
  return *_components[n]._vec3;
}

/**
 * Returns the 3x3 matrix associated with the nth component.  It is an error
 * to call this if the component type is not CT_matrix3.
 */
INLINE const LMatrix3d &EggTransform::
get_component_mat3(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LMatrix3d::ident_mat());
  nassertr(_components[n]._mat3 != nullptr, LMatrix3d::ident_mat());
  return *_components[n]._mat3;
}

/**
 * Returns the 4x4 matrix associated with the nth component.  It is an error
 * to call this if the component type is not CT_matrix4.
 */
INLINE const LMatrix4d &EggTransform::
get_component_mat4(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), LMatrix4d::ident_mat());
  nassertr(_components[n]._mat4 != nullptr, LMatrix4d::ident_mat());
  return *_components[n]._mat4;
}

/**
 * Sets the overall transform without calling transform_changed().
 */
INLINE void EggTransform::
internal_set_transform(const LMatrix3d &mat) {
  internal_clear_transform();
  internal_add_matrix(mat);
}

/**
 * Sets the overall transform without calling transform_changed().
 */
INLINE void EggTransform::
internal_set_transform(const LMatrix4d &mat) {
  internal_clear_transform();
  internal_add_matrix(mat);
}
