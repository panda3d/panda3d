// Filename: lensFlareNode.h
// Created by:  jason (18Jul00)
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

#if 0  // temporarily disabled until we can port to new scene graph.

#ifndef LENSFLARENODE_H
#define LENSFLARENODE_H

#include "pandabase.h"

#include "namedNode.h"
#include "texture.h"
#include "luse.h"
#include "pta_Colorf.h"
#include "pta_float.h"
#include "renderRelation.h"
#include "pmap.h"

class GraphicsStateGuardian;
class ClockObject;

class EXPCL_PANDAFX LensFlareNode : public Node
{
PUBLISHED:
  INLINE LensFlareNode(void);

  void add_flare(PT(Texture) flare, PTA_float scales, PTA_float offsets,
                 PTA_float angle_scales, PTA_Colorf colors);
  void add_blind(PT(Texture) blind);

  INLINE void set_texel_scale(float texel_to_world);
  INLINE void set_global_scale(float scale);

  INLINE void set_blind_falloff(float fall_off);
  INLINE void set_flare_falloff(float fall_off);

  INLINE void set_light_source(PT_Node source);

public:
  virtual bool sub_render(const AllTransitionsWrapper &input_trans,
                          AllTransitionsWrapper &modify_trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

private:
  typedef pvector<PTA_float> vector_Vfloat;
  typedef pvector<PTA_Colorf> vector_Vcolorf;
  typedef pvector< PT(RenderRelation) > vector_relation;
  typedef pvector< PT(Texture) > vector_texture;

  vector_texture _flares;
  PT(Texture) _blind;

  vector_relation _flare_arcs;
  PT(RenderRelation) _blind_arc;

  vector_Vfloat _flare_scales;
  vector_Vfloat _flare_angle_scales;
  vector_Vfloat _flare_offsets;
  vector_Vcolorf _flare_colors;

  float _global_scale;
  float _texel_scale;

  float _blind_fall_off;
  float _flare_fall_off;

  PT_Node _light_node;

  /****Internal functions*****/

  //Sub-routines that are defined only for code cleanliness
  void prepare_flares(const LVector3f &delta, const LPoint3f &light, const float &angle);
  void prepare_blind(const float &angle, const float &tnear);

  //All of the geometry for halos and blooms is created the same way.
  //Sparkle geometry is created differently because we cycle through
  //the set of Sparkle textures to create an animation

  void set_geometry(GeomSprite *sprite, const PTA_float &geom_scales,
                    const PTA_float &geom_offsets, const PTA_float &geom_angle_scales,
                    const PTA_Colorf &geom_colors, const LVector3f &delta,
                    const LPoint3f &light, const float &angle);


  void render_child(RenderRelation *arc,
                    const AllTransitionsWrapper &trans,
                    GraphicsStateGuardian *gsg);
  void render_children(const vector_relation &arcs,
                       const AllTransitionsWrapper &trans,
                       GraphicsStateGuardian *gsg);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

  static TypedWritable *make_LensFlareNode(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  int _num_flares, _num_arcs;

public:
  static TypeHandle get_class_type( void ) {
      return _type_handle;
  }
  static void init_type( void ) {
    NamedNode::init_type();
    register_type( _type_handle, "LensFlareNode",
                NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type( void ) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle             _type_handle;
};

#include "lensFlareNode.I"

#endif

/***********OLD SPARKLE CODE.  LEFT IN CASE
public:
  void add_sparkle(PT_Node source, PT(Texture) sparkle);
  void set_sparkles_attributes(PT_Node source, float angle_scale, vector_float scales,
                               vector_float offsets, vector_Colorf colors);
  void set_sparkle_fps(float fps);

private:
  //Sparkle animation variables
  ClockObject* _global_clock;
  float _next_switch;
  float _sparkle_fps;
  float _inv_sparkle_fps;

  typedef pmap<PT_Node, vector_float> Sparkle_Scales;
  typedef pmap<PT_Node, vector_float> Sparkle_Offsets;
  typedef pmap<PT_Node, vector_Colorf> Sparkle_Colors;

  Textures _sparkles;
  Relations _sparkle_arcs;

  Sparkle_Scales _sparkle_scales;
  Sparkle_Offsets _sparkle_offsets;
  Sparkle_Colors _sparkle_colors;

  int _num_sparkles_on;
  pmap<PT_Node, int> _current_sparkles;

  void prepare_sparkles(vector_relation &arcs, const vector_texture &sparkles,
                        const vector_float &scales, const vector_float &offsets,
                        const vector_Colorf &colors, const LVector3f &delta,
                        const LPoint3f &light, const BoundingVolume &bound, int &old_sparkle);

  //Timing function to control sparkle animation
  int compute_current(int &current_sparkle, vector_texture sparkles);

******************/

#endif  // temporarily disabled until we can port to new scene graph.
