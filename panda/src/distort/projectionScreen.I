// Filename: projectionScreen.I
// Created by:  drose (11Dec01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::set_projector
//       Access: Published
//  Description: Specifies the LensNode that is to serve as the
//               projector for this screen.  The relative position of
//               the LensNode to the ProjectionScreen, as well as the
//               properties of the lens associated with the LensNode,
//               determines the UV's that will be assigned to the
//               geometry within the ProjectionScreen.
////////////////////////////////////////////////////////////////////
INLINE void ProjectionScreen::
set_projector(LensNode *projector) {
  _projector = projector;
  _stale = true;
}


////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::get_projector
//       Access: Published
//  Description: Returns the LensNode that is to serve as the
//               projector for this screen, or NULL if no LensNode is
//               associated.
////////////////////////////////////////////////////////////////////
INLINE LensNode *ProjectionScreen::
get_projector() const {
  return _projector;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::set_vignette_on
//       Access: Published
//  Description: Specifies whether vertex-based vignetting should be
//               on.  When this is enabled, vertex color will be set
//               on the screen vertices to color the screen two
//               distinct colors, usually white and black, for the
//               parts of the screen in front of and outside the lens'
//               frustum, respectively.  When this is not enabled, the
//               screen color will be left alone.
//
//               This effect generally looks terrible, but it does
//               at least make the boundaries of the lens clear.
////////////////////////////////////////////////////////////////////
INLINE void ProjectionScreen::
set_vignette_on(bool vignette_on) {
  _vignette_on = vignette_on;
  _stale = true;
}


////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::get_vignette_on
//       Access: Published
//  Description: Returns true if vertex-based vignetting is on, false
//               otherwise.  See set_vignette_on().
////////////////////////////////////////////////////////////////////
INLINE bool ProjectionScreen::
get_vignette_on() const {
  return _vignette_on;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::set_vignette_color
//       Access: Published
//  Description: Specifies the color the screen will be painted at the
//               portions outside of the lens' frustum; i.e. where the
//               lens can't see it or illuminate it.  This color is
//               only used if the vignette_on flag is true; see
//               set_vignette_on().
////////////////////////////////////////////////////////////////////
INLINE void ProjectionScreen::
set_vignette_color(const Colorf &vignette_color) {
  _vignette_color = vignette_color;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::get_vignette_color
//       Access: Published
//  Description: Returns the color the screen will be painted at the
//               portions outside of the lens' frustum.  See
//               set_vignette_color().
////////////////////////////////////////////////////////////////////
INLINE const Colorf &ProjectionScreen::
get_vignette_color() const {
  return _vignette_color;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::set_frame_color
//       Access: Published
//  Description: Specifies the color the screen will be painted at the
//               portions outside of the lens' frustum; i.e. where the
//               lens can't see it or illuminate it.  This color is
//               only used if the vignette_on flag is true; see
//               set_vignette_on().
////////////////////////////////////////////////////////////////////
INLINE void ProjectionScreen::
set_frame_color(const Colorf &frame_color) {
  _frame_color = frame_color;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::get_frame_color
//       Access: Published
//  Description: Returns the color the screen will be painted at the
//               portions outside of the lens' frustum.  See
//               set_frame_color().
////////////////////////////////////////////////////////////////////
INLINE const Colorf &ProjectionScreen::
get_frame_color() const {
  return _frame_color;
}
