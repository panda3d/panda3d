// Filename: eggGroup.h
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

#ifndef EGGGROUP_H
#define EGGGROUP_H

#include "pandabase.h"

#include "eggGroupNode.h"
#include "eggRenderMode.h"
#include "eggTransform3d.h"
#include "eggVertex.h"
#include "eggSwitchCondition.h"
#include "pt_EggVertex.h"

#include "luse.h"
#include "collideMask.h"
#include "vector_string.h"

////////////////////////////////////////////////////////////////////
//       Class : EggGroup
// Description : The main glue of the egg hierarchy, this corresponds
//               to the <Group>, <Instance>, and <Joint> type nodes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggGroup : public EggGroupNode, public EggRenderMode, public EggTransform3d {
public:
  typedef pmap<PT_EggVertex, double> VertexRef;
  typedef pmap<string, string> TagData;

  // These bits are all stored somewhere in _flags.
  enum GroupType {
    // The bits here must correspond to those in Flags, below.
    GT_invalid               = -1,
    GT_group                 = 0x00000000,
    GT_instance              = 0x00000001,
    GT_joint                 = 0x00000002,
  };
  enum DartType {
    // The bits here must correspond to those in Flags, below.
    DT_none                  = 0x00000000,
    DT_sync                  = 0x00000004,
    DT_nosync                = 0x00000008,
    DT_default               = 0x0000000c,
  };
  enum DCSType {
    // The bits here must correspond to those in Flags2, below.
    DC_none                  = 0x00000000,
    DC_local                 = 0x00000010,
    DC_net                   = 0x00000020,
    DC_default               = 0x00000030,
  };
  enum BillboardType {
    // The bits here must correspond to those in Flags, below.
    BT_none                  = 0x00000000,
    BT_axis                  = 0x00000020,
    BT_point_camera_relative = 0x00000040,
    BT_point_world_relative  = 0x00000080,
  };
  enum CollisionSolidType {
    // The bits here must correspond to those in Flags, below.
    CST_none                 = 0x00000000,
    CST_plane                = 0x00010000,
    CST_polygon              = 0x00020000,
    CST_polyset              = 0x00030000,
    CST_sphere               = 0x00040000,
    CST_inverse_sphere       = 0x00050000,
    CST_geode                = 0x00060000,
    CST_tube                 = 0x00070000,
  };
  enum CollideFlags {
    // The bits here must correspond to those in Flags, below.
    CF_none                  = 0x00000000,
    CF_intangible            = 0x00080000,
    CF_descend               = 0x00100000,
    CF_event                 = 0x00200000,
    CF_keep                  = 0x00400000,
    CF_solid                 = 0x00800000,
    CF_center                = 0x01000000,
    CF_turnstile             = 0x02000000,
  };

  EggGroup(const string &name = "");
  EggGroup(const EggGroup &copy);
  EggGroup &operator = (const EggGroup &copy);
  ~EggGroup();

  virtual void write(ostream &out, int indent_level) const;
  void write_billboard_flags(ostream &out, int indent_level) const;
  void write_collide_flags(ostream &out, int indent_level) const;
  void write_model_flags(ostream &out, int indent_level) const;
  void write_switch_flags(ostream &out, int indent_level) const;
  void write_object_types(ostream &out, int indent_level) const;
  void write_decal_flags(ostream &out, int indent_level) const;
  void write_tags(ostream &out, int indent_level) const;
  void write_render_mode(ostream &out, int indent_level) const;

  virtual bool is_joint() const;

  virtual EggRenderMode *determine_alpha_mode();
  virtual EggRenderMode *determine_depth_write_mode();
  virtual EggRenderMode *determine_depth_test_mode();
  virtual EggRenderMode *determine_draw_order();
  virtual EggRenderMode *determine_bin();

  void set_group_type(GroupType type);
  INLINE GroupType get_group_type() const;
  INLINE bool is_instance_type() const;

  INLINE void set_billboard_type(BillboardType type);
  INLINE BillboardType get_billboard_type() const;

  INLINE void set_billboard_center(const LPoint3d &billboard_center);
  INLINE void clear_billboard_center();
  INLINE bool has_billboard_center() const;
  INLINE const LPoint3d &get_billboard_center() const;

  INLINE void set_cs_type(CollisionSolidType type);
  INLINE CollisionSolidType get_cs_type() const;

  INLINE void set_collide_flags(int flags);
  INLINE CollideFlags get_collide_flags() const;

  INLINE void set_collision_name(const string &collision_name);
  INLINE void clear_collision_name();
  INLINE bool has_collision_name() const;
  INLINE const string &get_collision_name() const;

  INLINE void set_dcs_type(DCSType type);
  INLINE DCSType get_dcs_type() const;

  INLINE void set_dart_type(DartType type);
  INLINE DartType get_dart_type() const;

  INLINE void set_switch_flag(bool flag);
  INLINE bool get_switch_flag() const;

  INLINE void set_switch_fps(double fps);
  INLINE double get_switch_fps() const;

  INLINE void add_object_type(const string &object_type);
  INLINE void clear_object_types();
  INLINE int get_num_object_types() const;
  INLINE string get_object_type(int index) const;
  bool has_object_type(const string &object_type) const;
  bool remove_object_type(const string &object_type);

  INLINE void set_model_flag(bool flag);
  INLINE bool get_model_flag() const;

  INLINE void set_texlist_flag(bool flag);
  INLINE bool get_texlist_flag() const;

  INLINE void set_nofog_flag(bool flag);
  INLINE bool get_nofog_flag() const;

  INLINE void set_decal_flag(bool flag);
  INLINE bool get_decal_flag() const;

  INLINE void set_direct_flag(bool flag);
  INLINE bool get_direct_flag() const;

  INLINE void set_collide_mask(CollideMask mask);
  INLINE void clear_collide_mask();
  INLINE bool has_collide_mask() const;
  INLINE CollideMask get_collide_mask() const;

  INLINE void set_from_collide_mask(CollideMask mask);
  INLINE void clear_from_collide_mask();
  INLINE bool has_from_collide_mask() const;
  INLINE CollideMask get_from_collide_mask() const;

  INLINE void set_into_collide_mask(CollideMask mask);
  INLINE void clear_into_collide_mask();
  INLINE bool has_into_collide_mask() const;
  INLINE CollideMask get_into_collide_mask() const;

  INLINE void set_lod(const EggSwitchCondition &lod);
  INLINE void clear_lod();
  INLINE bool has_lod() const;
  INLINE const EggSwitchCondition &get_lod() const;

  INLINE void set_tag(const string &key, const string &value);
  INLINE string get_tag(const string &key) const;
  INLINE bool has_tag(const string &key) const;
  INLINE void clear_tag(const string &key);

  INLINE TagData::const_iterator tag_begin() const;
  INLINE TagData::const_iterator tag_end() const;
  INLINE TagData::size_type tag_size() const;

  void ref_vertex(EggVertex *vert, double membership = 1.0);
  void unref_vertex(EggVertex *vert);
  void unref_all_vertices();
  double get_vertex_membership(const EggVertex *vert) const;
  void set_vertex_membership(EggVertex *vert, double membership);
  void steal_vrefs(EggGroup *other);

  INLINE VertexRef::const_iterator vref_begin() const;
  INLINE VertexRef::const_iterator vref_end() const;
  INLINE VertexRef::size_type vref_size() const;

#ifndef NDEBUG
  void test_vref_integrity() const;
#else
  void test_vref_integrity() const { }
#endif  // NDEBUG

  static GroupType string_group_type(const string &string);
  static DartType string_dart_type(const string &string);
  static DCSType string_dcs_type(const string &string);
  static BillboardType string_billboard_type(const string &string);
  static CollisionSolidType string_cs_type(const string &string);
  static CollideFlags string_collide_flags(const string &string);

protected:
  void write_vertex_ref(ostream &out, int indent_level) const;
  virtual bool egg_start_parse_body();
  virtual void adjust_under();
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_flatten_transforms();

  virtual void transform_changed();

private:

  enum Flags {
    F_group_type             = 0x00000003,
    F_dart_type              = 0x0000000c,

    F_billboard_type         = 0x000000e0,
    F_switch_flag            = 0x00000100,
    F_model_flag             = 0x00000400,
    F_texlist_flag           = 0x00000800,
    F_nofog_flag             = 0x00001000,
    F_decal_flag             = 0x00002000,
    F_direct_flag            = 0x00004000,
    F_cs_type                = 0x00070000,
    F_collide_flags          = 0x03f80000,
  };
  enum Flags2 {
    F2_collide_mask          = 0x00000001,
    F2_from_collide_mask     = 0x00000002,
    F2_into_collide_mask     = 0x00000004,
    F2_billboard_center      = 0x00000008,

    F2_dcs_type              = 0x00000030,
  };

  int _flags;
  int _flags2;
  CollideMask _collide_mask, _from_collide_mask, _into_collide_mask;
  LPoint3d _billboard_center;
  vector_string _object_types;
  string _collision_name;
  double _fps;
  PT(EggSwitchCondition) _lod;
  TagData _tag_data;
  VertexRef _vref;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroupNode::init_type();
    EggRenderMode::init_type();
    register_type(_type_handle, "EggGroup",
                  EggGroupNode::get_class_type(),
                  EggRenderMode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

ostream &operator << (ostream &out, EggGroup::GroupType t);
ostream &operator << (ostream &out, EggGroup::DartType t);
ostream &operator << (ostream &out, EggGroup::DCSType t);
ostream &operator << (ostream &out, EggGroup::BillboardType t);
ostream &operator << (ostream &out, EggGroup::CollisionSolidType t);
ostream &operator << (ostream &out, EggGroup::CollideFlags t);


#include "eggGroup.I"

#endif

