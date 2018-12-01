/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file planeNode.I
 * @author drose
 * @date 2002-07-11
 */

/**
 *
 */
INLINE PlaneNode::CData::
CData() :
  _viz_scale(100.0f)
{
  // The default plane (perpendicular to the Z-axis) is used until another one
  // is specified explicitly.
}

/**
 *
 */
INLINE PlaneNode::CData::
CData(const PlaneNode::CData &copy) :
  _plane(copy._plane),
  _front_viz(copy._front_viz),
  _back_viz(copy._back_viz),
  _viz_scale(copy._viz_scale)
{
}

/**
 * Sets the particular plane represented by the PlaneNode.
 */
INLINE void PlaneNode::
set_plane(const LPlane &plane) {
  CDWriter cdata(_cycler);
  if (cdata->_plane != plane) {
    cdata->_plane = plane;
    cdata->_front_viz = nullptr;
    cdata->_back_viz = nullptr;
  }
}

/**
 * Returns the plane represented by the PlaneNode.
 */
INLINE const LPlane &PlaneNode::
get_plane() const {
  CDReader cdata(_cycler);
  return cdata->_plane;
}

/**
 * Specifies the size of the visual representation of the plane that is drawn
 * if the PlaneNode is shown.
 */
INLINE void PlaneNode::
set_viz_scale(PN_stdfloat viz_scale) {
  CDWriter cdata(_cycler);
  if (cdata->_viz_scale != viz_scale) {
    cdata->_viz_scale = viz_scale;
    cdata->_front_viz = nullptr;
    cdata->_back_viz = nullptr;
  }
}

/**
 * Returns the size of the visual representation of the plane that is drawn if
 * the PlaneNode is shown.
 */
INLINE PN_stdfloat PlaneNode::
get_viz_scale() const {
  CDReader cdata(_cycler);
  return cdata->_viz_scale;
}

/**
 * Changes the relative importance of this PlaneNode (when it is used as a
 * clip plane) relative to the other clip planes that are applied
 * simultaneously.
 *
 * The priority number is used to decide which of the requested clip planes
 * are to be activated when more clip planes are requested than the hardware
 * will support.  The highest-priority n planes are selected for rendering.
 *
 * This is similar to TextureStage::set_priority().
 */
INLINE void PlaneNode::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all ClipPlaneAttribs in the world
  // must now re-sort their lists.
  _sort_seq++;
}

/**
 * Returns the priority associated with this clip plane.  See set_priority().
 */
INLINE int PlaneNode::
get_priority() const {
  return _priority;
}

/**
 * Specifies the sort of things this plane will actually clip (when it is used
 * as a clip plane).  This is a bitmask union of ClipEffect values.  If it
 * includes CE_visible, then it will clip visible geometry; if it includes
 * CE_collision, then it will clip collision polygons.  If it includes neither
 * bit, it will still affect culling, but objects will either be wholly behind
 * the clipping plane, or wholly present.
 */
INLINE void PlaneNode::
set_clip_effect(int clip_effect) {
  _clip_effect = clip_effect;
}

/**
 * Returns the clip_effect bits for this clip plane.  See set_clip_effect().
 */
INLINE int PlaneNode::
get_clip_effect() const {
  return _clip_effect;
}

/**
 * Returns a global sequence number that is incremented any time any PlaneNode
 * in the world changes sort or priority.  This is used by ClipPlaneAttrib to
 * determine when it is necessary to re-sort its internal array of stages.
 */
INLINE UpdateSeq PlaneNode::
get_sort_seq() {
  return _sort_seq;
}
