// Filename: cLerpNodePathInterval.h
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

#ifndef CLERPNODEPATHINTERVAL_H
#define CLERPNODEPATHINTERVAL_H

#include "directbase.h"
#include "cLerpInterval.h"
#include "nodePath.h"
#include "textureStage.h"

////////////////////////////////////////////////////////////////////
//       Class : CLerpNodePathInterval
// Description : An interval that lerps one or more properties (like
//               pos, hpr, etc.) on a NodePath over time.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CLerpNodePathInterval : public CLerpInterval {
PUBLISHED:
  CLerpNodePathInterval(const string &name, double duration, 
                        BlendType blend_type, bool bake_in_start,
                        bool fluid,
                        const NodePath &node, const NodePath &other);

  INLINE const NodePath &get_node() const;
  INLINE const NodePath &get_other() const;

  INLINE void set_start_pos(const LVecBase3f &pos);
  INLINE void set_end_pos(const LVecBase3f &pos);
  INLINE void set_start_hpr(const LVecBase3f &hpr);
  INLINE void set_end_hpr(const LVecBase3f &hpr);
  INLINE void set_end_hpr(const LQuaternionf &quat);
  INLINE void set_start_quat(const LQuaternionf &quat);
  INLINE void set_end_quat(const LVecBase3f &hpr);
  INLINE void set_end_quat(const LQuaternionf &quat);
  INLINE void set_start_scale(const LVecBase3f &scale);
  INLINE void set_start_scale(float scale);
  INLINE void set_end_scale(const LVecBase3f &scale);
  INLINE void set_end_scale(float scale);
  INLINE void set_start_shear(const LVecBase3f &shear);
  INLINE void set_end_shear(const LVecBase3f &shear);
  INLINE void set_start_color(const LVecBase4f &color);
  INLINE void set_end_color(const LVecBase4f &color);
  INLINE void set_start_color_scale(const LVecBase4f &color_scale);
  INLINE void set_end_color_scale(const LVecBase4f &color_scale);
  INLINE void set_texture_stage(TextureStage *stage);
  INLINE void set_start_tex_offset(const LVecBase2f &tex_offset);
  INLINE void set_end_tex_offset(const LVecBase2f &tex_offset);
  INLINE void set_start_tex_rotate(float tex_rotate);
  INLINE void set_end_tex_rotate(float tex_rotate);
  INLINE void set_start_tex_scale(const LVecBase2f &tex_scale);
  INLINE void set_end_tex_scale(const LVecBase2f &tex_scale);

  INLINE void set_override(int override);
  INLINE int get_override() const;

  virtual void priv_initialize(double t);
  virtual void priv_instant();
  virtual void priv_step(double t);
  virtual void priv_reverse_initialize(double t);
  virtual void priv_reverse_instant();

  virtual void output(ostream &out) const;

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
  LPoint3f _start_pos, _end_pos;
  LVecBase3f _start_hpr, _end_hpr;
  LQuaternionf _start_quat, _end_quat;
  LVecBase3f _start_scale, _end_scale;
  LVecBase3f _start_shear, _end_shear;
  Colorf _start_color, _end_color;
  LVecBase4f _start_color_scale, _end_color_scale;
  PT(TextureStage) _texture_stage;
  LVecBase2f _start_tex_offset, _end_tex_offset;
  float _start_tex_rotate, _end_tex_rotate;
  LVecBase2f _start_tex_scale, _end_tex_scale;

  int _override;
  double _prev_d;
  float _slerp_angle;
  float _slerp_denom;
  LQuaternionf _slerp_c;

  void slerp_basic(LQuaternionf &result, float t) const;
  void slerp_angle_0(LQuaternionf &result, float t) const;
  void slerp_angle_180(LQuaternionf &result, float t) const;

  // Define a pointer to one of the above three methods.
  void (CLerpNodePathInterval::*_slerp)(LQuaternionf &result, float t) const;
  
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

