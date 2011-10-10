// Filename: cLerpNodePathInterval.I
// Created by:  drose (27Aug02)
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
set_start_pos(const LVecBase3 &pos) {
  nassertv(!pos.is_nan());
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
set_end_pos(const LVecBase3 &pos) {
  nassertv(!pos.is_nan());
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
set_start_hpr(const LVecBase3 &hpr) {
  nassertv(!hpr.is_nan());
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
set_end_hpr(const LVecBase3 &hpr) {
  nassertv(!hpr.is_nan());
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
set_end_hpr(const LQuaternion &quat) {
  nassertv(!quat.is_nan());
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
set_start_quat(const LQuaternion &quat) {
  nassertv(!quat.is_nan());
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
set_end_quat(const LVecBase3 &hpr) {
  nassertv(!hpr.is_nan());
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
set_end_quat(const LQuaternion &quat) {
  nassertv(!quat.is_nan());
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
set_start_scale(const LVecBase3 &scale) {
  nassertv(!scale.is_nan());
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
set_start_scale(PN_stdfloat scale) {
  nassertv(!cnan(scale));
  set_start_scale(LVecBase3(scale, scale, scale));
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
set_end_scale(const LVecBase3 &scale) {
  nassertv(!scale.is_nan());
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
set_end_scale(PN_stdfloat scale) {
  nassertv(!cnan(scale));
  set_end_scale(LVecBase3(scale, scale, scale));
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
set_start_shear(const LVecBase3 &shear) {
  nassertv(!shear.is_nan());
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
set_end_shear(const LVecBase3 &shear) {
  nassertv(!shear.is_nan());
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
set_start_color(const LVecBase4 &color) {
  nassertv(!color.is_nan());
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
set_end_color(const LVecBase4 &color) {
  nassertv(!color.is_nan());
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
set_start_color_scale(const LVecBase4 &color_scale) {
  nassertv(!color_scale.is_nan());
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
set_end_color_scale(const LVecBase4 &color_scale) {
  nassertv(!color_scale.is_nan());
  _end_color_scale = color_scale;
  _flags |= F_end_color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_texture_stage
//       Access: Published
//  Description: Indicates the texture stage that is adjusted by
//               tex_offset, tex_rotate, and/or tex_scale.  If this is
//               not set, the default is the default texture stage.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_texture_stage(TextureStage *stage) {
  _texture_stage = stage;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_tex_offset
//       Access: Published
//  Description: Indicates the initial UV offset of the lerped node.
//               This is meaningful only if set_end_tex_offset() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               UV offset at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_tex_offset(const LVecBase2 &tex_offset) {
  nassertv(!tex_offset.is_nan());
  _start_tex_offset = tex_offset;
  _flags |= F_start_tex_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_tex_offset
//       Access: Published
//  Description: Indicates that the UV offset of the node should be
//               lerped, and specifies the final UV offset of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's UV offset will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_tex_offset(const LVecBase2 &tex_offset) {
  nassertv(!tex_offset.is_nan());
  _end_tex_offset = tex_offset;
  _flags |= F_end_tex_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_tex_rotate
//       Access: Published
//  Description: Indicates the initial UV rotate of the lerped node.
//               This is meaningful only if set_end_tex_rotate() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               UV rotate at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_tex_rotate(PN_stdfloat tex_rotate) {
  nassertv(!cnan(tex_rotate));
  _start_tex_rotate = tex_rotate;
  _flags |= F_start_tex_rotate;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_tex_rotate
//       Access: Published
//  Description: Indicates that the UV rotate of the node should be
//               lerped, and specifies the final UV rotate of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's UV rotate will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_tex_rotate(PN_stdfloat tex_rotate) {
  nassertv(!cnan(tex_rotate));
  _end_tex_rotate = tex_rotate;
  _flags |= F_end_tex_rotate;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_start_tex_scale
//       Access: Published
//  Description: Indicates the initial UV scale of the lerped node.
//               This is meaningful only if set_end_tex_scale() is also
//               called.  This parameter is optional; if unspecified,
//               the value will be taken from the node's actual
//               UV scale at the time the lerp is performed.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_start_tex_scale(const LVecBase2 &tex_scale) {
  nassertv(!tex_scale.is_nan());
  _start_tex_scale = tex_scale;
  _flags |= F_start_tex_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_end_tex_scale
//       Access: Published
//  Description: Indicates that the UV scale of the node should be
//               lerped, and specifies the final UV scale of the node.
//               This should be called before priv_initialize().  If this
//               is not called, the node's UV scale will not be
//               affected by the lerp.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_end_tex_scale(const LVecBase2 &tex_scale) {
  nassertv(!tex_scale.is_nan());
  _end_tex_scale = tex_scale;
  _flags |= F_end_tex_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::set_override
//       Access: Published
//  Description: Changes the override value that will be associated
//               with any state changes applied by the lerp.  If this
//               lerp is changing state (for instance, a color lerp or
//               a tex matrix lerp), then the new attributes created
//               by this lerp will be assigned the indicated override
//               value when they are applied to the node.
////////////////////////////////////////////////////////////////////
INLINE void CLerpNodePathInterval::
set_override(int override) {
  _override = override;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::get_override
//       Access: Published
//  Description: Returns the override value that will be associated
//               with any state changes applied by the lerp.  See
//               set_override().
////////////////////////////////////////////////////////////////////
INLINE int CLerpNodePathInterval::
get_override() const {
  return _override;
}
