// Filename: eggGroup.I
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

INLINE EggGroup::GroupType EggGroup::
get_group_type() const {
  return (GroupType)(_flags & F_group_type);
}

INLINE void EggGroup::
set_billboard_type(BillboardType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_billboard_type)==0);
  _flags = (_flags & ~F_billboard_type) | type;
}
INLINE EggGroup::BillboardType EggGroup::
get_billboard_type() const {
  return (BillboardType)(_flags & F_billboard_type);
}

INLINE void EggGroup::
set_cs_type(CollisionSolidType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_cs_type)==0);
  _flags = (_flags & ~F_cs_type) | type;
}
INLINE EggGroup::CollisionSolidType EggGroup::
get_cs_type() const {
  return (CollisionSolidType)(_flags & F_cs_type);
}

INLINE void EggGroup::
set_collision_name(const string &collision_name) {
  _collision_name = collision_name;
}
INLINE void EggGroup::
clear_collision_name() {
  _collision_name = "";
}
INLINE bool EggGroup::
has_collision_name() const {
  return !_collision_name.empty();
}
INLINE const string &EggGroup::
get_collision_name() const {
  return _collision_name;
}

INLINE void EggGroup::
set_collide_flags(int flags) {
  // Make sure the user didn't give us any stray bits.
  nassertv((flags & ~F_collide_flags)==0);
  _flags = (_flags & ~F_collide_flags) | flags;
}
INLINE EggGroup::CollideFlags EggGroup::
get_collide_flags() const {
  return (EggGroup::CollideFlags)(_flags & F_collide_flags);
}

INLINE void EggGroup::
set_dcs_flag(bool flag) {
  if (flag) {
    _flags |= F_dcs_flag;
  } else {
    _flags &= ~F_dcs_flag;
  }
}
INLINE bool EggGroup::
get_dcs_flag() const {
  return ((_flags & F_dcs_flag) != 0);
}

INLINE void EggGroup::
set_dart_type(DartType type) {
  // Make sure the user didn't give us any stray bits.
  nassertv((type & ~F_dart_type)==0);
  _flags = (_flags & ~F_dart_type) | type;
}
INLINE EggGroup::DartType EggGroup::
get_dart_type() const {
  return (DartType)(_flags & F_dart_type);
}

INLINE void EggGroup::
set_switch_flag(bool flag) {
  if (flag) {
    _flags |= F_switch_flag;
  } else {
    _flags &= ~F_switch_flag;
  }
}
INLINE bool EggGroup::
get_switch_flag() const {
  return ((_flags & F_switch_flag) != 0);
}

INLINE void EggGroup::
set_switch_fps(double fps) {
  _fps = fps;
}
INLINE double EggGroup::
get_switch_fps() const {
  return _fps;
}

INLINE bool EggGroup::
has_transform() const {
  return (_flags & F_has_transform) != 0;
}
INLINE LMatrix4d EggGroup::
get_transform() const {
  nassertr(_flags & F_has_transform, _transform);
  return _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::transform_is_identity()
//       Access: Public
//  Description: Returns true if no transform matrix has been
//               specified, or if the one specified is the identity
//               transform.  Returns false only if a nonidentity
//               transform has been applied.
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
transform_is_identity() const {
  return (!has_transform() || 
	  _transform.almost_equal(LMatrix4d::ident_mat(), 0.0001));
}

INLINE void EggGroup::
set_objecttype(const string &objecttype) {
  _objecttype = objecttype;
}
INLINE void EggGroup::
clear_objecttype() {
  _objecttype = "";
}
INLINE bool EggGroup::
has_objecttype() const {
  return !_objecttype.empty();
}
INLINE const string &EggGroup::
get_objecttype() const {
  return _objecttype;
}

INLINE void EggGroup::
set_model_flag(bool flag) {
  if (flag) {
    _flags |= F_model_flag;
  } else {
    _flags &= ~F_model_flag;
  }
}
INLINE bool EggGroup::
get_model_flag() const {
  return ((_flags & F_model_flag) != 0);
}

INLINE void EggGroup::
set_texlist_flag(bool flag) {
  if (flag) {
    _flags |= F_texlist_flag;
  } else {
    _flags &= ~F_texlist_flag;
  }
}
INLINE bool EggGroup::
get_texlist_flag() const {
  return ((_flags & F_texlist_flag) != 0);
}

INLINE void EggGroup::
set_nofog_flag(bool flag) {
  if (flag) {
    _flags |= F_nofog_flag;
  } else {
    _flags &= ~F_nofog_flag;
  }
}
INLINE bool EggGroup::
get_nofog_flag() const {
  return ((_flags & F_nofog_flag) != 0);
}

INLINE void EggGroup::
set_decal_flag(bool flag) {
  if (flag) {
    _flags |= F_decal_flag;
  } else {
    _flags &= ~F_decal_flag;
  }
}
INLINE bool EggGroup::
get_decal_flag() const {
  return ((_flags & F_decal_flag) != 0);
}

INLINE void EggGroup::
set_direct_flag(bool flag) {
  if (flag) {
    _flags |= F_direct_flag;
  } else {
    _flags &= ~F_direct_flag;
  }
}
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
//     Function: EggGroup::has_lod
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool EggGroup::
has_lod() const {
  return (!(_lod == NULL));
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
////////////////////////////////////////////////////////////////////
INLINE EggGroup::VertexRef::size_type EggGroup::
vref_size() const {
  return _vref.size();
}


