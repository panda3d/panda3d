/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cardMaker.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE CardMaker::
CardMaker(const std::string &name) : Namable(name) {
  reset();
}

/**
 *
 */
INLINE CardMaker::
~CardMaker() {
}

/**
 * Sets the flag indicating whether vertices will be generated with UV's or
 * not.
 */
INLINE void CardMaker::
set_has_uvs(bool flag) {
  _has_uvs = flag;
}

/**
 * Sets the flag indicating whether vertices will be generated with
 * 3-component UVW's (true) or 2-component UV's (the default, false).
 * Normally, this will be implicitly set by setting the uv_range.
 */
INLINE void CardMaker::
set_has_3d_uvs(bool flag) {
  _has_3d_uvs = flag;
}

/**
 * Sets the size of the card.
 */
INLINE void CardMaker::
set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _ll_pos = LVector3::rfu(left,  0.0f, bottom);
  _lr_pos = LVector3::rfu(right, 0.0f, bottom);
  _ur_pos = LVector3::rfu(right, 0.0f, top);
  _ul_pos = LVector3::rfu(left,  0.0f, top);
}

/**
 * Sets the size of the card.
 */
INLINE void CardMaker::
set_frame(const LVecBase4 &frame) {
  set_frame(frame[0], frame[1], frame[2], frame[3]);
}

/**
 * Sets the size of the card.
 */
INLINE void CardMaker::
set_frame(const LVertex &ll, const LVertex &lr, const LVertex &ur, const LVertex &ul) {
  _ll_pos = ll;
  _lr_pos = lr;
  _ur_pos = ur;
  _ul_pos = ul;
}

/**
 * Sets the card to (-1,1,-1,1), which is appropriate if you plan to parent it
 * to render2d and use it as a fullscreen quad.
 */
INLINE void CardMaker::
set_frame_fullscreen_quad() {
  set_frame(-1.0f, 1.0f, -1.0f, 1.0f);
}

/**
 * Sets the color of the card.
 */
INLINE void CardMaker::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_color(LVecBase4(r, g, b, a));
}

/**
 * Sets the color of the card.
 */
INLINE void CardMaker::
set_color(const LVecBase4 &color) {
  _color = color;
  _has_color = true;
}

/**
 * Sets the flag indicating whether vertices will be generated with normals or
 * not.  Normals are required if you intend to enable lighting on the card,
 * but are just wasted space and bandwidth otherwise, so there is a (slight)
 * optimization for disabling them.  If enabled, the normals will be generated
 * perpendicular to the card's face.
 */
INLINE void CardMaker::
set_has_normals(bool flag) {
  _has_normals = flag;
}

/**
 * Sets a node that will be copied (and scaled and translated) to generate the
 * frame, instead of generating a new polygon.  The node may contain arbitrary
 * geometry that describes a flat polygon contained within the indicated left,
 * right, bottom, top frame.
 *
 * When generate() is called, the geometry in this node will be scaled and
 * translated appropriately to give it the size and aspect ratio specified by
 * set_frame().
 */
INLINE void CardMaker::
set_source_geometry(PandaNode *node, const LVecBase4 &frame) {
  _source_geometry = node;
  _source_frame = frame;
}

/**
 * Removes the node specified by an earlier call to set_source_geometry().
 */
INLINE void CardMaker::
clear_source_geometry() {
  _source_geometry = nullptr;
}
