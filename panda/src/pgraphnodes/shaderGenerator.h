// Filename: shaderGenerator.h
// Created by: jyelon (15Dec07)
// Updated by: weifengh, PandaSE(15Apr10)
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

#ifndef SHADERGENERATOR_H
#define SHADERGENERATOR_H

#include "pandabase.h"
#include "typedReferenceCount.h"

#ifdef HAVE_CG

#include "graphicsStateGuardianBase.h"
#include "graphicsOutputBase.h"
#include "nodePath.h"
#include "shaderAttrib.h"
#include "renderState.h"
#include "renderAttrib.h"

class AmbientLight;
class DirectionalLight;
class PointLight;
class Spotlight;
class LightAttrib;
class GeomVertexAnimationSpec;

////////////////////////////////////////////////////////////////////
//       Class : ShaderGenerator
// Description : The ShaderGenerator is a device that effectively
//               replaces the classic fixed function pipeline with
//               a 'next-gen' fixed function pipeline.  The next-gen
//               fixed function pipeline supports features like
//               normal mapping, gloss mapping, cartoon lighting,
//               and so forth.  It works by automatically generating
//               a shader from a given RenderState.
//
//               Currently, there is one ShaderGenerator object per
//               GraphicsStateGuardian.  It is our intent that in
//               time, people will write classes that derive from
//               ShaderGenerator but which yield slightly different
//               results.
//
//               The ShaderGenerator owes its existence to the
//               'Bamboo Team' at Carnegie Mellon's Entertainment
//               Technology Center.  This is a group of students
//               who, as a semester project, decided that next-gen
//               graphics should be accessible to everyone, even if
//               they don't know shader programming.  The group
//               consisted of:
//
//               Aaron Lo, Programmer
//               Heegun Lee, Programmer
//               Erin Fernandez, Artist/Tester
//               Joe Grubb, Artist/Tester
//               Ivan Ortega, Technical Artist/Tester
//
//               Thanks to them!
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPHNODES ShaderGenerator : public TypedReferenceCount {
PUBLISHED:
  ShaderGenerator(GraphicsStateGuardianBase *gsg, GraphicsOutputBase *host);
  virtual ~ShaderGenerator();
  virtual CPT(ShaderAttrib) synthesize_shader(const RenderState *rs,
                                              const GeomVertexAnimationSpec &anim);

protected:
  CPT(RenderAttrib) create_shader_attrib(const string &txt);
  PT(Texture) update_shadow_buffer(NodePath light_np);
  static const string combine_mode_as_string(CPT(TextureStage) stage,
                      TextureStage::CombineMode c_mode, bool alpha, short texindex);
  static const string combine_source_as_string(CPT(TextureStage) stage,
                         short num, bool alpha, bool single_value, short texindex);
  static const string texture_type_as_string(Texture::TextureType ttype);

  // Shader register allocation:

  int _vcregs_used;
  int _fcregs_used;
  int _vtregs_used;
  int _ftregs_used;
  void reset_register_allocator();
  const char *alloc_vreg();
  const char *alloc_freg();

  // RenderState analysis information.  Created by analyze_renderstate:

  CPT(RenderState) _state;
  Material *_material;
  int _num_textures;

  pvector <AmbientLight *>     _alights;
  pvector <DirectionalLight *> _dlights;
  pvector <PointLight *>       _plights;
  pvector <Spotlight *>        _slights;
  pvector <NodePath>           _alights_np;
  pvector <NodePath>           _dlights_np;
  pvector <NodePath>           _plights_np;
  pvector <NodePath>           _slights_np;

  bool _vertex_colors;
  bool _flat_colors;

  bool _lighting;
  bool _shadows;
  bool _fog;

  bool _have_ambient;
  bool _have_diffuse;
  bool _have_emission;
  bool _have_specular;

  bool _separate_ambient_diffuse;

  int _map_index_normal;
  int _map_index_height;
  int _map_index_glow;
  int _map_index_gloss;
  bool _map_height_in_alpha;

  bool _out_primary_glow;
  bool _out_aux_normal;
  bool _out_aux_glow;
  bool _out_aux_any;

  bool _have_alpha_test;
  bool _have_alpha_blend;
  bool _calc_primary_alpha;
  bool _subsume_alpha_test;
  bool _disable_alpha_write;

  int _num_clip_planes;
  bool _use_shadow_filter;

  bool _need_material_props;
  bool _need_world_position;
  bool _need_world_normal;
  bool _need_eye_position;
  bool _need_eye_normal;
  bool _normalize_normals;
  bool _auto_normal_on;
  bool _auto_glow_on;
  bool _auto_gloss_on;
  bool _auto_ramp_on;
  bool _auto_shadow_on;

  void analyze_renderstate(const RenderState *rs);
  void clear_analysis();

  // This is not a PT() to prevent a circular reference.
  GraphicsStateGuardianBase *_gsg;
  GraphicsOutputBase *_host;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderGenerator",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#else

// If we don't have Cg, let's replace this with a stub.
class EXPCL_PANDA_PGRAPHNODES ShaderGenerator : public TypedReferenceCount {
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderGenerator",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderGenerator.I"

#endif  // HAVE_CG

#endif  // SHADERGENERATOR_H
