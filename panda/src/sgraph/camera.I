// Filename: camera.I
// Created by:  drose (27Sep99)
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
