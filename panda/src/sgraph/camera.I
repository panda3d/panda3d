// Filename: camera.I
// Created by:  drose (27Sep99)
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
//     Function: Camera::set_active
//       Access: Public
//  Description: Sets the active flag on the camera.  When the camera
//               is not active, nothing will be rendered.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::is_active
//       Access: Public
//  Description: Returns the current setting of the active flag on the
//               camera.
////////////////////////////////////////////////////////////////////
INLINE bool Camera::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_scene
//       Access: Public
//  Description: Sets the scene that will be rendered by the camera.
//               This is normally the root node of a scene graph,
//               typically a node called 'render', although it could
//               represent the root of any subgraph.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_scene(Node *scene) {
  _scene = scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_scene
//       Access: Public
//  Description: Returns the scene that will be rendered by the
//               camera.  See set_scene().
////////////////////////////////////////////////////////////////////
INLINE Node *Camera::
get_scene() const {
  return _scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_qpscene
//       Access: Public
//  Description: Sets the scene that will be rendered by the camera.
//               This is normally the root node of a scene graph,
//               typically a node called 'render', although it could
//               represent the root of any subgraph.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_qpscene(PandaNode *scene) {
  _qpscene = scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_qpscene
//       Access: Public
//  Description: Returns the scene that will be rendered by the
//               camera.  See set_qpscene().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *Camera::
get_qpscene() const {
  return _qpscene;
}
