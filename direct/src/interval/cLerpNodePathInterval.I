// Filename: cLerpNodePathInterval.I
// Created by:  drose (27Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::get_node
//       Access: Published
//  Description: Returns the node being lerped.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CLerpNodePathInterval::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::get_other
//       Access: Published
//  Description: Returns the "other" node, which the lerped node is
//               being moved relative to.  If this is an empty node
//               path, the lerped node is being moved in its own
//               coordinate system.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CLerpNodePathInterval::
get_other() const {
  return _other;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_pos
//       Access: Published
//  Description: Indicates the initial position of the lerped node.
//               This is meaningful only if set_end_pos() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               position at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_pos(const LVecBase3f &pos) {
  _start_pos = pos;
  _flags |= F_start_pos;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_pos
//       Access: Published
//  Description: Indicates that the position of the node should be
//               lerped, and specifies the final position of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's position will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_pos(const LVecBase3f &pos) {
  _end_pos = pos;
  _flags |= F_end_pos;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_hpr
//       Access: Published
//  Description: Indicates the initial rotation of the lerped node.
//               This is meaningful only if either set_end_hpr() or
//               set_end_quat() is also called.  This parameter is
//               optional; if unspecified, the value will be taken
//               from the node's actual rotation at the time the lerp
//               is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_hpr(const LVecBase3f &hpr) {
  _start_hpr = hpr;
  _flags = (_flags & ~(F_slerp_setup | F_start_quat)) | F_start_hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_hpr
//       Access: Published
//  Description: Indicates that the rotation of the node should be
//               lerped, and specifies the final rotation of the node.
//               This should be called before priv_initialize().
//
//               This replaces a previous call to set_end_quat().  If
//               neither set_end_hpr() nor set_end_quat() is called,
//               the node's rotation will not be affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_hpr(const LVecBase3f &hpr) {
  _end_hpr = hpr;
  _flags = (_flags & ~F_end_quat) | F_end_hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_hpr
//       Access: Published
//  Description: Indicates that the rotation of the node should be
//               lerped, and specifies the final rotation of the node.
//               This should be called before priv_initialize().
//
//               This special function is overloaded to accept a
//               quaternion, even though the function name is
//               set_end_hpr().  The quaternion will be implicitly
//               converted to a HPR trio, and the lerp will be
//               performed in HPR space, componentwise.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_hpr(const LQuaternionf &quat) {
  _end_hpr = quat.get_hpr();
  _flags = (_flags & ~F_end_quat) | F_end_hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_quat
//       Access: Published
//  Description: Indicates the initial rotation of the lerped node.
//               This is meaningful only if either set_end_quat() or
//               set_end_hpr() is also called.  This parameter is
//               optional; if unspecified, the value will be taken
//               from the node's actual rotation at the time the lerp
//               is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_quat(const LQuaternionf &quat) {
  _start_quat = quat;
  _flags = (_flags & ~(F_slerp_setup | F_start_hpr)) | F_start_quat;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_quat
//       Access: Published
//  Description: Indicates that the rotation of the node should be
//               lerped, and specifies the final rotation of the node.
//               This should be called before priv_initialize().
//
//               This replaces a previous call to set_end_hpr().  If
//               neither set_end_quat() nor set_end_hpr() is called,
//               the node's rotation will not be affected by the lerp.
//
//               This special function is overloaded to accept a HPR
//               trio, even though the function name is
//               set_end_quat().  The HPR will be implicitly converted
//               to a quaternion, and the lerp will be performed in
//               quaternion space, as a spherical lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_quat(const LVecBase3f &hpr) {
  _end_quat.set_hpr(hpr);
  _flags = (_flags & ~(F_slerp_setup | F_end_hpr)) | F_end_quat;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_quat
//       Access: Published
//  Description: Indicates that the rotation of the node should be
//               lerped, and specifies the final rotation of the node.
//               This should be called before priv_initialize().
//
//               This replaces a previous call to set_end_hpr().  If
//               neither set_end_quat() nor set_end_hpr() is called,
//               the node's rotation will not be affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_quat(const LQuaternionf &quat) {
  _end_quat = quat;
  _flags = (_flags & ~(F_slerp_setup | F_end_hpr)) | F_end_quat;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_scale
//       Access: Published
//  Description: Indicates the initial scale of the lerped node.
//               This is meaningful only if set_end_scale() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               scale at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_scale(const LVecBase3f &scale) {
  _start_scale = scale;
  _flags |= F_start_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_scale
//       Access: Published
//  Description: Indicates the initial scale of the lerped node.
//               This is meaningful only if set_end_scale() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               scale at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_scale(float scale) {
  set_start_scale(LVecBase3f(scale, scale, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_scale
//       Access: Published
//  Description: Indicates that the scale of the node should be
//               lerped, and specifies the final scale of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's scale will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_scale(const LVecBase3f &scale) {
  _end_scale = scale;
  _flags |= F_end_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_scale
//       Access: Published
//  Description: Indicates that the scale of the node should be
//               lerped, and specifies the final scale of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's scale will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_scale(float scale) {
  set_end_scale(LVecBase3f(scale, scale, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_shear
//       Access: Published
//  Description: Indicates the initial shear of the lerped node.
//               This is meaningful only if set_end_shear() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               shear at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_shear(const LVecBase3f &shear) {
  _start_shear = shear;
  _flags |= F_start_shear;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_shear
//       Access: Published
//  Description: Indicates that the shear of the node should be
//               lerped, and specifies the final shear of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's shear will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_shear(const LVecBase3f &shear) {
  _end_shear = shear;
  _flags |= F_end_shear;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_color
//       Access: Published
//  Description: Indicates the initial color of the lerped node.
//               This is meaningful only if set_end_color() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               color at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_color(const LVecBase4f &color) {
  _start_color = color;
  _flags |= F_start_color;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_color
//       Access: Published
//  Description: Indicates that the color of the node should be
//               lerped, and specifies the final color of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's color will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_color(const LVecBase4f &color) {
  _end_color = color;
  _flags |= F_end_color;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_color_scale
//       Access: Published
//  Description: Indicates the initial color scale of the lerped node.
//               This is meaningful only if set_end_color_scale() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               color scale at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_color_scale(const LVecBase4f &color_scale) {
  _start_color_scale = color_scale;
  _flags |= F_start_color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_color_scale
//       Access: Published
//  Description: Indicates that the color scale of the node should be
//               lerped, and specifies the final color scale of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's color scale will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_color_scale(const LVecBase4f &color_scale) {
  _end_color_scale = color_scale;
  _flags |= F_end_color_scale;
}
