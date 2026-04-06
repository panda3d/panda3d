/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexUV.I
 * @author drose
 * @date 2004-07-20
 */

/**
 * Returns the actual name that should be set for a given name string.
 * Usually this is the same string that is input, but for historical reasons
 * the texture coordinate name "default" is mapped to the empty string.
 */
INLINE std::string EggVertexUV::
filter_name(const std::string &name) {
  if (name == "default") {
    return std::string();
  }
  return name;
}

/**
 *
 */
INLINE void EggVertexUV::
set_name(const std::string &name) {
  Namable::set_name(filter_name(name));
}

/**
 * Returns the number of components of the texture coordinate set.  This is
 * either 2 (the normal case) or 3 (for a 3-d texture coordinate).
 */
INLINE int EggVertexUV::
get_num_dimensions() const {
  return has_w() ? 3 : 2;
}

/**
 * Returns true if the texture coordinate has a third, w component, false if
 * it is just a normal 2-d texture coordinate.
 */
INLINE bool EggVertexUV::
has_w() const {
  return (_flags & F_has_w) != 0;
}

/**
 * Returns the texture coordinate pair, if get_num_dimensions() is 2.
 */
INLINE LTexCoordd EggVertexUV::
get_uv() const {
  nassertr(!has_w(), LTexCoordd::zero());
  return LTexCoordd(_uvw[0], _uvw[1]);
}

/**
 * Returns the texture coordinate triple, if get_num_dimensions() is 3.  This
 * is also legal to call if get_num_dimensions() is 2 (but the last dimension
 * will be zero).
 */
INLINE const LTexCoord3d &EggVertexUV::
get_uvw() const {
  return _uvw;
}

/**
 * Sets the texture coordinate pair.  This makes the texture coordinate a 2-d
 * texture coordinate, which is the usual case.
 */
INLINE void EggVertexUV::
set_uv(const LTexCoordd &uv) {
  _uvw.set(uv[0], uv[1], 0.0);
  _flags &= ~F_has_w;
}

/**
 * Sets the texture coordinate triple.  This makes the texture coordinate a
 * 3-d texture coordinate.
 */
INLINE void EggVertexUV::
set_uvw(const LTexCoord3d &uvw) {
  _uvw = uvw;
  _flags |= F_has_w;
}

/**
 *
 */
INLINE bool EggVertexUV::
has_tangent() const {
  return (_flags & F_has_tangent) != 0;
}

/**
 *
 */
INLINE bool EggVertexUV::
has_tangent4() const {
  return (_flags & F_has_tangent4) != 0;
}

/**
 *
 */
INLINE const LNormald &EggVertexUV::
get_tangent() const {
  nassertr(has_tangent(), _tangent);
  return _tangent;
}

/**
 *
 */
INLINE LVecBase4d EggVertexUV::
get_tangent4() const {
  LVecBase4d tangent4(_tangent, 1.0);
  nassertr_always(has_tangent(), tangent4);
  if (_flags & F_flip_computed_binormal) {
    tangent4[3] = -1.0;
  }
  return tangent4;
}

/**
 *
 */
INLINE void EggVertexUV::
set_tangent(const LNormald &tangent) {
  _tangent = tangent;
  _flags |= F_has_tangent;
  _flags &= ~(F_has_tangent4 | F_flip_computed_binormal);
}

/**
 * Sets the tangent vector, along with a fourth parameter that is multiplied
 * with the result of cross(normal, tangent) when computing the binormal.
 */
INLINE void EggVertexUV::
set_tangent4(const LVecBase4d &tangent) {
  _tangent = tangent.get_xyz();
  _flags |= F_has_tangent4 | F_has_tangent;
  if (tangent[3] < 0.0) {
    _flags |= F_flip_computed_binormal;
  } else {
    _flags &= ~F_flip_computed_binormal;
  }
}

/**
 *
 */
INLINE void EggVertexUV::
clear_tangent() {
  _flags &= ~F_has_tangent;
}

/**
 *
 */
INLINE bool EggVertexUV::
has_binormal() const {
  return (_flags & F_has_binormal) != 0;
}

/**
 *
 */
INLINE const LNormald &EggVertexUV::
get_binormal() const {
  nassertr(has_binormal(), _binormal);
  return _binormal;
}

/**
 *
 */
INLINE void EggVertexUV::
set_binormal(const LNormald &binormal) {
  _binormal = binormal;
  _flags |= F_has_binormal;
}

/**
 *
 */
INLINE void EggVertexUV::
clear_binormal() {
  _flags &= ~F_has_binormal;
}
