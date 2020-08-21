/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLerpNodePathInterval.h
 * @author drose
 * @date 2002-08-27
 */

#ifndef CLERPNODEPATHINTERVAL_H
#define CLERPNODEPATHINTERVAL_H

#include "directbase.h"
#include "cLerpInterval.h"
#include "nodePath.h"
#include "textureStage.h"

/**
 * An interval that lerps one or more properties (like pos, hpr, etc.) on a
 * NodePath over time.
 */
class EXPCL_DIRECT_INTERVAL CLerpNodePathInterval : public CLerpInterval {
PUBLISHED:
  explicit CLerpNodePathInterval(const std::string &name, double duration,
                                 BlendType blend_type, bool bake_in_start,
                                 bool fluid,
                                 const NodePath &node, const NodePath &other);

  INLINE const NodePath &get_node() const;
  INLINE const NodePath &get_other() const;

  INLINE void set_start_pos(const LVecBase3 &pos);
  INLINE void set_end_pos(const LVecBase3 &pos);
  INLINE void set_start_hpr(const LVecBase3 &hpr);
  INLINE void set_end_hpr(const LVecBase3 &hpr);
  INLINE void set_end_hpr(const LQuaternion &quat);
  INLINE void set_start_quat(const LQuaternion &quat);
  INLINE void set_end_quat(const LVecBase3 &hpr);
  INLINE void set_end_quat(const LQuaternion &quat);
  INLINE void set_start_scale(const LVecBase3 &scale);
  INLINE void set_start_scale(PN_stdfloat scale);
  INLINE void set_end_scale(const LVecBase3 &scale);
  INLINE void set_end_scale(PN_stdfloat scale);
  INLINE void set_start_shear(const LVecBase3 &shear);
  INLINE void set_end_shear(const LVecBase3 &shear);
  INLINE void set_start_color(const LVecBase4 &color);
  INLINE void set_end_color(const LVecBase4 &color);
  INLINE void set_start_color_scale(const LVecBase4 &color_scale);
  INLINE void set_end_color_scale(const LVecBase4 &color_scale);
  INLINE void set_texture_stage(TextureStage *stage);
  INLINE void set_start_tex_offset(const LVecBase2 &tex_offset);
  INLINE void set_end_tex_offset(const LVecBase2 &tex_offset);
  INLINE void set_start_tex_rotate(PN_stdfloat tex_rotate);
  INLINE void set_end_tex_rotate(PN_stdfloat tex_rotate);
  INLINE void set_start_tex_scale(const LVecBase2 &tex_scale);
  INLINE void set_end_tex_scale(const LVecBase2 &tex_scale);

  INLINE void set_override(int override);
  INLINE int get_override() const;

  virtual void priv_initialize(double t);
  virtual void priv_instant();
  virtual void priv_step(double t);
  virtual void priv_reverse_initialize(double t);
  virtual void priv_reverse_instant();

  virtual void output(std::ostream &out) const;

private:
  void setup_slerp();

  NodePath _node;
  NodePath _other;

  enum Flags {
    F_end_pos            = 0x00000001,
    F_end_hpr            = 0x00000002,
    F_end_quat           = 0x00000004,
    F_end_scale          = 0x00000008,
    F_end_color          = 0x00000010,
    F_end_color_scale    = 0x00000020,
    F_end_shear          = 0x00000040,
    F_end_tex_offset     = 0x00000080,
    F_end_tex_rotate     = 0x00000100,
    F_end_tex_scale      = 0x00000200,

    F_start_pos          = 0x00010000,
    F_start_hpr          = 0x00020000,
    F_start_quat         = 0x00040000,
    F_start_scale        = 0x00080000,
    F_start_color        = 0x00100000,
    F_start_color_scale  = 0x00200000,
    F_start_shear        = 0x00400000,
    F_start_tex_offset   = 0x00800000,
    F_start_tex_rotate   = 0x01000000,
    F_start_tex_scale    = 0x02000000,

    F_fluid              = 0x10000000,
    F_bake_in_start      = 0x20000000,

    F_slerp_setup        = 0x40000000,
  };

  unsigned int _flags;
  LPoint3 _start_pos, _end_pos;
  LVecBase3 _start_hpr, _end_hpr;
  LQuaternion _start_quat, _end_quat;
  LVecBase3 _start_scale, _end_scale;
  LVecBase3 _start_shear, _end_shear;
  LColor _start_color, _end_color;
  LVecBase4 _start_color_scale, _end_color_scale;
  PT(TextureStage) _texture_stage;
  LVecBase2 _start_tex_offset, _end_tex_offset;
  PN_stdfloat _start_tex_rotate, _end_tex_rotate;
  LVecBase2 _start_tex_scale, _end_tex_scale;

  int _override;
  double _prev_d;
  PN_stdfloat _slerp_angle;
  PN_stdfloat _slerp_denom;
  LQuaternion _slerp_c;

  void slerp_basic(LQuaternion &result, PN_stdfloat t) const;
  void slerp_angle_0(LQuaternion &result, PN_stdfloat t) const;
  void slerp_angle_180(LQuaternion &result, PN_stdfloat t) const;

  // Define a pointer to one of the above three methods.
  void (CLerpNodePathInterval::*_slerp)(LQuaternion &result, PN_stdfloat t) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CLerpInterval::init_type();
    register_type(_type_handle, "CLerpNodePathInterval",
                  CLerpInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cLerpNodePathInterval.I"

#endif
