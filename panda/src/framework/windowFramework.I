// Filename: windowFramework.I
// Created by:  drose (02Apr02)
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
//     Function: WindowFramework::get_panda_framework
//       Access: Public
//  Description: Returns a pointer to the associated PandaFramework
//               object.
////////////////////////////////////////////////////////////////////
INLINE PandaFramework *WindowFramework::
get_panda_framework() const {
  return _panda_framework;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_graphics_window
//       Access: Public
//  Description: Returns a pointer to the underlying GraphicsWindow
//               object.
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindow *WindowFramework::
get_graphics_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_num_cameras
//       Access: Public
//  Description: Returns the number of 3-d cameras associated with the
//               window.  A window usually has only one camera, but it
//               may have multiple cameras if there are multiple
//               display regions within the window.
////////////////////////////////////////////////////////////////////
INLINE int WindowFramework::
get_num_cameras() const {
  return _cameras.size();
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_camera
//       Access: Public
//  Description: Returns the nth camera associated with the window.
////////////////////////////////////////////////////////////////////
INLINE Camera *WindowFramework::
get_camera(int n) const {
  nassertr(n >= 0 && n < (int)_cameras.size(), NULL);
  return _cameras[n];
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_wireframe
//       Access: Public
//  Description: Returns the current state of the wireframe flag.
////////////////////////////////////////////////////////////////////
INLINE bool WindowFramework::
get_wireframe() const {
  return _wireframe_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_texture
//       Access: Public
//  Description: Returns the current state of the texture flag.
////////////////////////////////////////////////////////////////////
INLINE bool WindowFramework::
get_texture() const {
  return _texture_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_two_sided
//       Access: Public
//  Description: Returns the current state of the two_sided flag.
////////////////////////////////////////////////////////////////////
INLINE bool WindowFramework::
get_two_sided() const {
  return _two_sided_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_one_sided_reverse
//       Access: Public
//  Description: Returns the current state of the one_sided_reverse flag.
////////////////////////////////////////////////////////////////////
INLINE bool WindowFramework::
get_one_sided_reverse() const {
  return _one_sided_reverse_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_lighting
//       Access: Public
//  Description: Returns the current state of the lighting flag.
////////////////////////////////////////////////////////////////////
INLINE bool WindowFramework::
get_lighting() const {
  return _lighting_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_background_type
//       Access: Public
//  Description: Returns the current background type setting.
////////////////////////////////////////////////////////////////////
INLINE WindowFramework::BackgroundType WindowFramework::
get_background_type() const {
  return _background_type;
}
