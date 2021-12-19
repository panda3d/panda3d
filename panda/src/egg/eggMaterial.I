/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.I
 * @author drose
 * @date 1999-01-29
 */

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
set_base(const LColor &base) {
  _base = base;
  _flags |= F_base;
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
clear_base() {
  _flags &= ~F_base;
}

/**
 * @since 1.10.0
 */
INLINE bool EggMaterial::
has_base() const {
  return (_flags & F_base) != 0;
}

/**
 * It is legal to call this even if has_base() returns false.  If so, it
 * simply returns the default base color.
 *
 * @since 1.10.0
 */
INLINE LColor EggMaterial::
get_base() const {
  if (has_base()) {
    return _base;
  } else {
    return LColor(1.0, 1.0, 1.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_diff(const LColor &diff) {
  _diff = diff;
  _flags |= F_diff;
}

/**
 *
 */
INLINE void EggMaterial::
clear_diff() {
  _flags &= ~F_diff;
}

/**
 *
 */
INLINE bool EggMaterial::
has_diff() const {
  return (_flags & F_diff) != 0;
}

/**
 * It is legal to call this even if has_diff() returns false.  If so, it
 * simply returns the default diff color.
 */
INLINE LColor EggMaterial::
get_diff() const {
  if (has_diff()) {
    return _diff;
  } else {
    return LColor(1.0, 1.0, 1.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_amb(const LColor &amb) {
  _amb = amb;
  _flags |= F_amb;
}

/**
 *
 */
INLINE void EggMaterial::
clear_amb() {
  _flags &= ~F_amb;
}

/**
 *
 */
INLINE bool EggMaterial::
has_amb() const {
  return (_flags & F_amb) != 0;
}

/**
 * It is legal to call this even if has_amb() returns false.  If so, it simply
 * returns the default amb color.
 */
INLINE LColor EggMaterial::
get_amb() const {
  if (has_amb()) {
    return _amb;
  } else {
    return LColor(1.0, 1.0, 1.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_emit(const LColor &emit) {
  _emit = emit;
  _flags |= F_emit;
}

/**
 *
 */
INLINE void EggMaterial::
clear_emit() {
  _flags &= ~F_emit;
}

/**
 *
 */
INLINE bool EggMaterial::
has_emit() const {
  return (_flags & F_emit) != 0;
}

/**
 * It is legal to call this even if has_emit() returns false.  If so, it
 * simply returns the default emit color.
 */
INLINE LColor EggMaterial::
get_emit() const {
  if (has_emit()) {
    return _emit;
  } else {
    return LColor(0.0, 0.0, 0.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_spec(const LColor &spec) {
  _spec = spec;
  _flags |= F_spec;
}

/**
 *
 */
INLINE void EggMaterial::
clear_spec() {
  _flags &= ~F_spec;
}

/**
 *
 */
INLINE bool EggMaterial::
has_spec() const {
  return (_flags & F_spec) != 0;
}

/**
 * It is legal to call this even if has_spec() returns false.  If so, it
 * simply returns the default spec color.
 */
INLINE LColor EggMaterial::
get_spec() const {
  if (has_spec()) {
    return _spec;
  } else {
    return LColor(0.0, 0.0, 0.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_shininess(double shininess) {
  _shininess = shininess;
  _flags |= F_shininess;
}

/**
 *
 */
INLINE void EggMaterial::
clear_shininess() {
  _flags &= ~F_shininess;
}

/**
 *
 */
INLINE bool EggMaterial::
has_shininess() const {
  return (_flags & F_shininess) != 0;
}

/**
 *
 */
INLINE double EggMaterial::
get_shininess() const {
  if (has_shininess()) {
    return _shininess;
  } else {
    return 0.0;
  }
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
set_roughness(double roughness) {
  _roughness = roughness;
  _flags |= F_roughness;
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
clear_roughness() {
  _flags &= ~F_roughness;
}

/**
 * @since 1.10.0
 */
INLINE bool EggMaterial::
has_roughness() const {
  return (_flags & F_roughness) != 0;
}

/**
 * @since 1.10.0
 */
INLINE double EggMaterial::
get_roughness() const {
  if (has_roughness()) {
    return _roughness;
  } else {
    return 1.0;
  }
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
set_metallic(double metallic) {
  _metallic = metallic;
  _flags |= F_metallic;
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
clear_metallic() {
  _flags &= ~F_metallic;
}

/**
 * @since 1.10.0
 */
INLINE bool EggMaterial::
has_metallic() const {
  return (_flags & F_metallic) != 0;
}

/**
 * @since 1.10.0
 */
INLINE double EggMaterial::
get_metallic() const {
  if (has_metallic()) {
    return _metallic;
  } else {
    return 0.0;
  }
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
set_ior(double ior) {
  _ior = ior;
  _flags |= F_ior;
}

/**
 * @since 1.10.0
 */
INLINE void EggMaterial::
clear_ior() {
  _flags &= ~F_ior;
}

/**
 * @since 1.10.0
 */
INLINE bool EggMaterial::
has_ior() const {
  return (_flags & F_ior) != 0;
}

/**
 * @since 1.10.0
 */
INLINE double EggMaterial::
get_ior() const {
  if (has_ior()) {
    return _ior;
  } else {
    return 1.0;
  }
}

/**
 *
 */
INLINE void EggMaterial::
set_local(bool local) {
  _local = local;
  _flags |= F_local;
}

/**
 *
 */
INLINE void EggMaterial::
clear_local() {
  _flags &= ~F_local;
}

/**
 *
 */
INLINE bool EggMaterial::
has_local() const {
  return (_flags & F_local) != 0;
}

/**
 *
 */
INLINE bool EggMaterial::
get_local() const {
  if (has_local()) {
    return _local;
  } else {
    return 0.0;
  }
}

/**
 *
 */
INLINE UniqueEggMaterials::
UniqueEggMaterials(int eq) : _eq(eq) {
}

/**
 *
 */
INLINE bool UniqueEggMaterials::
operator ()(const EggMaterial *t1, const EggMaterial *t2) const {
  return t1->sorts_less_than(*t2, _eq);
}
