// Filename: seaPatchNode.h
// Created by:  sshodhan (18Jun04)
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

#ifndef SEAPATCHNODE_H
#define SEAPATCHNODE_H
#include "pandabase.h"
#include "luse.h"
#include "nodePath.h"
#include "pmap.h"
#include "notify.h"
#include "pandaNode.h"
#include "geom.h"
#include "geomNode.h"
#include "pmap.h"
#include "lvecBase3.h"

static const int noise_table_size = 64;

////////////////////////////////////////////////////////////////////
//       Class : SeaPatchNode
// Description : A SeaPatchNode
//               Modeled after the old DQ SeaPatchNode
//               Any Geometry parented to a SeaPatchNode will have
//               - waves
//               - UV noise and transforms
//               - vertex color and height based shading
// Looking to work on:
// - multiple sine waves at different resolutions (for base waves
// and detail waves)
// - independant control over multiple layers parented to the same
//   SeaPatchNode (or should this just be done through multiple 
//   copies of the SeaPatchNode? Need to make copy constructor and =
// - perturbing normals (bump mapping, pixel level stuff?)
// - wakes, decals
// - damping floating objects
// - reflecting things like a flaming ship, torches, sun, moon
// - tiling to make ocean
// - offload UV manipulation to texture matrix?
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SeaPatchNode : public PandaNode{

PUBLISHED:

  enum Alpha_Type {
    AHIGH,
    ALOW,
    ATOTAL,
    ANONE,
  };

  SeaPatchNode(const string &name);

  INLINE void enable();
  INLINE void disable();
  INLINE bool is_enabled() const;

  // Comparison methods
  INLINE bool operator == (const SeaPatchNode &other) const;
  INLINE bool operator != (const SeaPatchNode &other) const;
  INLINE bool operator < (const SeaPatchNode &other) const;
  int compare_to(const SeaPatchNode &other) const;

  INLINE void set_amp(float amplitude);
  INLINE void set_freq(float frequency);
  INLINE float get_amp() const;
  INLINE float get_freq() const;
  INLINE LVecBase2f get_wavelength() const;
  INLINE void set_wavelength(LVecBase2f w);
  INLINE void set_passive_move(LVecBase2f UV);
  INLINE LVecBase2f get_passive_move() const;
  INLINE LPoint3f get_center() const;
  INLINE float get_radius() const;
  INLINE float get_threshold() const;
  INLINE void set_center(LPoint3f center);
  INLINE void set_radius(float r);
  INLINE void set_threshold(float t);
  INLINE void set_noise_amp(float x);
  INLINE float get_noise_amp() const;
  INLINE void enable_noise_uv();
  INLINE void disable_noise_uv();
  INLINE bool is_noise_on_uv() const;
  INLINE void set_noise_f(float f);
  INLINE float get_noise_f();
  INLINE float Weight(float t) ;
  INLINE int Mod(int quotient, int divisor);
  INLINE void set_xsize(float xs);
  INLINE void set_ysize(float ys);
  INLINE float get_xsize() const;
  INLINE float get_ysize() const;
  INLINE void set_u_scale(float us);
  INLINE void set_v_scale(float vs);
  INLINE float get_u_scale() const;
  INLINE float get_v_scale() const;
  INLINE float get_height(float x, float y) const;
  INLINE void set_cscale(float cs);
  INLINE void set_ascale(float as);
  INLINE float get_cscale() const;
  INLINE float get_ascale() const;
  INLINE LVecBase3f get_normal(float x, float y) const;
  INLINE void set_light_color(Colorf c);
  INLINE Colorf get_light_color() const;
  INLINE float get_u_sin_scale() const;
  INLINE float get_v_sin_scale() const;
  INLINE void set_u_sin_scale(float s);
  INLINE void set_v_sin_scale(float s);
  void set_alpha_mode(Alpha_Type a);
  

private:
  bool _enabled;
  float _wave_movement;
  float _amplitude;
  float _frequency;
  LVecBase2f _wavelength;
  LVecBase2f _passive_move;
  LVecBase2f _move_variable;
  LPoint3f _center;
  float _radius;
  float _noise_amp;
  bool _noise_on_uv;
  float _noise_f;
  int _perm_tab_u[noise_table_size];
  LVecBase3f _grad_tab_u[noise_table_size];
  int _perm_tab_v[noise_table_size];
  LVecBase3f _grad_tab_v[noise_table_size];
  int _noise_detail;
  float _xsize;
  float _ysize;
  float _u_scale;
  float _v_scale;
  float _color_scale;
  float _alpha_scale;
  Colorf _light_color;
  float _u_sin_scale;
  float _v_sin_scale;
  float _threshold;
  Alpha_Type _alpha_mode;
  
public:
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  void recurse_children(PandaNode *node, LMatrix4f mat);
  void do_wave(Geom *geom, LMatrix4f mat);
  void noise_init();
  float plain_noise(float x, float y, float z, unsigned int s, int uvi );
  float do_noise(float x, float y, float t, int uvi);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "SeaPatchNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "seaPatchNode.I"

#endif


