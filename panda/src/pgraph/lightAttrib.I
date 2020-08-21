/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightAttrib.I
 * @author drose
 * @date 2002-03-26
 */

/**
 * Use LightAttrib::make() to construct a new LightAttrib object.
 */
INLINE LightAttrib::
LightAttrib() : _off_all_lights(false), _num_non_ambient_lights(0) {
}

/**
 * Returns the number of lights that are turned on by the attribute.
 */
INLINE size_t LightAttrib::
get_num_on_lights() const {
  check_sorted();
  return _sorted_on_lights.size();
}

/**
 * Returns the number of non-ambient lights that are turned on by this
 * attribute.
 */
INLINE size_t LightAttrib::
get_num_non_ambient_lights() const {
  check_sorted();
  return _num_non_ambient_lights;
}

/**
 * Returns the nth light turned on by the attribute, sorted in render order.
 */
INLINE NodePath LightAttrib::
get_on_light(size_t n) const {
  nassertr(n < _sorted_on_lights.size(), NodePath::fail());
  return _sorted_on_lights[n];
}

/**
 * Returns true if the indicated light is turned on by the attrib, false
 * otherwise.
 */
INLINE bool LightAttrib::
has_on_light(const NodePath &light) const {
  return _on_lights.find(light) != _on_lights.end();
}

/**
 * Returns true if any light is turned on by the attrib, false otherwise.
 */
INLINE bool LightAttrib::
has_any_on_light() const {
  return !_on_lights.empty();
}

/**
 * Returns the number of lights that are turned off by the attribute.
 */
INLINE size_t LightAttrib::
get_num_off_lights() const {
  return _off_lights.size();
}

/**
 * Returns the nth light turned off by the attribute, sorted in arbitrary
 * (pointer) order.
 */
INLINE NodePath LightAttrib::
get_off_light(size_t n) const {
  nassertr(n < _off_lights.size(), NodePath::fail());
  return _off_lights[n];
}

/**
 * Returns true if the indicated light is turned off by the attrib, false
 * otherwise.
 */
INLINE bool LightAttrib::
has_off_light(const NodePath &light) const {
  return _off_lights.find(light) != _off_lights.end() ||
    (_off_all_lights && !has_on_light(light));
}

/**
 * Returns true if this attrib turns off all lights (although it may also turn
 * some on).
 */
INLINE bool LightAttrib::
has_all_off() const {
  return _off_all_lights;
}

/**
 * Returns true if this is an identity attrib: it does not change the set of
 * lights in use.
 */
INLINE bool LightAttrib::
is_identity() const {
  return _on_lights.empty() && _off_lights.empty() && !_off_all_lights;
}

/**
 * Makes sure that the on lights are still sorted by priority.  It may become
 * invalid if someone calls Light::set_priority().
 */
INLINE void LightAttrib::
check_sorted() const {
  if (_sort_seq != Light::get_sort_seq()) {
    ((LightAttrib *)this)->sort_on_lights();
  }
}
