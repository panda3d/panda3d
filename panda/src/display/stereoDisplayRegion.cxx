// Filename: stereoDisplayRegion.cxx
// Created by:  drose (19Feb09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "stereoDisplayRegion.h"
#include "pandaNode.h"

TypeHandle StereoDisplayRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
StereoDisplayRegion::
StereoDisplayRegion(GraphicsOutput *window,
                    const LVecBase4 &dimensions,
                    DisplayRegion *left, DisplayRegion *right) :
  DisplayRegion(window, dimensions),
  _left_eye(left),
  _right_eye(right)
{
  nassertv(window == left->get_window() &&
           window == right->get_window());
  set_stereo_channel(Lens::SC_stereo);
  set_sort(0);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
StereoDisplayRegion::
~StereoDisplayRegion() {
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_clear_active
//       Access: Published, Virtual
//  Description: Sets the clear-active flag for any bitplane.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_clear_active(int n, bool clear_active) {
  // The clear_active flag gets set only on the parent, stereo display
  // region.
  DisplayRegion::set_clear_active(n, clear_active);

  // Except for non-color buffers.  These also get set on the
  // right display region by default, on the assumption that we want
  // to clear these buffers between drawing the eyes, and that the
  // right eye is the second of the pair.
  if (n != RTP_color) {
    _right_eye->set_clear_active(n, clear_active);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_clear_value
//       Access: Published, Virtual
//  Description: Sets the clear value for any bitplane.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_clear_value(int n, const LColor &clear_value) {
  DisplayRegion::set_clear_value(n, clear_value);
  _left_eye->set_clear_value(n, clear_value);
  _right_eye->set_clear_value(n, clear_value);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::disable_clears
//       Access: Published, Virtual
//  Description: Disables both the color and depth clear.  See
//               set_clear_color_active and set_clear_depth_active.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
disable_clears() {
  DisplayRegion::disable_clears();
  _left_eye->disable_clears();
  _right_eye->disable_clears();
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_pixel_zoom
//       Access: Published, Virtual
//  Description: Sets the pixel_zoom for left and right eyes.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_pixel_zoom(PN_stdfloat pixel_zoom) {
  DisplayRegion::set_pixel_zoom(pixel_zoom);
  _left_eye->set_pixel_zoom(pixel_zoom);
  _right_eye->set_pixel_zoom(pixel_zoom);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_dimensions
//       Access: Published, Virtual
//  Description: Sets both the left and right DisplayRegions to the
//               indicated dimensions.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_dimensions(const LVecBase4 &dimensions) {
  DisplayRegion::set_dimensions(dimensions);
  _left_eye->set_dimensions(dimensions);
  _right_eye->set_dimensions(dimensions);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::is_stereo
//       Access: Published, Virtual
//  Description: Returns true if this is a StereoDisplayRegion, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool StereoDisplayRegion::
is_stereo() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_camera
//       Access: Published, Virtual
//  Description: Sets both the left and right DisplayRegions to the
//               indicated camera.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_camera(const NodePath &camera) {
  DisplayRegion::set_camera(camera);
  _left_eye->set_camera(camera);
  _right_eye->set_camera(camera);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_active
//       Access: Published, Virtual
//  Description: Sets the active flag on both the left and right
//               DisplayRegions to the indicated value.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_active(bool active) {
  DisplayRegion::set_active(active);
  _left_eye->set_active(active);
  _right_eye->set_active(active);
  if (active) {
    // Reenable the appropriate eyes according to our stereo_channel
    // setting.
    set_stereo_channel(get_stereo_channel());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_sort
//       Access: Published, Virtual
//  Description: Sets the indicated sort value on the overall
//               DisplayRegion, the indicated sort value + 1 on the
//               left eye, and the indicated sort value + 2 on the
//               right eye.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_sort(int sort) {
  DisplayRegion::set_sort(sort);
  _left_eye->set_sort(sort + 1);
  _right_eye->set_sort(sort + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_stereo_channel
//       Access: Published, Virtual
//  Description: Sets the stereo channels on the left and right eyes,
//               and also sets the active flags independently on both
//               eyes.  For a StereoDisplayRegion, a different action
//               is performed for each different value:
//
//               SC_stereo - the left eye is set to SC_left, the right
//               eye to SC_right, and both eyes are activated.
//
//               SC_left - the left eye is set to SC_left and
//               activated; the right eye is deactivated.
//
//               SC_right - the right eye is set to SC_right and
//               activated; the left eye is deactivated.
//
//               SC_mono - the left eye is set to SC_mono and
//               activated; the right eye is deactivated.
//
//               This call also resets tex_view_offset to its default
//               value, which is 0 for the left eye or 1 for the right
//               eye of a stereo display region, or 0 for a mono
//               display region.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_stereo_channel(Lens::StereoChannel stereo_channel) {
  DisplayRegion::set_stereo_channel(stereo_channel);
  if (!is_active()) {
    return;
  }

  switch (stereo_channel) {
  case Lens::SC_stereo:
    _left_eye->set_stereo_channel(Lens::SC_left);
    _left_eye->set_active(true);
    _right_eye->set_stereo_channel(Lens::SC_right);
    _right_eye->set_active(true);
    break;

  case Lens::SC_left:
    _left_eye->set_stereo_channel(Lens::SC_left);
    _left_eye->set_active(true);
    _right_eye->set_active(false);
    break;

  case Lens::SC_right:
    _left_eye->set_active(false);
    _right_eye->set_stereo_channel(Lens::SC_right);
    _right_eye->set_active(true);
    break;

  case Lens::SC_mono:
    _left_eye->set_stereo_channel(Lens::SC_mono);
    _left_eye->set_active(true);
    _right_eye->set_active(false);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_tex_view_offset
//       Access: Published, Virtual
//  Description: Sets the current texture view offset for this
//               DisplayRegion.  This is normally set to zero.  If
//               nonzero, it is used to select a particular view of
//               any multiview textures that are rendered within this
//               DisplayRegion.
//
//               When you call this on a StereoDisplayRegion, it
//               automatically sets the specified value on the left
//               eye, and the specified value + 1 on the right eye.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_tex_view_offset(int tex_view_offset) {
  DisplayRegion::set_tex_view_offset(tex_view_offset);
  _left_eye->set_tex_view_offset(tex_view_offset);
  _right_eye->set_tex_view_offset(tex_view_offset + 1);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_incomplete_render
//       Access: Published, Virtual
//  Description: Sets the incomplete_render flag on both the left and
//               right DisplayRegions to the indicated value.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_incomplete_render(bool incomplete_render) {
  DisplayRegion::set_incomplete_render(incomplete_render);
  _left_eye->set_incomplete_render(incomplete_render);
  _right_eye->set_incomplete_render(incomplete_render);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_texture_reload_priority
//       Access: Published, Virtual
//  Description: Sets the texture_reload_priority on both the left and
//               right DisplayRegions to the indicated value.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_texture_reload_priority(int texture_reload_priority) {
  DisplayRegion::set_texture_reload_priority(texture_reload_priority);
  _left_eye->set_texture_reload_priority(texture_reload_priority);
  _right_eye->set_texture_reload_priority(texture_reload_priority);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_cull_traverser
//       Access: Published, Virtual
//  Description: Sets the CullTraverser for both the left and right
//               DisplayRegions.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_cull_traverser(CullTraverser *trav) {
  DisplayRegion::set_cull_traverser(trav);
  _left_eye->set_cull_traverser(trav);
  _right_eye->set_cull_traverser(trav);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::set_target_tex_page
//       Access: Published, Virtual
//  Description: Sets the page and view on both the left and
//               right DisplayRegions to the indicated value.
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
set_target_tex_page(int page) {
  DisplayRegion::set_target_tex_page(page);
  _left_eye->set_target_tex_page(page);
  _right_eye->set_target_tex_page(page);
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void StereoDisplayRegion::
output(ostream &out) const {
  out << "StereoDisplayRegion(" << *_left_eye << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: StereoDisplayRegion::make_cull_result_graph
//       Access: Published, Virtual
//  Description: Returns a special scene graph constructed to
//               represent the results of the last frame's cull
//               operation.
////////////////////////////////////////////////////////////////////
PT(PandaNode) StereoDisplayRegion::
make_cull_result_graph() {
  PT(PandaNode) root = new PandaNode("stereo");

  PT(PandaNode) left = _left_eye->make_cull_result_graph();
  left->set_name("left");
  root->add_child(left, _left_eye->get_sort());

  PT(PandaNode) right = _right_eye->make_cull_result_graph();
  right->set_name("right");
  root->add_child(right, _right_eye->get_sort());

  return root;
}
