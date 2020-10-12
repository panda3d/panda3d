/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stTransform.I
 * @author drose
 * @date 2010-10-06
 */

/**
 * The default constructor creates an identity transform.
 */
INLINE STTransform::
STTransform() :
  _pos(0.0f, 0.0f, 0.0f),
  _rotate(0.0f),
  _scale(1.0f)
{
}

/**
 * Construct a transform with componentwise inputs.
 */
INLINE STTransform::
STTransform(const LPoint3 &pos, PN_stdfloat rotate, PN_stdfloat scale) :
  _pos(pos),
  _rotate(rotate),
  _scale(scale)
{
}

/**
 * Construct a transform with componentwise inputs.
 */
INLINE STTransform::
STTransform(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat rotate, PN_stdfloat scale) :
  _pos(x, y, z),
  _rotate(rotate),
  _scale(scale)
{
}

/**
 *
 */
INLINE STTransform::
STTransform(const STTransform &copy) :
  _pos(copy._pos),
  _rotate(copy._rotate),
  _scale(copy._scale)
{
}

/**
 *
 */
INLINE void STTransform::
operator = (const STTransform &copy) {
  _pos = copy._pos;
  _rotate = copy._rotate;
  _scale = copy._scale;
}

/**
 * This is used internally to construct an STTransform from a
 * SpeedTree::CInstance object.
 */
INLINE STTransform::
STTransform(const SpeedTree::CInstance &instance) {
  const SpeedTree::Vec3 &pos = instance.GetPos();
  _pos.set(pos[0], pos[1], pos[2]);
  _rotate = rad_2_deg(instance.GetRotationAngle());
  _scale = instance.GetScale();
}

/**
 * This is used internally to convert an STTransform into a
 * SpeedTree::CInstance object.
 */
INLINE STTransform::
operator SpeedTree::CInstance () const {
  SpeedTree::CInstance instance;
  instance.SetPos(SpeedTree::Vec3(_pos[0], _pos[1], _pos[2]));
  instance.SetRotation(deg_2_rad(_rotate));
  instance.SetScale(_scale);
  return instance;
}

/**
 * This is used internally to convert an STTransform into a TransformState
 * pointer.
 */
INLINE STTransform::
operator CPT(TransformState) () const {
  return TransformState::make_pos_hpr_scale(_pos,
                                            LVecBase3(_rotate, 0.0f, 0.0f),
                                            LVecBase3(_scale, _scale, _scale));
}

/**
 * Returns a global identity transform object.
 */
INLINE const STTransform &STTransform::
ident_mat() {
  return _ident_mat;
}

/**
 * Replaces the translation component.
 */
INLINE void STTransform::
set_pos(const LPoint3 &pos) {
  _pos = pos;
}

/**
 * Returns the translation component.
 */
INLINE const LPoint3 &STTransform::
get_pos() const {
  return _pos;
}

/**
 * Replaces the rotation component.  Accepts a rotation in degrees counter-
 * clockwise around the vertical axis.
 */
INLINE void STTransform::
set_rotate(PN_stdfloat rotate) {
  _rotate = rotate;
}

/**
 * Returns the rotation component, in degrees counter-clockwise around the
 * vertical axis.
 */
INLINE PN_stdfloat STTransform::
get_rotate() const {
  return _rotate;
}

/**
 * Replaces the scale component.  Accepts a uniform scale value.
 */
INLINE void STTransform::
set_scale(PN_stdfloat scale) {
  _scale = scale;
}

/**
 * Returns the scale component, as a uniform scale value.
 */
INLINE PN_stdfloat STTransform::
get_scale() const {
  return _scale;
}

/**
 * Composes these transforms and stores the result in-place.
 */
INLINE void STTransform::
operator *= (const STTransform &other) {
  LQuaternion quat;
  quat.set_hpr(LVecBase3(_rotate, 0.0f, 0.0f));
  _pos += quat.xform(other.get_pos()) * _scale;
  _rotate += other._rotate;
  _scale *= other._scale;
}

/**
 * Composes these transforms and returns the result
 */
INLINE STTransform STTransform::
operator * (const STTransform &other) const {
  STTransform result = *this;
  result *= other;
  return result;
}
