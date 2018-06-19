/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroup.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef EGGGROUP_H
#define EGGGROUP_H

#include "pandabase.h"

#include "eggGroupNode.h"
#include "eggRenderMode.h"
#include "eggTransform.h"
#include "eggVertex.h"
#include "eggSwitchCondition.h"
#include "pt_EggVertex.h"

#include "luse.h"
#include "collideMask.h"
#include "vector_string.h"

/**
 * The main glue of the egg hierarchy, this corresponds to the <Group>,
 * <Instance>, and <Joint> type nodes.
 */
class EXPCL_PANDA_EGG EggGroup : public EggGroupNode, public EggRenderMode, public EggTransform {
PUBLISHED:
  typedef pmap<PT_EggVertex, double> VertexRef;
  typedef pmap<std::string, std::string> TagData;

  // These bits are all stored somewhere in _flags.
  enum GroupType {
    // The bits here must correspond to those in Flags, below.
    GT_invalid               = -1,
    GT_group                 = 0x00000000,
    GT_instance              = 0x00000001,
    GT_joint                 = 0x00000002,
  };
  enum DCSType {
    // The bits here must correspond to those in Flags2, below.
    DC_unspecified           = 0x00000000,
    DC_none                  = 0x00000010,
    DC_local                 = 0x00000020,
    DC_net                   = 0x00000030,
    DC_no_touch              = 0x00000040,
    DC_default               = 0x00000050,
  };
  enum BillboardType {
    // The bits here must correspond to those in Flags, below.
    BT_none                  = 0x00000000,
    BT_axis                  = 0x00000020,
    BT_point_camera_relative = 0x00000040,
    BT_point_world_relative  = 0x00000080,
  };
  enum CollisionSolidType {
    // The bits here must correspond to those in Flags, below, and they must
    // fit within F_cs_type.
    CST_none                 = 0x00000000,
    CST_plane                = 0x00010000,
    CST_polygon              = 0x00020000,
    CST_polyset              = 0x00030000,
    CST_sphere               = 0x00040000,
    CST_tube                 = 0x00050000,
    CST_inv_sphere           = 0x00060000,
    CST_box                  = 0x00070000,
    CST_floor_mesh           = 0x00080000,
  };
  enum CollideFlags {
    // The bits here must correspond to those in Flags, below, and they must
    // fit within F_collide_flags.
    CF_none                  = 0x00000000,
    CF_descend               = 0x00100000,
    CF_event                 = 0x00200000,
    CF_keep                  = 0x00400000,
    CF_solid                 = 0x00800000,
    CF_center                = 0x01000000,
    CF_turnstile             = 0x02000000,
    CF_level                 = 0x04000000,
    CF_intangible            = 0x08000000,
  };

  enum DartType {
    // The bits here must correspond to those in Flags, below.
    DT_none                  = 0x00000000,
    DT_structured            = 0x10000000,
    DT_sync                  = 0x20000000,
    DT_nosync                = 0x30000000,
    DT_default               = 0x40000000,
  };


  // These correspond to ColorBlendAttrib::Mode (but not numerically).
  enum BlendMode {
    BM_unspecified,
    BM_none,
    BM_add,
    BM_subtract,
    BM_inv_subtract,
    BM_min,
    BM_max
  };

  // These correspond to ColorBlendAttrib::Operand (but not numerically).
  enum BlendOperand {
    BO_unspecified,
    BO_zero,
    BO_one,
    BO_incoming_color,
    BO_one_minus_incoming_color,
    BO_fbuffer_color,
    BO_one_minus_fbuffer_color,
    BO_incoming_alpha,
    BO_one_minus_incoming_alpha,
    BO_fbuffer_alpha,
    BO_one_minus_fbuffer_alpha,
    BO_constant_color,
    BO_one_minus_constant_color,
    BO_constant_alpha,
    BO_one_minus_constant_alpha,
    BO_incoming_color_saturate,
    BO_color_scale,
    BO_one_minus_color_scale,
    BO_alpha_scale,
    BO_one_minus_alpha_scale,
  };

  explicit EggGroup(const std::string &name = "");
  EggGroup(const EggGroup &copy);
  EggGroup &operator = (const EggGroup &copy);
  ~EggGroup();

  virtual void write(std::ostream &out, int indent_level) const;
  void write_billboard_flags(std::ostream &out, int indent_level) const;
  void write_collide_flags(std::ostream &out, int indent_level) const;
  void write_model_flags(std::ostream &out, int indent_level) const;
  void write_switch_flags(std::ostream &out, int indent_level) const;
  void write_object_types(std::ostream &out, int indent_level) const;
  void write_decal_flags(std::ostream &out, int indent_level) const;
  void write_tags(std::ostream &out, int indent_level) const;
  void write_render_mode(std::ostream &out, int indent_level) const;

  virtual bool is_joint() const;

  virtual EggRenderMode *determine_alpha_mode();
  virtual EggRenderMode *determine_depth_write_mode();
  virtual EggRenderMode *determine_depth_test_mode();
  virtual EggRenderMode *determine_visibility_mode();
  virtual EggRenderMode *determine_depth_offset();
  virtual EggRenderMode *determine_draw_order();
  virtual EggRenderMode *determine_bin();
  virtual bool determine_indexed();
  virtual bool determine_decal();

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

  INLINE void set_collision_name(const std::string &collision_name);
  INLINE void clear_collision_name();
  INLINE bool has_collision_name() const;
  INLINE const std::string &get_collision_name() const;

  INLINE void set_dcs_type(DCSType type);
  INLINE DCSType get_dcs_type() const;
  INLINE bool has_dcs_type() const;

  INLINE void set_dart_type(DartType type);
  INLINE DartType get_dart_type() const;

  INLINE void set_switch_flag(bool flag);
  INLINE bool get_switch_flag() const;

  INLINE void set_switch_fps(double fps);
  INLINE double get_switch_fps() const;

  INLINE void add_object_type(const std::string &object_type);
  INLINE void clear_object_types();
  INLINE int get_num_object_types() const;
  INLINE std::string get_object_type(int index) const;
  MAKE_SEQ(get_object_types, get_num_object_types, get_object_type);
  bool has_object_type(const std::string &object_type) const;
  bool remove_object_type(const std::string &object_type);

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

  INLINE void set_portal_flag(bool flag);
  INLINE bool get_portal_flag() const;

  INLINE void set_occluder_flag(bool flag);
  INLINE bool get_occluder_flag() const;

  INLINE void set_polylight_flag(bool flag);
  INLINE bool get_polylight_flag() const;

  INLINE void set_indexed_flag(bool flag);
  INLINE void clear_indexed_flag();
  INLINE bool has_indexed_flag() const;
  INLINE bool get_indexed_flag() const;

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

  INLINE void set_blend_mode(BlendMode blend_mode);
  INLINE BlendMode get_blend_mode() const;
  INLINE void set_blend_operand_a(BlendOperand blend_operand_a);
  INLINE BlendOperand get_blend_operand_a() const;
  INLINE void set_blend_operand_b(BlendOperand blend_operand_b);
  INLINE BlendOperand get_blend_operand_b() const;
  INLINE void set_blend_color(const LColor &blend_color);
  INLINE void clear_blend_color();
  INLINE bool has_blend_color() const;
  INLINE const LColor &get_blend_color() const;

  INLINE void set_lod(const EggSwitchCondition &lod);
  INLINE void clear_lod();
  INLINE bool has_lod() const;
  INLINE const EggSwitchCondition &get_lod() const;

  INLINE void set_tag(const std::string &key, const std::string &value);
  INLINE std::string get_tag(const std::string &key) const;
  INLINE bool has_tag(const std::string &key) const;
  INLINE void clear_tag(const std::string &key);

  INLINE const EggTransform &get_default_pose() const;
  INLINE EggTransform &modify_default_pose();
  INLINE void set_default_pose(const EggTransform &transform);
  INLINE void clear_default_pose();

  INLINE void set_scroll_u(const double u_speed);
  INLINE void set_scroll_v(const double v_speed);
  INLINE void set_scroll_w(const double w_speed);
  INLINE void set_scroll_r(const double r_speed);
  INLINE double get_scroll_u() const;
  INLINE double get_scroll_v() const;
  INLINE double get_scroll_w() const;
  INLINE double get_scroll_r() const;

  INLINE bool has_scrolling_uvs();

  MAKE_PROPERTY(group_type, get_group_type, set_group_type);
  MAKE_PROPERTY(billboard_type, get_billboard_type, set_billboard_type);
  MAKE_PROPERTY2(billboard_center, has_billboard_center, get_billboard_center,
                                   set_billboard_center, clear_billboard_center);
  MAKE_PROPERTY(cs_type, get_cs_type, set_cs_type);
  MAKE_PROPERTY(collide_flags, get_collide_flags, set_collide_flags);
  MAKE_PROPERTY(collision_name, get_collision_name, set_collision_name);
  MAKE_PROPERTY(dcs_type, get_dcs_type, set_dcs_type);
  MAKE_PROPERTY(dart_type, get_dart_type, set_dart_type);
  MAKE_PROPERTY(switch_flag, get_switch_flag, set_switch_flag);
  MAKE_PROPERTY(switch_fps, get_switch_fps, set_switch_fps);
  MAKE_SEQ_PROPERTY(object_types, get_num_object_types, get_object_type);
  MAKE_PROPERTY(model_flag, get_model_flag, set_model_flag);
  MAKE_PROPERTY(texlist_flag, get_texlist_flag, set_texlist_flag);
  MAKE_PROPERTY(nofog_flag, get_nofog_flag, set_nofog_flag);
  MAKE_PROPERTY(decal_flag, get_decal_flag, set_decal_flag);
  MAKE_PROPERTY(direct_flag, get_direct_flag, set_direct_flag);
  MAKE_PROPERTY(portal_flag, get_portal_flag, set_portal_flag);
  MAKE_PROPERTY(occluder_flag, get_occluder_flag, set_occluder_flag);
  MAKE_PROPERTY2(indexed_flag, has_indexed_flag, get_indexed_flag,
                               set_indexed_flag, clear_indexed_flag);
  MAKE_PROPERTY2(collide_mask, has_collide_mask, get_collide_mask,
                               set_collide_mask, clear_collide_mask);
  MAKE_PROPERTY2(from_collide_mask, has_from_collide_mask, get_from_collide_mask,
                                    set_from_collide_mask, clear_from_collide_mask);
  MAKE_PROPERTY2(into_collide_mask, has_into_collide_mask, get_into_collide_mask,
                                    set_into_collide_mask, clear_into_collide_mask);
  MAKE_PROPERTY(blend_mode, get_blend_mode, set_blend_mode);
  MAKE_PROPERTY(blend_operand_a, get_blend_operand_a, set_blend_operand_a);
  MAKE_PROPERTY(blend_operand_b, get_blend_operand_b, set_blend_operand_b);
  MAKE_PROPERTY2(blend_color, has_blend_color, get_blend_color,
                              set_blend_color, clear_blend_color);
  MAKE_PROPERTY2(lod, has_lod, get_lod, set_lod, clear_lod);
  MAKE_PROPERTY(default_pose, get_default_pose, set_default_pose);
  MAKE_PROPERTY(scroll_u, get_scroll_u, set_scroll_u);
  MAKE_PROPERTY(scroll_v, get_scroll_v, set_scroll_v);
  MAKE_PROPERTY(scroll_w, get_scroll_w, set_scroll_w);
  MAKE_PROPERTY(scroll_r, get_scroll_r, set_scroll_r);

public:
  INLINE TagData::const_iterator tag_begin() const;
  INLINE TagData::const_iterator tag_end() const;
  INLINE TagData::size_type tag_size() const;

PUBLISHED:
  void ref_vertex(EggVertex *vert, double membership = 1.0);
  void unref_vertex(EggVertex *vert);
  void unref_all_vertices();
  double get_vertex_membership(const EggVertex *vert) const;
  void set_vertex_membership(EggVertex *vert, double membership);
  void steal_vrefs(EggGroup *other);

public:
  INLINE VertexRef::const_iterator vref_begin() const;
  INLINE VertexRef::const_iterator vref_end() const;
  INLINE VertexRef::size_type vref_size() const;

PUBLISHED:
#ifdef _DEBUG
  void test_vref_integrity() const;
#else
  void test_vref_integrity() const { }
#endif  // _DEBUG

  void add_group_ref(EggGroup *group);
  int get_num_group_refs() const;
  EggGroup *get_group_ref(int n) const;
  MAKE_SEQ(get_group_refs, get_num_group_refs, get_group_ref);
  void remove_group_ref(int n);
  void clear_group_refs();

  static GroupType string_group_type(const std::string &strval);
  static DartType string_dart_type(const std::string &strval);
  static DCSType string_dcs_type(const std::string &strval);
  static BillboardType string_billboard_type(const std::string &strval);
  static CollisionSolidType string_cs_type(const std::string &strval);
  static CollideFlags string_collide_flags(const std::string &strval);
  static BlendMode string_blend_mode(const std::string &strval);
  static BlendOperand string_blend_operand(const std::string &strval);

public:
  virtual EggTransform *as_transform();

protected:
  void write_vertex_ref(std::ostream &out, int indent_level) const;
  virtual bool egg_start_parse_body();
  virtual void adjust_under();
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_flatten_transforms();

  virtual void transform_changed();

private:

  enum Flags {
    F_group_type             = 0x00000003,

    F_billboard_type         = 0x000000e0,
    F_switch_flag            = 0x00000100,
    F_model_flag             = 0x00000400,
    F_texlist_flag           = 0x00000800,
    F_nofog_flag             = 0x00001000,
    F_decal_flag             = 0x00002000,
    F_direct_flag            = 0x00004000,
    F_cs_type                = 0x000f0000,
    F_collide_flags          = 0x0ff00000,
    F_dart_type              = 0xf0000000,
  };
  enum Flags2 {
    F2_collide_mask          = 0x00000001,
    F2_from_collide_mask     = 0x00000002,
    F2_into_collide_mask     = 0x00000004,
    F2_billboard_center      = 0x00000008,

    F2_dcs_type              = 0x00000070,
    F2_portal_flag           = 0x00000080,
    F2_polylight_flag        = 0x00000100,
    F2_indexed_flag          = 0x00000200,
    F2_has_indexed_flag      = 0x00000400,
    F2_has_blend_color       = 0x00000800,
    F2_occluder_flag         = 0x00001000,
  };

  int _flags;
  int _flags2;
  CollideMask _collide_mask, _from_collide_mask, _into_collide_mask;
  BlendMode _blend_mode;
  BlendOperand _blend_operand_a;
  BlendOperand _blend_operand_b;
  LColor _blend_color;
  LPoint3d _billboard_center;
  vector_string _object_types;
  std::string _collision_name;
  double _fps;
  PT(EggSwitchCondition) _lod;
  TagData _tag_data;

  double _u_speed;
  double _v_speed;
  double _w_speed;
  double _r_speed;

  // This is the <DefaultPose> entry for a <Joint>.  It is not the <Transform>
  // entry (that is stored via inheritance, in the EggTransform class we
  // inherit from).
  EggTransform _default_pose;

  VertexRef _vref;

  typedef pvector< PT(EggGroup) > GroupRefs;
  GroupRefs _group_refs;


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

std::ostream &operator << (std::ostream &out, EggGroup::GroupType t);
std::ostream &operator << (std::ostream &out, EggGroup::DartType t);
std::ostream &operator << (std::ostream &out, EggGroup::DCSType t);
std::ostream &operator << (std::ostream &out, EggGroup::BillboardType t);
std::ostream &operator << (std::ostream &out, EggGroup::CollisionSolidType t);
std::ostream &operator << (std::ostream &out, EggGroup::CollideFlags t);
std::ostream &operator << (std::ostream &out, EggGroup::BlendMode t);
std::ostream &operator << (std::ostream &out, EggGroup::BlendOperand t);


#include "eggGroup.I"

#endif
