// Filename: eggGroup.I
// Created by:  drose (16Jan99)
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
//     Function: EggGroup::get_group_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroup::GroupType EggGroup::
get_group_type() const {
  return (GroupType)(_flags & F_group_type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_billboard_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_billboard_type(BillboardType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_billboard_type)==0);
  _flags = (_flags & ~F_billboard_type) | type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_billboard_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroup::BillboardType EggGroup::
get_billboard_type() const {
  return (BillboardType)(_flags & F_billboard_type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_billboard_center
//       Access: Public
//  Description: Sets the point around which the billboard will
//               rotate, if this node contains a billboard
//               specification.
//
//               If a billboard type is given but no billboard_center
//               is specified, then the group node is treated as an
//               <Instance>, and the billboard rotates around the
//               origin.  If, however, a billboard_center is
//               specified, then the group node is *not* treated as an
//               <Instance>, and the billboard rotates around the
//               specified point.
//
//               The point is in the same coordinate system as the
//               vertices of this node: usually global, but possibly
//               local if there is an <Instance> somewhere above.
//               Specifically, this is the coordinate system defined
//               by get_vertex_frame().
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_billboard_center(const LPoint3d &billboard_center) {
  _billboard_center = billboard_center;
  _flags2 |= F2_billboard_center;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_billboard_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_billboard_center() {
  _flags2 &= ~F2_billboard_center;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_billboard_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_billboard_center() const {
  return (_flags2 & F2_billboard_center) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_billboard_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3d &EggGroup::
get_billboard_center() const {
  nassertr(has_billboard_center(), _billboard_center);
  return _billboard_center;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_cs_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_cs_type(CollisionSolidType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_cs_type)==0);
  _flags = (_flags & ~F_cs_type) | type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_cs_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroup::CollisionSolidType EggGroup::
get_cs_type() const {
  return (CollisionSolidType)(_flags & F_cs_type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_collision_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_collision_name(const string &collision_name) {
  _collision_name = collision_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_collision_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_collision_name() {
  _collision_name = "";
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_collision_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_collision_name() const {
  return !_collision_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_collision_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &EggGroup::
get_collision_name() const {
  return _collision_name;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_collide_flags
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_collide_flags(int flags) {
  // Make sure the user didn't give us any stray bits.
  nassertv((flags & ~F_collide_flags)==0);
  _flags = (_flags & ~F_collide_flags) | flags;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_collide_flags
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroup::CollideFlags EggGroup::
get_collide_flags() const {
  return (EggGroup::CollideFlags)(_flags & F_collide_flags);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_dcs_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_dcs_flag(bool flag) {
  if (flag) {
    _flags |= F_dcs_flag;
  } else {
    _flags &= ~F_dcs_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_dcs_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_dcs_flag() const {
  return ((_flags & F_dcs_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_dart_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_dart_type(DartType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_dart_type)==0);
  _flags = (_flags & ~F_dart_type) | type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_dart_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggGroup::DartType EggGroup::
get_dart_type() const {
  return (DartType)(_flags & F_dart_type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_switch_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_switch_flag(bool flag) {
  if (flag) {
    _flags |= F_switch_flag;
  } else {
    _flags &= ~F_switch_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_switch_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_switch_flag() const {
  return ((_flags & F_switch_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_switch_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_switch_fps(double fps) {
  _fps = fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_switch_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE double EggGroup::
get_switch_fps() const {
  return _fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_objecttype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_objecttype(const string &objecttype) {
  _objecttype = objecttype;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_objecttype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_objecttype() {
  _objecttype = "";
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_objecttype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_objecttype() const {
  return !_objecttype.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_objecttype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &EggGroup::
get_objecttype() const {
  return _objecttype;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_model_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_model_flag(bool flag) {
  if (flag) {
    _flags |= F_model_flag;
  } else {
    _flags &= ~F_model_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_model_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_model_flag() const {
  return ((_flags & F_model_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_texlist_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_texlist_flag(bool flag) {
  if (flag) {
    _flags |= F_texlist_flag;
  } else {
    _flags &= ~F_texlist_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_texlist_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_texlist_flag() const {
  return ((_flags & F_texlist_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_nofog_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_nofog_flag(bool flag) {
  if (flag) {
    _flags |= F_nofog_flag;
  } else {
    _flags &= ~F_nofog_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_nofog_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_nofog_flag() const {
  return ((_flags & F_nofog_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_decal_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_decal_flag(bool flag) {
  if (flag) {
    _flags |= F_decal_flag;
  } else {
    _flags &= ~F_decal_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_decal_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_decal_flag() const {
  return ((_flags & F_decal_flag) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_direct_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_direct_flag(bool flag) {
  if (flag) {
    _flags |= F_direct_flag;
  } else {
    _flags &= ~F_direct_flag;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_direct_flag
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
get_direct_flag() const {
  return ((_flags & F_direct_flag) != 0);
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_collide_mask(CollideMask mask) {
  _collide_mask = mask;
  _flags2 |= F2_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_collide_mask() {
  _flags2 &= ~F2_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_collide_mask() const {
  return (_flags2 & F2_collide_mask) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollideMask EggGroup::
get_collide_mask() const {
  nassertr(has_collide_mask(), CollideMask());
  return _collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_from_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_from_collide_mask(CollideMask mask) {
  _from_collide_mask = mask;
  _flags2 |= F2_from_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_from_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_from_collide_mask() {
  _flags2 &= ~F2_from_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_from_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_from_collide_mask() const {
  return (_flags2 & F2_from_collide_mask) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_from_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollideMask EggGroup::
get_from_collide_mask() const {
  nassertr(has_from_collide_mask(), CollideMask());
  return _from_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_into_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_into_collide_mask(CollideMask mask) {
  _into_collide_mask = mask;
  _flags2 |= F2_into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_into_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_into_collide_mask() {
  _flags2 &= ~F2_into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_into_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_into_collide_mask() const {
  return (_flags2 & F2_into_collide_mask) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_into_collide_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollideMask EggGroup::
get_into_collide_mask() const {
  nassertr(has_into_collide_mask(), CollideMask());
  return _into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_lod
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
set_lod(const EggSwitchCondition &lod) {
  _lod = lod.make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_lod
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggGroup::
clear_lod() {
  _lod = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::has_lod
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_lod() const {
  return (_lod != (EggSwitchCondition *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_lod
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const EggSwitchCondition &EggGroup::
get_lod() const {
  return *_lod;
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroup::vref_begin
//       Access: Public
//  Description: Returns an iterator that can, in conjunction with
//               vref_end(), be used to traverse the entire set of
//               referenced vertices.  Each iterator returns a
//               pair<PT(EggVertex), double>.
//
//               This interface is not safe to use outside of
//               PANDAEGG.DLL.
////////////////////////////////////////////////////////////////////
INLINE EggGroup::VertexRef::const_iterator EggGroup::
vref_begin() const {
  return _vref.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::vref_end
//       Access: Public
//  Description: Returns an iterator that can, in conjunction with
//               vref_begin(), be used to traverse the entire set of
//               referenced vertices.  Each iterator returns a
//               pair<PT(EggVertex), double>.
//
//               This interface is not safe to use outside of
//               PANDAEGG.DLL.
////////////////////////////////////////////////////////////////////
INLINE EggGroup::VertexRef::const_iterator EggGroup::
vref_end() const {
  return _vref.end();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGrop::vref_size
//       Access: Public
//  Description: Returns the number of elements between vref_begin()
//               and vref_end().
//
//               This interface is not safe to use outside of
//               PANDAEGG.DLL.
////////////////////////////////////////////////////////////////////
INLINE EggGroup::VertexRef::size_type EggGroup::
vref_size() const {
  return _vref.size();
}
