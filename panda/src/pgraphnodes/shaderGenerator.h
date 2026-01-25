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
#include "graphicsStateGuardianBase.h"
#include "shaderAttrib.h"
#include "renderState.h"
#include "graphicsOutputBase.h"
#include "nodePath.h"
#include "renderAttrib.h"
#include "lightMutex.h"

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

  bool reload_cache();
  bool flush_cache();

  virtual CPT(ShaderAttrib) synthesize_shader(const RenderState *rs,
                                              const GeomVertexAnimationSpec &anim);

  void rehash_generated_shaders();
  void clear_generated_shaders();

protected:
  // Shader register allocation:
  int _ftregs_used;
  void reset_register_allocator();
  const char *alloc_freg();

  Filename _cache_filename;
  bool _use_pointcoord;
  bool _use_shadow_filter;
  int _num_indexed_transforms;

  // RenderState analysis information.  Created by analyze_renderstate:

  struct ShaderKey {
    ShaderKey();
    bool operator < (const ShaderKey &other) const;
    bool operator == (const ShaderKey &other) const;
    bool operator != (const ShaderKey &other) const { return !operator ==(other); }

    void read_datagram(DatagramIterator &source);
    void write_datagram(Datagram &dg) const;

    INLINE RenderAttrib::PandaCompareFunc get_alpha_test_mode() const;

    enum Flags {
      F_MATERIAL_FLAGS_SHIFT  = 0,
      F_MATERIAL_FLAGS_MASK   = 0x0007ff,

      F_FOG_MODE_SHIFT        = 11,
      F_FOG_MODE_MASK         = 0x001800,

      F_lighting              = 0x002000,
      F_have_separate_ambient = 0x004000,
      F_flat_color            = 0x008000,
      F_vertex_color          = 0x010000,
      F_calc_primary_alpha    = 0x020000,
      F_disable_alpha_write   = 0x040000,
      F_out_alpha_glow        = 0x080000,
      F_out_aux_normal        = 0x100000,
      F_out_aux_glow          = 0x200000,
      F_perspective_points    = 0x400000,
      F_indexed_transforms    = 0x800000,

      F_ALPHA_TEST_SHIFT      = 24,
      F_ALPHA_TEST_MASK       = 0x07000000,

      F_use_shadow_filter     = 0x08000000,
      F_disable_color_write   = 0x10000000,
    };

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
      TF_map_emission = 0x001000000,
      TF_map_occlusion = 0x002000000,
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

    int _flags;
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
      LF_type_directional,
      LF_type_spot,
      LF_type_point,
      LF_type_sphere,

      LF_TYPE_MASK = 3,

      LF_has_shadows = 4,
      LF_has_specular_color = 8,
    };

    pvector<int> _lights;

    PN_stdfloat _alpha_test_ref;

    int _num_clip_planes;

    LightRampAttrib::LightRampMode _light_ramp_mode;
    PN_stdfloat _light_ramp_level[2];
    PN_stdfloat _light_ramp_threshold[2];

    int _num_anim_transforms;
  };

  struct GeneratedShader {
    PT(Shader) _shader;
    CPT(ShaderAttrib) _attrib;
    time_t _last_use = 0;
  };

  LightMutex _lock;
  typedef phash_map<ShaderKey, GeneratedShader> GeneratedShaders;
  GeneratedShaders _generated_shaders;

  void analyze_renderstate(ShaderKey &key, const RenderState *rs);
  void analyze_renderstate(ShaderKey &key, const RenderState *rs,
                           const GeomVertexAnimationSpec &anim);

  static CPT(ShaderAttrib) make_attrib(const ShaderKey &key, Shader *shader);

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

#include "shaderGenerator.I"

#endif  // SHADERGENERATOR_H
