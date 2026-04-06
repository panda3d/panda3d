/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lodNode.I
 * @author drose
 * @date 2002-03-06
 */

/**
 *
 */
INLINE LODNode::
LODNode(const std::string &name) :
  PandaNode(name)
{
  set_cull_callback();
}

/**
 *
 */
INLINE LODNode::
LODNode(const LODNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Adds a switch range to the LODNode.  This implies that the corresponding
 * child node has been parented to the node.
 *
 * The sense of in vs.  out distances is as if the object were coming towards
 * you from far away: it switches "in" at the far distance, and switches "out"
 * at the close distance.  Thus, "in" should be larger than "out".
 */
INLINE void LODNode::
add_switch(PN_stdfloat in, PN_stdfloat out) {
  nassertv(in >= out);

  CDWriter cdata(_cycler);
  cdata->_switch_vector.push_back(Switch(in, out));
  cdata->check_limits();

  if (cdata->_num_shown != 0) {
    mark_internal_bounds_stale();
  }
}

/**
 * Changes the switching range of a particular child of the LODNode.  See
 * add_switch().
 */
INLINE bool LODNode::
set_switch(int index, PN_stdfloat in, PN_stdfloat out) {
  nassertr(in >= out, false);

  CDWriter cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), false);
  cdata->_switch_vector[index].set_range(in, out);
  cdata->check_limits();

  if (cdata->_num_shown != 0) {
    mark_internal_bounds_stale();
  }

  return true;
}

/**
 * Removes the set of switching ranges for the LODNode, presumably in
 * conjunction with removing all of its children.  See add_switch().
 */
INLINE void LODNode::
clear_switches() {
  CDWriter cdata(_cycler);
  cdata->_switch_vector.clear();
  cdata->_lowest = 0;
  cdata->_highest = 0;

  if (cdata->_num_shown != 0) {
    mark_internal_bounds_stale();
  }
}

/**
 * Returns the number of switch ranges added to the LODNode.  This should
 * correspond to the number of children of the node in order for the LODNode
 * to function correctly.
 */
INLINE int LODNode::
get_num_switches() const {
  CDReader cdata(_cycler);
  return cdata->_switch_vector.size();
}

/**
 * Returns the multiplier for lod distances
 */
INLINE PN_stdfloat LODNode::
get_lod_scale() const {
  CDReader cdata(_cycler);
  return cdata->_lod_scale;
}

/**
 * Sets the multiplier for lod distances.  A higher value means you'll see
 * farther switchs than normal
 */
INLINE void LODNode::
set_lod_scale(PN_stdfloat value) {
  CDWriter cdata(_cycler);
  cdata->_lod_scale = value;
}


/**
 * Returns the "in" distance of the indicated switch range.  This should be
 * larger than the "out" distance of the same range.
 */
INLINE PN_stdfloat LODNode::
get_in(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), 0.0);
  return cdata->_switch_vector[index].get_in();
}

/**
 * Returns the "out" distance of the indicated switch range.  This should be
 * smaller than the "in" distance of the same range.
 */
INLINE PN_stdfloat LODNode::
get_out(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), 0.0);
  return cdata->_switch_vector[index].get_out();
}

/**
 * Returns the index number of the child with the lowest level of detail; that
 * is, the one that is designed to be seen from the farthest away.  This is
 * usually the first child, but it is not necessarily so.
 */
INLINE int LODNode::
get_lowest_switch() const {
  CDReader cdata(_cycler);
  return cdata->_lowest;
}

/**
 * Returns the index number of the child with the highest level of detail;
 * that is, the one that is designed to be seen from the closest to the
 * camera.  This is usually the last child, but it is not necessarily so.
 */
INLINE int LODNode::
get_highest_switch() const {
  CDReader cdata(_cycler);
  return cdata->_highest;
}

/**
 * Forces the LODNode to show the indicated level instead of the level that
 * would normally be shown based on the distance from the camera.
 */
INLINE void LODNode::
force_switch(int index) {
  CDWriter cdata(_cycler);
  cdata->_force_switch = index;
  cdata->_got_force_switch = true;
}

/**
 * Undoes the effect of a previous call to force_switch() and releases the
 * LODNode to once again display the normal level.
 */
INLINE void LODNode::
clear_force_switch() {
  CDWriter cdata(_cycler);
  cdata->_got_force_switch = false;
}

/**
 * Specifies the center of the LOD.  This is the point that is compared to the
 * camera (in camera space) to determine the particular LOD that should be
 * chosen.
 */
INLINE void LODNode::
set_center(const LPoint3 &center) {
  CDWriter cdata(_cycler);
  cdata->_center = center;

  if (cdata->_num_shown != 0) {
    mark_internal_bounds_stale();
  }
}

/**
 * Returns the center of the LOD.  This is the point that is compared to the
 * camera (in camera space) to determine the particular LOD that should be
 * chosen.
 */
INLINE const LPoint3 &LODNode::
get_center() const {
  CDReader cdata(_cycler);
  return cdata->_center;
}

/**
 * Returns true if any switch has been shown with show_switch(), indicating
 * the LODNode is in debug show mode; or false if it is in the normal mode.
 */
INLINE bool LODNode::
is_any_shown() const {
  CDReader cdata(_cycler);
  return (cdata->_num_shown != 0);
}

/**
 * To be called internally when the node is rendered, this will raise an
 * assertion if verify-lods is configured true, and verify_child_bounds()
 * returns false.
 */
INLINE void LODNode::
consider_verify_lods(CullTraverser *trav, CullTraverserData &data) {
#ifndef NDEBUG
  if (verify_lods) {
    do_auto_verify_lods(trav, data);
  }
#endif  // NDEBUG
}

/**
 *
 */
INLINE LODNode::CData::
CData() :
  _center(0.0f, 0.0f, 0.0f),
  _lowest(0),
  _highest(0),
  _got_force_switch(false),
  _force_switch(0),
  _num_shown(0),
  _lod_scale(1)
{
}

/**
 *
 */
INLINE LODNode::CData::
CData(const LODNode::CData &copy) :
  _center(copy._center),
  _switch_vector(copy._switch_vector),
  _lowest(copy._lowest),
  _highest(copy._highest),
  _bounds_seq(UpdateSeq::old()),
  _got_force_switch(copy._got_force_switch),
  _force_switch(copy._force_switch),
  _num_shown(copy._num_shown),
  _lod_scale(copy._lod_scale)
{
}

/**
 *
 */
INLINE LODNode::Switch::
Switch(PN_stdfloat in, PN_stdfloat out) :
  _shown(false),
  _bounds_seq(UpdateSeq::old()),
  _verify_ok(false)
{
  set_range(in, out);
}

/**
 *
 */
INLINE PN_stdfloat LODNode::Switch::
get_in() const {
  return _in;
}

/**
 *
 */
INLINE PN_stdfloat LODNode::Switch::
get_out() const {
  return _out;
}

/**
 *
 */
INLINE void LODNode::Switch::
set_range(PN_stdfloat in, PN_stdfloat out) {
  _in = in;
  _out = out;
  clear_ring_viz();
}

/**
 * Returns true if the indicated distance is within the range for the LOD.
 */
INLINE bool LODNode::Switch::
in_range(PN_stdfloat dist) const {
  return (dist >= _out && dist < _in);
}

/**
 * Returns true if the indicated distance squared is within the range for the
 * LOD.  (The distance value is understood to be the square of the distance
 * from the camera to the object.)
 */
INLINE bool LODNode::Switch::
in_range_2(PN_stdfloat dist2) const {
  return (dist2 >= _out * _out && dist2 < _in * _in);
}

/**
 * Scales the switching distances by the indicated factor.
 */
INLINE void LODNode::Switch::
rescale(PN_stdfloat factor) {
  _in *= factor;
  _out *= factor;
  clear_ring_viz();
}

/**
 * Returns true if show() has been called.
 */
INLINE bool LODNode::Switch::
is_shown() const {
  return _shown;
}

/**
 * Shows this ring in debug mode using the indicated color.
 */
INLINE void LODNode::Switch::
show(const LColor &color) {
  _shown = true;
  _show_color = color;
}

/**
 * Undoes a previous call to show().
 */
INLINE void LODNode::Switch::
hide() {
  _shown = false;
}

/**
 * Returns a PandaNode suitable for rendering the ring associated with this
 * switch.
 */
INLINE PandaNode *LODNode::Switch::
get_ring_viz() const {
  if (_ring_viz.is_null()) {
    ((Switch *)this)->compute_ring_viz();
  }

  return _ring_viz;
}

/**
 * Returns a PandaNode suitable for rendering the center spindle of the
 * LODNode, in the color of this switch.
 */
INLINE PandaNode *LODNode::Switch::
get_spindle_viz() const {
  if (_spindle_viz.is_null()) {
    ((Switch *)this)->compute_spindle_viz();
  }

  return _spindle_viz;
}

/**
 * Returns a RenderState suitable for drawing the visible children of this
 * switch level when the show_switch() debugging mode is enabled.
 */
INLINE const RenderState *LODNode::Switch::
get_viz_model_state() const {
  if (_viz_model_state.is_null()) {
    ((Switch *)this)->compute_viz_model_state();
  }

  return _viz_model_state;
}

/**
 * Writes the contents of the Switch out to the datagram, presumably in
 * preparation to writing to a Bam file.
 */
INLINE void LODNode::Switch::
write_datagram(Datagram &destination) const {
  destination.add_stdfloat(_in);
  destination.add_stdfloat(_out);
}

/**
 * Reads the contents of the Switch from the datagram, presumably in response
 * to reading a Bam file.
 */
INLINE void LODNode::Switch::
read_datagram(DatagramIterator &source) {
  _in = source.get_stdfloat();
  _out = source.get_stdfloat();
}

/**
 * Resets the internal cache values for the ring and spindle viz, and related
 * pointers, for the set_switch() debugging mode.
 */
INLINE void LODNode::Switch::
clear_ring_viz() {
  _ring_viz.clear();
  _spindle_viz.clear();
  _viz_model_state.clear();
  _bounds_seq = UpdateSeq::old();
}
