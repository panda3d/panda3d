/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderGenerator.h
 * @author jyelon
 * @date 2007-12-15
 * @author weifengh, PandaSE
 * @date 2010-04-15
 */

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

#include "colorAttrib.h"
#include "lightRampAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"

class AmbientLight;
class DirectionalLight;
class PointLight;
class Spotlight;
class LightAttrib;
class GeomVertexAnimationSpec;

/**
 * The ShaderGenerator is a device that effectively replaces the classic fixed
 * function pipeline with a 'next-gen' fixed function pipeline.  The next-gen
 * fixed function pipeline supports features like normal mapping, gloss
 * mapping, cartoon lighting, and so forth.  It works by automatically
 * generating a shader from a given RenderState.
 *
 * Currently, there is one ShaderGenerator object per GraphicsStateGuardian.
 * It is our intent that in time, people will write classes that derive from
 * ShaderGenerator but which yield slightly different results.
 *
 * The ShaderGenerator owes its existence to the 'Bamboo Team' at Carnegie
 * Mellon's Entertainment Technology Center.  This is a group of students who,
 * as a semester project, decided that next-gen graphics should be accessible
 * to everyone, even if they don't know shader programming.  The group
 * consisted of:
 *
 * Aaron Lo, Programmer Heegun Lee, Programmer Erin Fernandez, Artist/Tester
 * Joe Grubb, Artist/Tester Ivan Ortega, Technical Artist/Tester
 *
 * Thanks to them!
 *
 */
class EXPCL_PANDA_PGRAPHNODES ShaderGenerator : public TypedReferenceCount {
PUBLISHED:
  ShaderGenerator(const GraphicsStateGuardianBase *gsg);
  virtual ~ShaderGenerator();
  virtual CPT(ShaderAttrib) synthesize_shader(const RenderState *rs,
                                              const GeomVertexAnimationSpec &anim);

  void rehash_generated_shaders();
  void clear_generated_shaders();

protected:
  // Shader register allocation:

  bool _use_generic_attr;
  int _vcregs_used;
  int _fcregs_used;
  int _vtregs_used;
  int _ftregs_used;
  void reset_register_allocator();
  const char *alloc_vreg();
  const char *alloc_freg();

  bool _use_shadow_filter;

  // RenderState analysis information.  Created by analyze_renderstate:

  CPT(RenderState) _state;
  struct ShaderKey {
    ShaderKey();
    bool operator < (const ShaderKey &other) const;
    bool operator == (const ShaderKey &other) const;
    bool operator != (const ShaderKey &other) const { return !operator ==(other); }

    GeomVertexAnimationSpec _anim_spec;
    enum TextureFlags {
      TF_has_rgb      = 0x001,
      TF_has_alpha    = 0x002,
      TF_has_texscale = 0x004,
      TF_has_texmat   = 0x008,
      TF_saved_result = 0x010,
      TF_map_normal   = 0x020,
      TF_map_height   = 0x040,
      TF_map_glow     = 0x080,
      TF_map_gloss    = 0x100,
      TF_uses_color   = 0x200,
      TF_uses_primary_color = 0x400,
      TF_uses_last_saved_result = 0x800,

      TF_rgb_scale_2 = 0x1000,
      TF_rgb_scale_4 = 0x2000,
      TF_alpha_scale_2 = 0x4000,
      TF_alpha_scale_4 = 0x8000,

      TF_COMBINE_RGB_MODE_SHIFT = 16,
      TF_COMBINE_RGB_MODE_MASK = 0x0000f0000,
      TF_COMBINE_ALPHA_MODE_SHIFT = 20,
      TF_COMBINE_ALPHA_MODE_MASK = 0x000f00000,
    };

    ColorAttrib::Type _color_type;
    int _material_flags;
    int _texture_flags;

    struct TextureInfo {
      CPT_InternalName _texcoord_name;
      Texture::TextureType _type;
      TextureStage::Mode _mode;
      TexGenAttrib::Mode _gen_mode;
      int _flags;
      uint16_t _combine_rgb;
      uint16_t _combine_alpha;
    };
    pvector<TextureInfo> _textures;

    enum LightFlags {
      LF_has_shadows = 1,
      LF_has_specular_color = 2,
    };

    struct LightInfo {
      TypeHandle _type;
      int _flags;
    };
    pvector<LightInfo> _lights;
    bool _lighting;
    bool _have_separate_ambient;

    int _fog_mode;

    int _outputs;
    bool _calc_primary_alpha;
    bool _disable_alpha_write;
    RenderAttrib::PandaCompareFunc _alpha_test_mode;
    PN_stdfloat _alpha_test_ref;

    int _num_clip_planes;

    CPT(LightRampAttrib) _light_ramp;
  };

  typedef phash_map<ShaderKey, CPT(ShaderAttrib)> GeneratedShaders;
  GeneratedShaders _generated_shaders;

  void analyze_renderstate(ShaderKey &key, const RenderState *rs);

  static std::string combine_mode_as_string(const ShaderKey::TextureInfo &info,
                      TextureStage::CombineMode c_mode, bool alpha, short texindex);
  static std::string combine_source_as_string(const ShaderKey::TextureInfo &info,
                                         short num, bool alpha, short texindex);
  static const char *texture_type_as_string(Texture::TextureType ttype);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ShaderGenerator",
                  TypedReferenceCount::get_class_type());
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
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ShaderGenerator",
                  TypedReferenceCount::get_class_type());
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
