/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMorph.I
 * @author drose
 * @date 1999-01-29
 */

/**
 *
 */
template<class Parameter>
INLINE EggMorph<Parameter>::
EggMorph(const std::string &name, const Parameter &offset)
  : Namable(name), _offset(offset) {
}



/**
 *
 */
template<class Parameter>
INLINE void EggMorph<Parameter>::
set_offset(const Parameter &offset) {
  _offset = offset;
}


/**
 *
 */
template<class Parameter>
INLINE const Parameter &EggMorph<Parameter>::
get_offset() const {
  return _offset;
}


/**
 *
 */
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator < (const EggMorph<Parameter> &other) const {
  return get_name() < other.get_name();
}


/**
 *
 */
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator == (const EggMorph<Parameter> &other) const {
  return get_name() == other.get_name();
}

/**
 *
 */
template<class Parameter>
INLINE bool EggMorph<Parameter>::
operator != (const EggMorph<Parameter> &other) const {
  return !operator == (other);
}

/**
 * compare_to() compares a different space than the operator methods, which
 * only check the name.  compare_to() compares the name and the value as well.
 */
template<class Parameter>
INLINE int EggMorph<Parameter>::
compare_to(const EggMorph<Parameter> &other, double threshold) const {
  int compare = strcmp(get_name().c_str(), other.get_name().c_str());
  if (compare != 0) {
    return compare;
  }
  return _offset.compare_to(other._offset, threshold);
}

/**
 *
 */
template<class Parameter>
INLINE void EggMorph<Parameter>::
output(std::ostream &out, const std::string &tag, int num_dimensions) const {
  out << tag << " " << get_name() << " {";
  for (int i = 0; i < num_dimensions; ++i) {
    out << " " << MAYBE_ZERO(_offset[i]);
  }
  out << " }";
}
