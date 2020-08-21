/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file light.I
 * @author drose
 * @date 2002-03-26
 */

/**
 *
 */
INLINE Light::CData::
CData() :
  _color(1.0f, 1.0f, 1.0f, 1.0f),
  _viz_geom_stale(true)
{
}

/**
 *
 */
INLINE Light::CData::
CData(const Light::CData &copy) :
  _color(copy._color),
  _viz_geom(copy._viz_geom),
  _viz_geom_stale(copy._viz_geom_stale)
{
}

/**
 *
 */
INLINE Light::
Light() :
  _priority(0),
  _has_color_temperature(true),
  _color_temperature(6500)
{
}

/**
 *
 */
INLINE Light::
Light(const Light &copy) :
  _priority(copy._priority),
  _has_color_temperature(copy._has_color_temperature),
  _color_temperature(copy._color_temperature),
  _cycler(copy._cycler)
{
}

/**
 * Returns the basic color of the light.
 */
INLINE const LColor &Light::
get_color() const {
  CDReader cdata(_cycler);
  return cdata->_color;
}

/**
 * Sets the basic color of the light.
 */
INLINE void Light::
set_color(const LColor &color) {
  CDWriter cdata(_cycler);
  cdata->_color = color;
  _has_color_temperature = false;
  mark_viz_stale();
}

/**
 * Returns true if the color was specified as a temperature in kelvins, and
 * get_color_temperature is defined.
 *
 * @since 1.10.0
 */
INLINE bool Light::
has_color_temperature() const {
  return _has_color_temperature;
}

/**
 * Returns the basic color temperature of the light, assuming
 * has_color_temperature() returns true.
 *
 * @since 1.10.0
 */
INLINE PN_stdfloat Light::
get_color_temperature() const {
  nassertr(_has_color_temperature, _color_temperature);
  return _color_temperature;
}

/**
 * Changes the relative importance of this light relative to the other lights
 * that are applied simultaneously.
 *
 * The priority number is used to decide which of the requested lights are to
 * be selected for rendering when more lights are requested than the hardware
 * will support.  The highest-priority n lights are selected for rendering.
 *
 * This is similar to TextureStage::set_priority().
 */
INLINE void Light::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all LightAttribs in the world
  // must now re-sort their lists.
  _sort_seq++;
}

/**
 * Returns the priority associated with this light.  See set_priority().
 */
INLINE int Light::
get_priority() const {
  return _priority;
}

/**
 * Returns a global sequence number that is incremented any time any Light in
 * the world changes sort or priority.  This is used by LightAttrib to
 * determine when it is necessary to re-sort its internal array of stages.
 */
INLINE UpdateSeq Light::
get_sort_seq() {
  return _sort_seq;
}

/**
 * Indicates that the internal visualization object will need to be updated.
 */
INLINE void Light::
mark_viz_stale() {
  CDWriter cdata(_cycler);
  cdata->_viz_geom_stale = true;
}
