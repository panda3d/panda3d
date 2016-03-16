/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggAttributes.I
 * @author drose
 * @date 1999-01-16
 */

/**
 *
 */
INLINE bool EggAttributes::
has_normal() const {
  return (_flags & F_has_normal) != 0;
}

/**
 *
 */
INLINE const LNormald &EggAttributes::
get_normal() const {
  nassertr(has_normal(), _normal);
  return _normal;
}

/**
 *
 */
INLINE void EggAttributes::
set_normal(const LNormald &normal) {
  _normal = normal;
  _flags |= F_has_normal;
}

/**
 *
 */
INLINE void EggAttributes::
clear_normal() {
  _flags &= ~F_has_normal;
}

/**
 * Returns true if this normal matches that of the other EggAttributes object,
 * include the morph list.
 */
INLINE bool EggAttributes::
matches_normal(const EggAttributes &other) const {
  if (((_flags ^ other._flags) & F_has_normal) != 0) {
    return false;
  }
  if (!has_normal()) {
    return true;
  }
  return (get_normal() == other.get_normal() &&
          _dnormals.compare_to(other._dnormals, egg_parameters->_normal_threshold) == 0);
}

/**
 * Sets this normal to be the same as the other's, include morphs.  If the
 * other has no normal, this clears the normal.
 */
INLINE void EggAttributes::
copy_normal(const EggAttributes &other) {
  if (!other.has_normal()) {
    clear_normal();
  } else {
    set_normal(other.get_normal());
    _dnormals = other._dnormals;
  }
}

/**
 *
 */
INLINE bool EggAttributes::
has_color() const {
  return (_flags & F_has_color) != 0;
}

/**
 * Returns the color set on this particular attribute.  If there is no color
 * set, returns white.
 */
INLINE LColor EggAttributes::
get_color() const {
  if (has_color()) {
    return _color;
  } else {
    return LColor(1.0, 1.0, 1.0, 1.0);
  }
}

/**
 *
 */
INLINE void EggAttributes::
set_color(const LColor &color) {
  _color = color;
  _flags |= F_has_color;
}

/**
 *
 */
INLINE void EggAttributes::
clear_color() {
  _flags &= ~F_has_color;
}

/**
 * Returns true if this color matches that of the other EggAttributes object,
 * include the morph list.
 */
INLINE bool EggAttributes::
matches_color(const EggAttributes &other) const {
  if (((_flags ^ other._flags) & F_has_color) != 0) {
    return false;
  }
  if (!has_color()) {
    return true;
  }
  return (get_color() == other.get_color() &&
          _drgbas.compare_to(other._drgbas, egg_parameters->_color_threshold) == 0);
}

/**
 * Sets this color to be the same as the other's, include morphs.  If the
 * other has no color, this clears the color.
 */
INLINE void EggAttributes::
copy_color(const EggAttributes &other) {
  if (!other.has_color()) {
    clear_color();
  } else {
    set_color(other.get_color());
    _drgbas = other._drgbas;
  }
}

/**
 * An ordering operator to compare two vertices for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique vertices.
 */
INLINE bool EggAttributes::
sorts_less_than(const EggAttributes &other) const {
  return compare_to(other) < 0;
}
