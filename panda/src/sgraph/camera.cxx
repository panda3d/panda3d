// Filename: camera.cxx
// Created by:  mike (09Jan97)
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
#include <pandabase.h>
#include "camera.h"
#include <projection.h>


////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Camera::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: Camera::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
Camera(const string &name) :
  ProjectionNode(name),
  _active(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
Camera(const Camera &copy) :
  ProjectionNode(copy),
  _active(copy._active),
  _scene(copy._scene)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Camera::
operator = (const Camera &copy) {
  ProjectionNode::operator = (copy);
  _active = copy._active;
  _scene = copy._scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *Camera::
make_copy() const {
  return new Camera(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_transform() const {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: Camera::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
~Camera(void) {
  // We don't have to destroy the display regions associated with the
  // camera; they're responsible for themselves.

  // We don't have to remove the camera from the global list, because
  // it must have already been removed--otherwise we couldn't be
  // deleting it!
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_num_drs
//       Access: Public
//  Description: Returns the number of display regions that share this
//               camera.
////////////////////////////////////////////////////////////////////
int Camera::
get_num_drs() const {
  return _display_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_dr
//       Access: Public
//  Description: Returns the nth display region that shares this
//               camera.
////////////////////////////////////////////////////////////////////
DisplayRegion *Camera::
get_dr(int index) const {
  nassertr(index >= 0 && index < get_num_drs(), NULL);
  return _display_regions[index];
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::add_display_region
//       Access: Private
//  Description: Adds the indicated DisplayRegion to the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
add_display_region(DisplayRegion *display_region) {
  _display_regions.push_back(display_region);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::remove_display_region
//       Access: Private
//  Description: Removes the indicated DisplayRegion from the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
remove_display_region(DisplayRegion *display_region) {
  DisplayRegions::iterator dri =
    find(_display_regions.begin(), _display_regions.end(), display_region);
  if (dri != _display_regions.end()) {
    _display_regions.erase(dri);
  }
}
