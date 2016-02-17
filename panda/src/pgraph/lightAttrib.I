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
LightAttrib() {
  _off_all_lights = false;
}

/**
 * Use LightAttrib::make() to construct a new LightAttrib object.  The copy
 * constructor is only defined to facilitate methods like add_on_light().
 */
INLINE LightAttrib::
LightAttrib(const LightAttrib &copy) :
  _on_lights(copy._on_lights),
  _off_lights(copy._off_lights),
  _off_all_lights(copy._off_all_lights)
{
}

/**
 * Returns the number of lights that are turned on by the attribute.
 */
INLINE int LightAttrib::
get_num_on_lights() const {
  return _on_lights.size();
}

/**
 * Returns the nth light turned on by the attribute, sorted in render order.
 */
INLINE NodePath LightAttrib::
get_on_light(int n) const {
  nassertr(n >= 0 && n < (int)_on_lights.size(), NodePath::fail());
  return _on_lights[n];
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
 * Returns the number of lights that are turned off by the attribute.
 */
INLINE int LightAttrib::
get_num_off_lights() const {
  return _off_lights.size();
}

/**
 * Returns the nth light turned off by the attribute, sorted in arbitrary
 * (pointer) order.
 */
INLINE NodePath LightAttrib::
get_off_light(int n) const {
  nassertr(n >= 0 && n < (int)_off_lights.size(), NodePath::fail());
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
 * Confirms whether the _filtered table is still valid.  It may become invalid
 * if someone calls Light::set_priority().
 *
 * If the table is invalid, transparently empties it before returning.
 */
INLINE void LightAttrib::
check_filtered() const {
  if (_sort_seq != Light::get_sort_seq()) {
    ((LightAttrib *)this)->sort_on_lights();
  }
}
