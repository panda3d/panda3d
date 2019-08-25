/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderGenerator.cxx
 * @author jyelon
 * @date 2007-12-15
 * @author weifengh, PandaSE
 * @date 2010-04-15
 * @author agartner, PandaSE
 * @date 2010-04-16
 * TextureStage::M_modulate (before this, separate textures formatted as
 * alpha wiped color off resulting rgb)
 */

#include "shaderGenerator.h"

#include "renderState.h"
#include "shaderAttrib.h"
#include "auxBitplaneAttrib.h"
#include "alphaTestAttrib.h"
#include "colorBlendAttrib.h"
#include "transparencyAttrib.h"
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "lightAttrib.h"
#include "materialAttrib.h"
#include "lightRampAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "colorScaleAttrib.h"
#include "clipPlaneAttrib.h"
#include "fogAttrib.h"
#include "texture.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "rescaleNormalAttrib.h"
#include "pointLight.h"
#include "sphereLight.h"
#include "spotlight.h"
#include "lightLensNode.h"
#include "lvector4.h"
#include "config_pgraphnodes.h"
#include "pStatTimer.h"

using std::string;

TypeHandle ShaderGenerator::_type_handle;

#ifdef HAVE_CG

#define UNPACK_COMBINE_SRC(from, n) (TextureStage::CombineSource)((from >> ((uint16_t)n * 5u)) & 7u)
#define UNPACK_COMBINE_OP(from, n) (TextureStage::CombineOperand)(((from >> (((uint16_t)n * 5u) + 3u)) & 3u) + 1u)

static inline uint16_t
pack_combine(TextureStage::CombineSource src0, TextureStage::CombineOperand op0,
             TextureStage::CombineSource src1, TextureStage::CombineOperand op1,
             TextureStage::CombineSource src2, TextureStage::CombineOperand op2) {
  if (op0 == TextureStage::CO_undefined) op0 = TextureStage::CO_src_alpha;
  if (op1 == TextureStage::CO_undefined) op1 = TextureStage::CO_src_alpha;
  if (op2 == TextureStage::CO_undefined) op2 = TextureStage::CO_src_alpha;

  return ((uint16_t)src0) | ((((uint16_t)op0 - 1u) & 3u) << 3u) |
         ((uint16_t)src1 << 5u) | ((((uint16_t)op1 - 1u) & 3u) << 8u) |
         ((uint16_t)src2 << 10u) | ((((uint16_t)op2 - 1u) & 3u) << 13u);
}

static PStatCollector lookup_collector("*:Munge:ShaderGen:Lookup");
static PStatCollector synthesize_collector("*:Munge:ShaderGen:Synthesize");

/**
 * Create a ShaderGenerator.  This has no state, except possibly to cache
 * certain results.  The parameter that must be passed is the GSG to which the
 * shader generator belongs.
 */
ShaderGenerator::
ShaderGenerator(const GraphicsStateGuardianBase *gsg) {
  // The ATTR# input semantics seem to map to generic vertex attributes in
  // both arbvp1 and glslv, which behave more consistently.  However, they
  // don't exist in Direct3D 9.  Use this silly little check for now.
#ifdef _WIN32
  _use_generic_attr = !gsg->get_supports_hlsl();
#else
  _use_generic_attr = true;
#endif

  // Do we want to use the ARB_shadow extension?  This also allows us to use
  // hardware shadows PCF.
  _use_shadow_filter = gsg->get_supports_shadow_filter();
}

/**
 * Destroy a ShaderGenerator.
 */
ShaderGenerator::
~ShaderGenerator() {
}

/**
 * Clears the register allocator.  Initially, the pool of available registers
 * is empty.  You have to add some if you want there to be any.
 */
void ShaderGenerator::
reset_register_allocator() {
  _vtregs_used = 0;
  _vcregs_used = 0;
  _ftregs_used = 0;
  _fcregs_used = 0;
}

/**
 * Allocate a vreg.
 */
const char *ShaderGenerator::
alloc_vreg() {
  if (_use_generic_attr) {
    // OpenGL case.
    switch (_vtregs_used) {
    case  0: _vtregs_used += 1; return "ATTR8";
    case  1: _vtregs_used += 1; return "ATTR9";
    case  2: _vtregs_used += 1; return "ATTR10";
    case  3: _vtregs_used += 1; return "ATTR11";
    case  4: _vtregs_used += 1; return "ATTR12";
    case  5: _vtregs_used += 1; return "ATTR13";
    case  6: _vtregs_used += 1; return "ATTR14";
    case  7: _vtregs_used += 1; return "ATTR15";
    }
    switch (_vcregs_used) {
    case  0: _vcregs_used += 1; return "ATTR3";
    case  1: _vcregs_used += 1; return "ATTR4";
    case  2: _vcregs_used += 1; return "ATTR5";
    case  3: _vcregs_used += 1; return "ATTR6";
    case  4: _vcregs_used += 1; return "ATTR7";
    case  5: _vcregs_used += 1; return "ATTR1";
    }
  } else {
    // DirectX 9 case.
    switch (_vtregs_used) {
    case  0: _vtregs_used += 1; return "TEXCOORD0";
    case  1: _vtregs_used += 1; return "TEXCOORD1";
    case  2: _vtregs_used += 1; return "TEXCOORD2";
    case  3: _vtregs_used += 1; return "TEXCOORD3";
    case  4: _vtregs_used += 1; return "TEXCOORD4";
    case  5: _vtregs_used += 1; return "TEXCOORD5";
    case  6: _vtregs_used += 1; return "TEXCOORD6";
    case  7: _vtregs_used += 1; return "TEXCOORD7";
    }
    switch (_vcregs_used) {
    case  0: _vcregs_used += 1; return "COLOR0";
    case  1: _vcregs_used += 1; return "COLOR1";
    }
  }
  // These don't exist in arbvp1, though they're reportedly supported by other
  // profiles.
  switch (_vtregs_used) {
  case  8: _vtregs_used += 1; return "TEXCOORD8";
  case  9: _vtregs_used += 1; return "TEXCOORD9";
  case 10: _vtregs_used += 1; return "TEXCOORD10";
  case 11: _vtregs_used += 1; return "TEXCOORD11";
  case 12: _vtregs_used += 1; return "TEXCOORD12";
  case 13: _vtregs_used += 1; return "TEXCOORD13";
  case 14: _vtregs_used += 1; return "TEXCOORD14";
  case 15: _vtregs_used += 1; return "TEXCOORD15";
  }
  return "UNKNOWN";
}

/**
 * Allocate a freg.
 */
const char *ShaderGenerator::
alloc_freg() {
  switch (_ftregs_used) {
  case  0: _ftregs_used += 1; return "TEXCOORD0";
  case  1: _ftregs_used += 1; return "TEXCOORD1";
  case  2: _ftregs_used += 1; return "TEXCOORD2";
  case  3: _ftregs_used += 1; return "TEXCOORD3";
  case  4: _ftregs_used += 1; return "TEXCOORD4";
  case  5: _ftregs_used += 1; return "TEXCOORD5";
  case  6: _ftregs_used += 1; return "TEXCOORD6";
  case  7: _ftregs_used += 1; return "TEXCOORD7";
  }
  // NB. We really shouldn't use the COLOR fregs, since the clamping can have
  // unexpected side-effects.
  switch (_ftregs_used) {
  case  8: _ftregs_used += 1; return "TEXCOORD8";
  case  9: _ftregs_used += 1; return "TEXCOORD9";
  case 10: _ftregs_used += 1; return "TEXCOORD10";
  case 11: _ftregs_used += 1; return "TEXCOORD11";
  case 12: _ftregs_used += 1; return "TEXCOORD12";
  case 13: _ftregs_used += 1; return "TEXCOORD13";
  case 14: _ftregs_used += 1; return "TEXCOORD14";
  case 15: _ftregs_used += 1; return "TEXCOORD15";
  }
  return "UNKNOWN";
}

/**
 * Analyzes the RenderState prior to shader generation.  The results of the
 * analysis are stored in instance variables of the Shader Generator.
 */
void ShaderGenerator::
analyze_renderstate(ShaderKey &key, const RenderState *rs) {
  const ShaderAttrib *shader_attrib;
  rs->get_attrib_def(shader_attrib);
  nassertv(shader_attrib->auto_shader());

  // verify_enforce_attrib_lock();
  const AuxBitplaneAttrib *aux_bitplane;
  rs->get_attrib_def(aux_bitplane);
  key._outputs = aux_bitplane->get_outputs();

  // Decide whether or not we need alpha testing or alpha blending.
  bool have_alpha_test = false;
  bool have_alpha_blend = false;
  const AlphaTestAttrib *alpha_test;
  rs->get_attrib_def(alpha_test);
  if (alpha_test->get_mode() != RenderAttrib::M_none &&
      alpha_test->get_mode() != RenderAttrib::M_always) {
    have_alpha_test = true;
  }
  const ColorBlendAttrib *color_blend;
  rs->get_attrib_def(color_blend);
  if (color_blend->get_mode() != ColorBlendAttrib::M_none) {
    have_alpha_blend = true;
  }
  const TransparencyAttrib *transparency;
  rs->get_attrib_def(transparency);
  if (transparency->get_mode() == TransparencyAttrib::M_alpha ||
      transparency->get_mode() == TransparencyAttrib::M_premultiplied_alpha ||
      transparency->get_mode() == TransparencyAttrib::M_dual) {
    have_alpha_blend = true;
  }

  // Decide what to send to the framebuffer alpha, if anything.
  if (key._outputs & AuxBitplaneAttrib::ABO_glow) {
    if (have_alpha_blend) {
      key._outputs &= ~AuxBitplaneAttrib::ABO_glow;
      key._disable_alpha_write = true;
    } else if (have_alpha_test) {
      // Subsume the alpha test in our shader.
      key._alpha_test_mode = alpha_test->get_mode();
      key._alpha_test_ref = alpha_test->get_reference_alpha();
    }
  }

  if (have_alpha_blend || have_alpha_test) {
    key._calc_primary_alpha = true;
  }

  // Determine whether or not vertex colors or flat colors are present.
  const ColorAttrib *color;
  rs->get_attrib_def(color);
  key._color_type = color->get_color_type();

  // Store the material flags (not the material values itself).
  const MaterialAttrib *material;
  rs->get_attrib_def(material);
  Material *mat = material->get_material();
  if (mat != nullptr) {
    // The next time the Material flags change, the Material should cause the
    // states to be rehashed.
    mat->mark_used_by_auto_shader();
    key._material_flags = mat->get_flags();

    if ((key._material_flags & Material::F_base_color) != 0) {
      key._material_flags |= (Material::F_diffuse | Material::F_specular | Material::F_ambient);
      key._material_flags &= ~Material::F_base_color;
    }
  }

  // Break out the lights by type.
  const LightAttrib *la;
  rs->get_attrib_def(la);
  bool have_ambient = false;

  for (size_t i = 0; i < la->get_num_on_lights(); ++i) {
    NodePath np = la->get_on_light(i);
    nassertv(!np.is_empty());
    PandaNode *node = np.node();
    nassertv(node != nullptr);

    if (node->is_ambient_light()) {
      have_ambient = true;
      key._lighting = true;
    } else {
      ShaderKey::LightInfo info;
      info._type = node->get_type();
      info._flags = 0;

      if (node->is_of_type(LightLensNode::get_class_type())) {
        const LightLensNode *llnode = (const LightLensNode *)node;
        if (shader_attrib->auto_shadow_on()) {
          if (llnode->is_shadow_caster()) {
            info._flags |= ShaderKey::LF_has_shadows;
          }

          // Make sure that the next time the shadows are toggled on this
          // light, it triggers a state rehash.
          llnode->mark_used_by_auto_shader();
        }
        if (llnode->has_specular_color()) {
          info._flags |= ShaderKey::LF_has_specular_color;
        }
      }

      key._lights.push_back(info);
      key._lighting = true;
    }
  }

  // See if there is a normal map, height map, gloss map, or glow map.  Also
  // check if anything has TexGen.

  const TextureAttrib *texture;
  rs->get_attrib_def(texture);
  const TexGenAttrib *tex_gen;
  rs->get_attrib_def(tex_gen);
  const TexMatrixAttrib *tex_matrix;
  rs->get_attrib_def(tex_matrix);

  size_t num_textures = texture->get_num_on_stages();
  for (size_t i = 0; i < num_textures; ++i) {
    TextureStage *stage = texture->get_on_stage(i);
    Texture *tex = texture->get_on_texture(stage);
    nassertd(tex != nullptr) continue;

    // Mark this TextureStage as having been used by the shader generator, so
    // that the next time its properties change, it will cause the state to be
    // rehashed to ensure that the shader is regenerated if needed.
    stage->mark_used_by_auto_shader();

    ShaderKey::TextureInfo info;
    info._type = tex->get_texture_type();
    info._mode = stage->get_mode();
    info._flags = 0;
    info._combine_rgb = 0u;
    info._combine_alpha = 0u;

    // While we look at the mode, determine whether we need to change the mode
    // in order to reflect disabled features.
    switch (info._mode) {
    case TextureStage::M_modulate:
      {
        Texture::Format format = tex->get_format();
        if (format != Texture::F_alpha) {
          info._flags |= ShaderKey::TF_has_rgb;
        }
        if (Texture::has_alpha(format)) {
          info._flags |= ShaderKey::TF_has_alpha;
        }
      }
      break;

    case TextureStage::M_modulate_glow:
      if (shader_attrib->auto_glow_on()) {
        info._flags = ShaderKey::TF_map_glow;
      } else {
        info._mode = TextureStage::M_modulate;
        info._flags = ShaderKey::TF_has_rgb;
      }
      break;

    case TextureStage::M_modulate_gloss:
      if (shader_attrib->auto_gloss_on()) {
        info._flags = ShaderKey::TF_map_gloss;
      } else {
        info._mode = TextureStage::M_modulate;
        info._flags = ShaderKey::TF_has_rgb;
      }
      break;

    case TextureStage::M_normal_height:
      if (parallax_mapping_samples == 0) {
        info._mode = TextureStage::M_normal;
      } else if (!shader_attrib->auto_normal_on() ||
                 (key._lights.empty() && (key._outputs & AuxBitplaneAttrib::ABO_aux_normal) == 0)) {
        info._mode = TextureStage::M_height;
        info._flags = ShaderKey::TF_has_alpha;
      } else {
        info._flags = ShaderKey::TF_map_normal | ShaderKey::TF_map_height;
      }
      break;

    case TextureStage::M_normal_gloss:
      if (!shader_attrib->auto_gloss_on() || key._lights.empty()) {
        info._mode = TextureStage::M_normal;
      } else if (!shader_attrib->auto_normal_on()) {
        info._mode = TextureStage::M_gloss;
      } else {
        info._flags = ShaderKey::TF_map_normal | ShaderKey::TF_map_gloss;
      }
      break;

    case TextureStage::M_combine:
      // If we have this rare, special mode, we encode all these extra
      // parameters as flags to prevent bloating the shader key.
      info._flags |= (uint32_t)stage->get_combine_rgb_mode() << ShaderKey::TF_COMBINE_RGB_MODE_SHIFT;
      info._flags |= (uint32_t)stage->get_combine_alpha_mode() << ShaderKey::TF_COMBINE_ALPHA_MODE_SHIFT;
      if (stage->get_rgb_scale() == 2) {
        info._flags |= ShaderKey::TF_rgb_scale_2;
      }
      if (stage->get_rgb_scale() == 4) {
        info._flags |= ShaderKey::TF_rgb_scale_4;
      }
      if (stage->get_alpha_scale() == 2) {
        info._flags |= ShaderKey::TF_alpha_scale_2;
      }
      if (stage->get_alpha_scale() == 4) {
        info._flags |= ShaderKey::TF_alpha_scale_4;
      }

      info._combine_rgb = pack_combine(
        stage->get_combine_rgb_source0(), stage->get_combine_rgb_operand0(),
        stage->get_combine_rgb_source1(), stage->get_combine_rgb_operand1(),
        stage->get_combine_rgb_source2(), stage->get_combine_rgb_operand2());
      info._combine_alpha = pack_combine(
        stage->get_combine_alpha_source0(), stage->get_combine_alpha_operand0(),
        stage->get_combine_alpha_source1(), stage->get_combine_alpha_operand1(),
        stage->get_combine_alpha_source2(), stage->get_combine_alpha_operand2());

      if (stage->uses_primary_color()) {
        info._flags |= ShaderKey::TF_uses_primary_color;
      }
      if (stage->uses_last_saved_result()) {
        info._flags |= ShaderKey::TF_uses_last_saved_result;
      }
      break;

    default:
      break;
    }

    // In fact, perhaps this stage should be disabled altogether?
    bool skip = false;
    switch (info._mode) {
    case TextureStage::M_normal:
      if (!shader_attrib->auto_normal_on() ||
          (key._lights.empty() && (key._outputs & AuxBitplaneAttrib::ABO_aux_normal) == 0)) {
        skip = true;
      } else {
        info._flags = ShaderKey::TF_map_normal;
      }
      break;
    case TextureStage::M_glow:
      if (shader_attrib->auto_glow_on()) {
        info._flags = ShaderKey::TF_map_glow;
      } else {
        skip = true;
      }
      break;
    case TextureStage::M_gloss:
      if (!key._lights.empty() && shader_attrib->auto_gloss_on()) {
        info._flags = ShaderKey::TF_map_gloss;
      } else {
        skip = true;
      }
      break;
    case TextureStage::M_height:
      if (parallax_mapping_samples > 0) {
        info._flags = ShaderKey::TF_map_height;
      } else {
        skip = true;
      }
      break;
    default:
      break;
    }
    // We can't just drop a disabled slot from the list, since then the
    // indices for the texture stages will no longer match up.  So we keep it,
    // but set it to a noop state to indicate that it should be skipped.
    if (skip) {
      info._type = Texture::TT_1d_texture;
      info._mode = TextureStage::M_modulate;
      info._flags = 0;
      key._textures.push_back(info);
      continue;
    }

    // Check if this state has a texture matrix to transform the texture
    // coordinates.
    if (tex_matrix->has_stage(stage)) {
      CPT(TransformState) transform = tex_matrix->get_transform(stage);
      if (!transform->is_identity()) {
        // Optimize for common case: if we only have a scale component, we
        // can get away with fewer shader inputs and operations.
        if (transform->has_components() && !transform->has_nonzero_shear() &&
            transform->get_pos() == LPoint3::zero() &&
            transform->get_hpr() == LVecBase3::zero()) {
          info._flags |= ShaderKey::TF_has_texscale;
        } else {
          info._flags |= ShaderKey::TF_has_texmat;
        }
      }
    }

    if (tex_gen->has_stage(stage)) {
      info._texcoord_name = nullptr;
      info._gen_mode = tex_gen->get_mode(stage);
    } else {
      info._texcoord_name = stage->get_texcoord_name();
      info._gen_mode = TexGenAttrib::M_off;
    }

    // Does this stage require saving its result?
    if (stage->get_saved_result()) {
      info._flags |= ShaderKey::TF_saved_result;
    }

    // Does this stage need a texcolor_# input?
    if (stage->uses_color()) {
      info._flags |= ShaderKey::TF_uses_color;
    }

    key._textures.push_back(info);
    key._texture_flags |= info._flags;
  }

  // Does nothing use the saved result?  If so, don't bother saving it.
  if ((key._texture_flags & ShaderKey::TF_uses_last_saved_result) == 0 &&
      (key._texture_flags & ShaderKey::TF_saved_result) != 0) {

    pvector<ShaderKey::TextureInfo>::iterator it;
    for (it = key._textures.begin(); it != key._textures.end(); ++it) {
      (*it)._flags &= ~ShaderKey::TF_saved_result;
    }
    key._texture_flags &= ~ShaderKey::TF_saved_result;
  }

  // Decide whether to separate ambient and diffuse calculations.
  if (have_ambient) {
    if (key._material_flags & Material::F_ambient) {
      key._have_separate_ambient = true;
    } else {
      if (key._material_flags & Material::F_diffuse) {
        key._have_separate_ambient = true;
      } else {
        key._have_separate_ambient = false;
      }
    }
  }

  if (shader_attrib->auto_ramp_on()) {
    const LightRampAttrib *light_ramp;
    if (rs->get_attrib(light_ramp)) {
      key._light_ramp = light_ramp;
      if (key._lighting) {
        key._have_separate_ambient = true;
      }
    }
  }

  // Check for clip planes.
  const ClipPlaneAttrib *clip_plane;
  rs->get_attrib_def(clip_plane);
  key._num_clip_planes = clip_plane->get_num_on_planes();

  // Check for fog.
  const FogAttrib *fog;
  if (rs->get_attrib(fog) && !fog->is_off()) {
    key._fog_mode = (int)fog->get_fog()->get_mode() + 1;
  }
}

/**
 * Rehashes all the states with generated shaders, removing the ones that are
 * no longer fresh.
 *
 * Call this if certain state has changed in such a way as to require a rerun
 * of the shader generator.  This should be rare because in most cases, the
 * shader generator will automatically regenerate shaders as necessary.
 *
 * @since 1.10.0
 */
void ShaderGenerator::
rehash_generated_shaders() {
  LightReMutexHolder holder(*RenderState::_states_lock);

  // With uniquify-states turned on, we can actually go through all the states
  // and check whether their generated shader is still OK.
  size_t size = RenderState::_states.get_num_entries();
  for (size_t si = 0; si < size; ++si) {
    const RenderState *state = RenderState::_states.get_key(si);

    if (state->_generated_shader != nullptr) {
      ShaderKey key;
      analyze_renderstate(key, state);

      GeneratedShaders::const_iterator si;
      si = _generated_shaders.find(key);
      if (si != _generated_shaders.end()) {
        if (si->second != state->_generated_shader) {
          state->_generated_shader = si->second;
          state->_munged_states.clear();
        }
      } else {
        // We have not yet generated a shader for this modified state.
        state->_generated_shader.clear();
        state->_munged_states.clear();
      }
    }
  }

  // If we don't have uniquify-states, however, the above list won't contain
  // all the state.  We can change a global seq value to require Panda to
  // rehash the states the next time it tries to render an object with it.
  if (!uniquify_states) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * Removes all previously generated shaders, requiring all shaders to be
 * regenerated.  Does not clear cache of compiled shaders.
 *
 * @since 1.10.0
 */
void ShaderGenerator::
clear_generated_shaders() {
  LightReMutexHolder holder(*RenderState::_states_lock);

  size_t size = RenderState::_states.get_num_entries();
  for (size_t si = 0; si < size; ++si) {
    const RenderState *state = RenderState::_states.get_key(si);
    state->_generated_shader.clear();
  }

  _generated_shaders.clear();

  // If we don't have uniquify-states, we can't clear all the ShaderAttribs
  // that are cached on the states, but we can simulate the effect of that.
  if (!uniquify_states) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * This is the routine that implements the next-gen fixed function pipeline by
 * synthesizing a shader.  It also takes care of setting up any buffers needed
 * to produce the requested effects.
 *
 * Currently supports:
 * - flat colors
 * - vertex colors
 * - lighting
 * - normal maps, even multiple
 * - gloss maps, but not multiple
 * - glow maps, but not multiple
 * - materials, but not updates to materials
 * - 2D textures
 * - all texture stage modes, including combine modes
 * - color scale attrib
 * - light ramps (for cartoon shading)
 * - shadow mapping
 * - most texgen modes
 * - texmatrix
 * - 1D/2D/3D textures, cube textures, 2D tex arrays
 * - linear/exp/exp2 fog
 * - animation
 *
 * Potential optimizations
 * - omit attenuation calculations if attenuation off
 *
 */
CPT(ShaderAttrib) ShaderGenerator::
synthesize_shader(const RenderState *rs, const GeomVertexAnimationSpec &anim) {
  ShaderKey key;

  // First look up the state key in the table of already generated shaders.
  {
    PStatTimer timer(lookup_collector);
    key._anim_spec = anim;
    analyze_renderstate(key, rs);

    GeneratedShaders::const_iterator si;
    si = _generated_shaders.find(key);
    if (si != _generated_shaders.end()) {
      // We've already generated a shader for this state.
      return si->second;
    }
  }

  PStatTimer timer(synthesize_collector);

  reset_register_allocator();

  if (pgraphnodes_cat.is_debug()) {
    pgraphnodes_cat.debug()
      << "Generating shader for render state " << rs << ":\n";
    rs->write(pgraphnodes_cat.debug(false), 2);
  }

  // These variables will hold the results of register allocation.

  const char *tangent_freg = nullptr;
  const char *binormal_freg = nullptr;
  string tangent_input;
  string binormal_input;
  pmap<const InternalName *, const char *> texcoord_fregs;
  pvector<const char *> lightcoord_fregs;
  const char *world_position_freg = nullptr;
  const char *world_normal_freg = nullptr;
  const char *eye_position_freg = nullptr;
  const char *eye_normal_freg = nullptr;
  const char *hpos_freg = nullptr;

  const char *position_vreg;
  const char *transform_weight_vreg = nullptr;
  const char *normal_vreg;
  const char *color_vreg = nullptr;
  const char *transform_index_vreg = nullptr;

  if (_use_generic_attr) {
    position_vreg = "ATTR0";
    transform_weight_vreg = "ATTR1";
    normal_vreg = "ATTR2";
    transform_index_vreg = "ATTR7";
  } else {
    position_vreg = "POSITION";
    normal_vreg = "NORMAL";
  }

  if (key._color_type == ColorAttrib::T_vertex) {
    // Reserve COLOR0
    color_vreg = _use_generic_attr ? "ATTR3" : "COLOR0";
    _vcregs_used = 1;
    _fcregs_used = 1;
  }

  // Generate the shader's text.

  std::ostringstream text;

  text << "//Cg\n";

  text << "/* Generated shader for render state:\n";
  rs->write(text, 2);
  text << "*/\n";

  int map_index_glow = -1;
  int map_index_gloss = -1;

  // Figure out whether we need to calculate any of these variables.
  bool need_world_position = (key._num_clip_planes > 0);
  bool need_world_normal = false;
  bool need_eye_position = key._lighting;
  bool need_eye_normal = !key._lights.empty() || ((key._outputs & AuxBitplaneAttrib::ABO_aux_normal) != 0);
  bool need_tangents = ((key._texture_flags & ShaderKey::TF_map_normal) != 0);

  // If we have binormal/tangent and eye position, we can pack eye normal in
  // the w channels of the others.
  bool pack_eye_normal = need_eye_normal && need_tangents && need_eye_position;

  bool have_specular = false;
  if (key._lighting) {
    if (key._material_flags & Material::F_specular) {
      have_specular = true;
    } else if ((key._texture_flags & ShaderKey::TF_map_gloss) != 0) {
      have_specular = true;
    }
  }

  bool need_color = false;
  if (key._color_type != ColorAttrib::T_off) {
    if (key._lighting) {
      if (((key._material_flags & Material::F_ambient) == 0 && key._have_separate_ambient) ||
          (key._material_flags & Material::F_diffuse) == 0 ||
          key._calc_primary_alpha) {
        need_color = true;
      }
    } else {
      need_color = true;
    }
  }

  text << "void vshader(\n";
  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];

    switch (tex._gen_mode) {
    case TexGenAttrib::M_world_position:
      need_world_position = true;
      break;
    case TexGenAttrib::M_world_normal:
      need_world_normal = true;
      break;
    case TexGenAttrib::M_eye_position:
      need_eye_position = true;
      break;
    case TexGenAttrib::M_eye_normal:
      need_eye_normal = true;
      break;
    default:
      break;
    }

    if (tex._texcoord_name != nullptr) {
      if (texcoord_fregs.count(tex._texcoord_name) == 0) {
        const char *freg = alloc_freg();
        texcoord_fregs[tex._texcoord_name] = freg;

        string tcname = tex._texcoord_name->join("_");
        text << "\t in float4 vtx_" << tcname << " : " << alloc_vreg() << ",\n";
        text << "\t out float4 l_" << tcname << " : " << freg << ",\n";
      }
    }

    if (tangent_input.empty() &&
        (tex._flags & (ShaderKey::TF_map_normal | ShaderKey::TF_map_height)) != 0) {
      PT(InternalName) tangent_name = InternalName::get_tangent();
      PT(InternalName) binormal_name = InternalName::get_binormal();

      if (tex._texcoord_name != nullptr &&
          tex._texcoord_name != InternalName::get_texcoord()) {
        tangent_name = tangent_name->append(tex._texcoord_name->get_basename());
        binormal_name = binormal_name->append(tex._texcoord_name->get_basename());
      }

      tangent_input = tangent_name->join("_");
      binormal_input = binormal_name->join("_");

      text << "\t in float4 vtx_" << tangent_input << " : " << alloc_vreg() << ",\n";
      text << "\t in float4 vtx_" << binormal_input << " : " << alloc_vreg() << ",\n";
    }

    if (tex._flags & ShaderKey::TF_map_glow) {
      map_index_glow = i;
    }
    if (tex._flags & ShaderKey::TF_map_gloss) {
      map_index_gloss = i;
    }
  }
  if (need_tangents) {
    tangent_freg = alloc_freg();
    binormal_freg = alloc_freg();
    text << "\t out float4 l_tangent : " << tangent_freg << ",\n";
    text << "\t out float4 l_binormal : " << binormal_freg << ",\n";
  }
  if (need_color && key._color_type == ColorAttrib::T_vertex) {
    text << "\t in float4 vtx_color : " << color_vreg << ",\n";
    text << "\t out float4 l_color : COLOR0,\n";
  }
  if (need_world_position || need_world_normal) {
    text << "\t uniform float4x4 trans_model_to_world,\n";
  }
  if (need_world_position) {
    world_position_freg = alloc_freg();
    text << "\t out float4 l_world_position : " << world_position_freg << ",\n";
  }
  if (need_world_normal) {
    world_normal_freg = alloc_freg();
    text << "\t out float4 l_world_normal : " << world_normal_freg << ",\n";
  }
  if (need_eye_position) {
    text << "\t uniform float4x4 trans_model_to_view,\n";
    eye_position_freg = alloc_freg();
    text << "\t out float4 l_eye_position : " << eye_position_freg << ",\n";
  } else if (need_tangents) {
    text << "\t uniform float4x4 trans_model_to_view,\n";
  }
  if (need_eye_normal) {
    text << "\t uniform float4x4 tpose_view_to_model,\n";
    if (!pack_eye_normal)  {
      eye_normal_freg = alloc_freg();
      text << "\t out float3 l_eye_normal : " << eye_normal_freg << ",\n";
    }
  }
  if ((key._texture_flags & ShaderKey::TF_map_height) != 0 || need_world_normal || need_eye_normal) {
    text << "\t in float3 vtx_normal : " << normal_vreg << ",\n";
  }
  if (key._texture_flags & ShaderKey::TF_map_height) {
    text << "\t uniform float4 mspos_view,\n";
    text << "\t out float3 l_eyevec,\n";
  }
  if (key._fog_mode != 0) {
    hpos_freg = alloc_freg();
    text << "\t out float4 l_hpos : " << hpos_freg << ",\n";
  }
  for (size_t i = 0; i < key._lights.size(); ++i) {
    const ShaderKey::LightInfo &light = key._lights[i];
    if (light._flags & ShaderKey::LF_has_shadows) {
      if (_ftregs_used >= 8) {
        // We ran out of TEXCOORD registers.  That means we have to do this
        // calculation in the fragment shader, which is slower.
        lightcoord_fregs.push_back(nullptr);
      } else {
        lightcoord_fregs.push_back(alloc_freg());
        text << "\t uniform float4x4 mat_shadow_" << i << ",\n";
        text << "\t out float4 l_lightcoord" << i << " : " << lightcoord_fregs[i] << ",\n";
      }
    } else {
      lightcoord_fregs.push_back(nullptr);
    }
  }
  if (key._anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      key._anim_spec.get_num_transforms() > 0) {
    int num_transforms;
    if (key._anim_spec.get_indexed_transforms()) {
      num_transforms = 120;
    } else {
      num_transforms = key._anim_spec.get_num_transforms();
    }
    if (transform_weight_vreg == nullptr) {
      transform_weight_vreg = alloc_vreg();
    }
    if (transform_index_vreg == nullptr) {
      transform_index_vreg = alloc_vreg();
    }
    text << "\t uniform float4x4 tbl_transforms[" << num_transforms << "],\n";
    text << "\t in float4 vtx_transform_weight : " << transform_weight_vreg << ",\n";
    if (key._anim_spec.get_indexed_transforms()) {
      text << "\t in uint4 vtx_transform_index : " << transform_index_vreg << ",\n";
    }
  }
  text << "\t in float4 vtx_position : " << position_vreg << ",\n";
  text << "\t out float4 l_position : POSITION,\n";
  text << "\t uniform float4x4 mat_modelproj\n";
  text << ") {\n";

  if (key._anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      key._anim_spec.get_num_transforms() > 0) {

    if (!key._anim_spec.get_indexed_transforms()) {
      text << "\t const uint4 vtx_transform_index = uint4(0, 1, 2, 3);\n";
    }

    text << "\t float4x4 matrix = tbl_transforms[vtx_transform_index.x] * vtx_transform_weight.x";
    if (key._anim_spec.get_num_transforms() > 1) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.y] * vtx_transform_weight.y";
    }
    if (key._anim_spec.get_num_transforms() > 2) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.z] * vtx_transform_weight.z";
    }
    if (key._anim_spec.get_num_transforms() > 3) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.w] * vtx_transform_weight.w";
    }
    text << ";\n";

    text << "\t vtx_position = mul(matrix, vtx_position);\n";
    if (need_world_normal || need_eye_normal) {
      text << "\t vtx_normal = mul((float3x3)matrix, vtx_normal);\n";
    }
  }

  text << "\t l_position = mul(mat_modelproj, vtx_position);\n";
  if (key._fog_mode != 0) {
    text << "\t l_hpos = l_position;\n";
  }
  if (need_world_position) {
    text << "\t l_world_position = mul(trans_model_to_world, vtx_position);\n";
  }
  if (need_world_normal) {
    text << "\t l_world_normal = mul(trans_model_to_world, float4(vtx_normal, 0));\n";
  }
  if (need_eye_position) {
    text << "\t l_eye_position = mul(trans_model_to_view, vtx_position);\n";
  }
  pmap<const InternalName *, const char *>::const_iterator it;
  for (it = texcoord_fregs.begin(); it != texcoord_fregs.end(); ++it) {
    // Pass through all texcoord inputs as-is.
    string tcname = it->first->join("_");
    text << "\t l_" << tcname << " = vtx_" << tcname << ";\n";
  }
  if (need_color && key._color_type == ColorAttrib::T_vertex) {
    text << "\t l_color = vtx_color;\n";
  }
  if (need_tangents) {
    text << "\t l_tangent.xyz = normalize(mul((float3x3)trans_model_to_view, vtx_" << tangent_input << ".xyz));\n";
    text << "\t l_tangent.w = 0;\n";
    text << "\t l_binormal.xyz = normalize(mul((float3x3)trans_model_to_view, -vtx_" << binormal_input << ".xyz));\n";
    text << "\t l_binormal.w = 0;\n";
  }
  for (size_t i = 0; i < key._lights.size(); ++i) {
    if (key._lights[i]._flags & ShaderKey::LF_has_shadows) {
      if (lightcoord_fregs[i] != nullptr) {
        text << "\t l_lightcoord" << i << " = mul(mat_shadow_" << i << ", l_eye_position);\n";
      }
    }
  }
  if (key._texture_flags & ShaderKey::TF_map_height) {
    text << "\t float3 eyedir = mspos_view.xyz - vtx_position.xyz;\n";
    text << "\t l_eyevec.x = dot(vtx_" << tangent_input << ".xyz, eyedir);\n";
    text << "\t l_eyevec.y = dot(vtx_" << binormal_input << ".xyz, eyedir);\n";
    text << "\t l_eyevec.z = dot(vtx_normal, eyedir);\n";
    text << "\t l_eyevec = normalize(l_eyevec);\n";
  }
  if (need_eye_normal) {
    if (pack_eye_normal) {
      // We can pack the normal into the w channels of these unused varyings.
      text << "\t float3 eye_normal = normalize(mul((float3x3)tpose_view_to_model, vtx_normal));\n";
      text << "\t l_tangent.w = eye_normal.x;\n";
      text << "\t l_binormal.w = eye_normal.y;\n";
      text << "\t l_eye_position.w = eye_normal.z;\n";
    } else {
      text << "\t l_eye_normal = normalize(mul((float3x3)tpose_view_to_model, vtx_normal));\n";
    }
  }
  text << "}\n\n";

  // Fragment shader

  text << "void fshader(\n";
  if (key._fog_mode != 0) {
    text << "\t in float4 l_hpos : " << hpos_freg << ",\n";
    text << "\t in uniform float4 attr_fog,\n";
    text << "\t in uniform float4 attr_fogcolor,\n";
  }
  if (need_world_position) {
    text << "\t in float4 l_world_position : " << world_position_freg << ",\n";
  }
  if (need_world_normal) {
    text << "\t in float4 l_world_normal : " << world_normal_freg << ",\n";
  }
  if (need_eye_position) {
    text << "\t in float4 l_eye_position : " << eye_position_freg << ",\n";
  }
  if (need_eye_normal && !pack_eye_normal) {
    text << "\t in float3 l_eye_normal : " << eye_normal_freg << ",\n";
  }
  for (it = texcoord_fregs.begin(); it != texcoord_fregs.end(); ++it) {
    text << "\t in float4 l_" << it->first->join("_") << " : " << it->second << ",\n";
  }
  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }

    text << "\t uniform sampler" << texture_type_as_string(tex._type) << " tex_" << i << ",\n";

    if (tex._flags & ShaderKey::TF_has_texscale) {
      text << "\t uniform float3 texscale_" << i << ",\n";
    } else if (tex._flags & ShaderKey::TF_has_texmat) {
      text << "\t uniform float4x4 texmat_" << i << ",\n";
    }

    if (tex._flags & ShaderKey::TF_uses_color) {
      text << "\t uniform float4 texcolor_" << i << ",\n";
    }
  }
  if (need_tangents) {
    text << "\t in float4 l_tangent : " << tangent_freg << ",\n";
    text << "\t in float4 l_binormal : " << binormal_freg << ",\n";
  }
  for (size_t i = 0; i < key._lights.size(); ++i) {
    text << "\t uniform float4x4 attr_light" << i << ",\n";

    const ShaderKey::LightInfo &light = key._lights[i];
    if (light._flags & ShaderKey::LF_has_shadows) {
      if (light._type.is_derived_from(PointLight::get_class_type())) {
        text << "\t uniform samplerCUBE shadow_" << i << ",\n";
      } else if (_use_shadow_filter) {
        text << "\t uniform sampler2DShadow shadow_" << i << ",\n";
      } else {
        text << "\t uniform sampler2D shadow_" << i << ",\n";
      }
      if (lightcoord_fregs[i] != nullptr) {
        text << "\t in float4 l_lightcoord" << i << " : " << lightcoord_fregs[i] << ",\n";
      } else {
        text << "\t uniform float4x4 mat_shadow_" << i << ",\n";
      }
    }
    if (light._flags & ShaderKey::LF_has_specular_color) {
      text << "\t uniform float4 attr_lspec" << i << ",\n";
    }
  }

  // Does the shader need material properties as input?
  if (key._material_flags & (Material::F_ambient | Material::F_diffuse | Material::F_emission | Material::F_specular)) {
    text << "\t uniform float4x4 attr_material,\n";
  }
  if (key._texture_flags & ShaderKey::TF_map_height) {
    text << "\t float3 l_eyevec,\n";
  }
  if (key._outputs & (AuxBitplaneAttrib::ABO_aux_normal | AuxBitplaneAttrib::ABO_aux_glow)) {
    text << "\t out float4 o_aux : COLOR1,\n";
  }
  text << "\t out float4 o_color : COLOR0,\n";

  if (need_color) {
    if (key._color_type == ColorAttrib::T_vertex) {
      text << "\t in float4 l_color : COLOR0,\n";
    } else if (key._color_type == ColorAttrib::T_flat) {
      text << "\t uniform float4 attr_color,\n";
    }
  }

  for (int i = 0; i < key._num_clip_planes; ++i) {
    text << "\t uniform float4 clipplane_" << i << ",\n";
  }

  text << "\t uniform float4 attr_ambient,\n";
  text << "\t uniform float4 attr_colorscale\n";
  text << ") {\n";

  // Clipping first!
  for (int i = 0; i < key._num_clip_planes; ++i) {
    text << "\t if (l_world_position.x * clipplane_" << i << ".x + l_world_position.y ";
    text << "* clipplane_" << i << ".y + l_world_position.z * clipplane_" << i << ".z + clipplane_" << i << ".w <= 0) {\n";
    text << "\t discard;\n";
    text << "\t }\n";
  }

  // Reconstruct a packed normal vector.
  if (need_eye_normal && pack_eye_normal) {
    text << "\t float3 l_eye_normal = float3(l_tangent.w, l_binormal.w, l_eye_position.w);\n";
  }

  text << "\t float4 result;\n";
  if (key._outputs & (AuxBitplaneAttrib::ABO_aux_normal | AuxBitplaneAttrib::ABO_aux_glow)) {
    text << "\t o_aux = float4(0, 0, 0, 0);\n";
  }
  // Now generate any texture coordinates according to TexGenAttrib.  If it
  // has a TexMatrixAttrib, also transform them.
  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    switch (tex._gen_mode) {
    case TexGenAttrib::M_off:
      // Cg seems to be able to optimize this temporary away when appropriate.
      text << "\t float4 texcoord" << i << " = l_" << tex._texcoord_name->join("_") << ";\n";
      break;
    case TexGenAttrib::M_world_position:
      text << "\t float4 texcoord" << i << " = l_world_position;\n";
      break;
    case TexGenAttrib::M_world_normal:
      text << "\t float4 texcoord" << i << " = l_world_normal;\n";
      break;
    case TexGenAttrib::M_eye_position:
      text << "\t float4 texcoord" << i << " = float4(l_eye_position.xyz, 1.0f);\n";
      break;
    case TexGenAttrib::M_eye_normal:
      text << "\t float4 texcoord" << i << " = float4(l_eye_normal, 1.0f);\n";
      break;
    default:
      text << "\t float4 texcoord" << i << " = float4(0, 0, 0, 0);\n";
      pgraphnodes_cat.error()
        << "Unsupported TexGenAttrib mode: " << tex._gen_mode << "\n";
    }
    if (tex._flags & ShaderKey::TF_has_texscale) {
      text << "\t texcoord" << i << ".xyz *= texscale_" << i << ";\n";
    } else if (tex._flags & ShaderKey::TF_has_texmat) {
      text << "\t texcoord" << i << " = mul(texmat_" << i << ", texcoord" << i << ");\n";
      text << "\t texcoord" << i << ".xyz /= texcoord" << i << ".w;\n";
    }
  }
  text << "\t // Fetch all textures.\n";
  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if ((tex._flags & ShaderKey::TF_map_height) == 0) {
      continue;
    }

    text << "\t float4 tex" << i << " = tex" << texture_type_as_string(tex._type);
    text << "(tex_" << i << ", texcoord" << i << ".";
    switch (tex._type) {
    case Texture::TT_cube_map:
    case Texture::TT_3d_texture:
    case Texture::TT_2d_texture_array:
      text << "xyz";
      break;
    case Texture::TT_2d_texture:
      text << "xy";
      break;
    case Texture::TT_1d_texture:
      text << "x";
      break;
    default:
      break;
    }
    text << ");\n\t float3 parallax_offset = l_eyevec.xyz * (tex" << i;
    if (tex._mode == TextureStage::M_normal_height ||
        (tex._flags & ShaderKey::TF_has_alpha) != 0) {
      text << ".aaa";
    } else {
      text << ".rgb";
    }
    text << " * 2.0 - 1.0) * " << parallax_mapping_scale << ";\n";
    // Additional samples
    for (int j = 0; j < parallax_mapping_samples - 1; ++j) {
      text << "\t parallax_offset = l_eyevec.xyz * (parallax_offset + (tex" << i;
      if (tex._mode == TextureStage::M_normal_height ||
          (tex._flags & ShaderKey::TF_has_alpha) != 0) {
        text << ".aaa";
      } else {
        text << ".rgb";
      }
      text << " * 2.0 - 1.0)) * " << 0.5 * parallax_mapping_scale << ";\n";
    }
  }
  for (size_t i = 0; i < key._textures.size(); ++i) {
    ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    if ((tex._flags & ShaderKey::TF_map_height) == 0) {
      // Parallax mapping pushes the texture coordinates of the other textures
      // away from the camera.
      if (key._texture_flags & ShaderKey::TF_map_height) {
        text << "\t texcoord" << i << ".xyz -= parallax_offset;\n";
      }
      text << "\t float4 tex" << i << " = tex" << texture_type_as_string(tex._type);
      text << "(tex_" << i << ", texcoord" << i << ".";
      switch (tex._type) {
      case Texture::TT_cube_map:
      case Texture::TT_3d_texture:
      case Texture::TT_2d_texture_array:
        text << "xyz";
        break;
      case Texture::TT_2d_texture:
        text << "xy";
        break;
      case Texture::TT_1d_texture:
        text << "x";
        break;
      default:
        break;
      }
      text << ");\n";
    }
  }
  if (need_eye_normal) {
    text << "\t // Correct the surface normal for interpolation effects\n";
    text << "\t l_eye_normal = normalize(l_eye_normal);\n";
  }
  if (need_tangents) {
    text << "\t // Translate tangent-space normal in map to view-space.\n";

    // Use Reoriented Normal Mapping to blend additional normal maps.
    bool is_first = true;
    for (size_t i = 0; i < key._textures.size(); ++i) {
      const ShaderKey::TextureInfo &tex = key._textures[i];
      if (tex._flags & ShaderKey::TF_map_normal) {
        if (is_first) {
          text << "\t float3 tsnormal = normalize((tex" << i << ".xyz * 2) - 1);\n";
          is_first = false;
          continue;
        }
        text << "\t tsnormal.z += 1;\n";
        text << "\t float3 tmp" << i << " = tex" << i << ".xyz * float3(-2, -2, 2) + float3(1, 1, -1);\n";
        text << "\t tsnormal = normalize(tsnormal * dot(tsnormal, tmp" << i << ") - tmp" << i << " * tsnormal.z);\n";
      }
    }
    text << "\t l_eye_normal *= tsnormal.z;\n";
    text << "\t l_eye_normal += normalize(l_tangent.xyz) * tsnormal.x;\n";
    text << "\t l_eye_normal += normalize(l_binormal.xyz) * tsnormal.y;\n";
    text << "\t l_eye_normal = normalize(l_eye_normal);\n";
  }
  if (key._outputs & AuxBitplaneAttrib::ABO_aux_normal) {
    text << "\t // Output the camera-space surface normal\n";
    text << "\t o_aux.rgb = (l_eye_normal*0.5) + float3(0.5,0.5,0.5);\n";
  }
  if (key._lighting) {
    text << "\t // Begin view-space light calculations\n";
    text << "\t float ldist,lattenv,langle,lshad;\n";
    text << "\t float4 lcolor,lspec,lpoint,latten,ldir,leye;\n";
    text << "\t float3 lvec,lhalf;\n";
    if (key._have_separate_ambient) {
      text << "\t float4 tot_ambient = float4(0,0,0,0);\n";
    }
    text << "\t float4 tot_diffuse = float4(0,0,0,0);\n";
    if (have_specular) {
      text << "\t float4 tot_specular = float4(0,0,0,0);\n";
      if (key._material_flags & Material::F_specular) {
        text << "\t float shininess = attr_material[3].w;\n";
      } else {
        text << "\t float shininess = 50; // no shininess specified, using default\n";
      }
    }
    if (key._have_separate_ambient) {
      text << "\t tot_ambient += attr_ambient;\n";
    } else {
      text << "\t tot_diffuse += attr_ambient;\n";
    }
  }
  for (size_t i = 0; i < key._lights.size(); ++i) {
    const ShaderKey::LightInfo &light = key._lights[i];

    if (light._flags & ShaderKey::LF_has_shadows) {
      if (lightcoord_fregs[i] == nullptr) {
        // We have to do this one in the fragment shader if we ran out of
        // varyings.
        text << "\t float4 l_lightcoord" << i << " = mul(mat_shadow_" << i << ", float4(l_eye_position.xyz, 1.0f));\n";
      }
    }

    if (light._type.is_derived_from(DirectionalLight::get_class_type())) {
      text << "\t // Directional Light " << i << "\n";
      text << "\t lcolor = attr_light" << i << "[0];\n";
      if (light._flags & ShaderKey::LF_has_specular_color) {
        text << "\t lspec  = attr_lspec" << i << ";\n";
      } else {
        text << "\t lspec  = lcolor;\n";
      }
      text << "\t lvec   = attr_light" << i << "[3].xyz;\n";
      text << "\t lcolor *= saturate(dot(l_eye_normal, lvec.xyz));\n";
      if (light._flags & ShaderKey::LF_has_shadows) {
        if (_use_shadow_filter) {
          text << "\t lshad = shadow2DProj(shadow_" << i << ", l_lightcoord" << i << ").r;\n";
        } else {
          text << "\t lshad = tex2Dproj(shadow_" << i << ", l_lightcoord" << i << ").r > l_lightcoord" << i << ".z / l_lightcoord" << i << ".w;\n";
        }
        text << "\t lcolor *= lshad;\n";
        text << "\t lspec *= lshad;\n";
      }
      text << "\t tot_diffuse += lcolor;\n";
      if (have_specular) {
        if (key._material_flags & Material::F_local) {
          text << "\t lhalf = normalize(lvec - normalize(l_eye_position.xyz));\n";
        } else {
          text << "\t lhalf = normalize(lvec - float3(0, 1, 0));\n";
        }
        text << "\t lspec *= pow(saturate(dot(l_eye_normal, lhalf)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    } else if (light._type.is_derived_from(PointLight::get_class_type())) {
      text << "\t // Point Light " << i << "\n";
      text << "\t lcolor = attr_light" << i << "[0];\n";
      if (light._flags & ShaderKey::LF_has_specular_color) {
        text << "\t lspec  = attr_lspec" << i << ";\n";
      } else {
        text << "\t lspec  = lcolor;\n";
      }
      text << "\t latten = attr_light" << i << "[1];\n";
      text << "\t lpoint = attr_light" << i << "[3];\n";
      text << "\t lvec   = lpoint.xyz - l_eye_position.xyz;\n";
      text << "\t ldist = length(lvec);\n";
      text << "\t lvec /= ldist;\n";
      if (light._type.is_derived_from(SphereLight::get_class_type())) {
        text << "\t ldist = max(ldist, attr_light" << i << "[2].w);\n";
      }
      text << "\t lattenv = 1/(latten.x + latten.y*ldist + latten.z*ldist*ldist);\n";
      text << "\t lcolor *= lattenv * saturate(dot(l_eye_normal, lvec));\n";
      if (light._flags & ShaderKey::LF_has_shadows) {
        text << "\t ldist = max(abs(l_lightcoord" << i << ".x), max(abs(l_lightcoord" << i << ".y), abs(l_lightcoord" << i << ".z)));\n";
        text << "\t ldist = ((latten.w+lpoint.w)/(latten.w-lpoint.w))+((-2*latten.w*lpoint.w)/(ldist * (latten.w-lpoint.w)));\n";
        text << "\t lshad = texCUBE(shadow_" << i << ", l_lightcoord" << i << ".xyz).r >= ldist * 0.5 + 0.5;\n";
        text << "\t lcolor *= lshad;\n";
        text << "\t lspec *= lshad;\n";
      }
      text << "\t tot_diffuse += lcolor;\n";
      if (have_specular) {
        if (key._material_flags & Material::F_local) {
          text << "\t lhalf = normalize(lvec - normalize(l_eye_position.xyz));\n";
        } else {
          text << "\t lhalf = normalize(lvec - float3(0, 1, 0));\n";
        }
        text << "\t lspec *= lattenv;\n";
        text << "\t lspec *= pow(saturate(dot(l_eye_normal, lhalf)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    } else if (light._type.is_derived_from(Spotlight::get_class_type())) {
      text << "\t // Spot Light " << i << "\n";
      text << "\t lcolor = attr_light" << i << "[0];\n";
      if (light._flags & ShaderKey::LF_has_specular_color) {
        text << "\t lspec  = attr_lspec" << i << ";\n";
      } else {
        text << "\t lspec  = lcolor;\n";
      }
      text << "\t latten = attr_light" << i << "[1];\n";
      text << "\t ldir   = attr_light" << i << "[2];\n";
      text << "\t lpoint = attr_light" << i << "[3];\n";
      text << "\t lvec   = lpoint.xyz - l_eye_position.xyz;\n";
      text << "\t ldist  = length(lvec);\n";
      text << "\t lvec /= ldist;\n";
      text << "\t langle = saturate(dot(ldir.xyz, lvec));\n";
      text << "\t lattenv = 1/(latten.x + latten.y*ldist + latten.z*ldist*ldist);\n";
      text << "\t lattenv *= pow(langle, latten.w);\n";
      text << "\t if (langle < ldir.w) lattenv = 0;\n";
      text << "\t lcolor *= lattenv * saturate(dot(l_eye_normal, lvec));\n";
      if (light._flags & ShaderKey::LF_has_shadows) {
        if (_use_shadow_filter) {
          text << "\t lshad = shadow2DProj(shadow_" << i << ", l_lightcoord" << i << ").r;\n";
        } else {
          text << "\t lshad = tex2Dproj(shadow_" << i << ", l_lightcoord" << i << ").r > l_lightcoord" << i << ".z / l_lightcoord" << i << ".w;\n";
        }
        text << "\t lcolor *= lshad;\n";
        text << "\t lspec *= lshad;\n";
      }

      text << "\t tot_diffuse += lcolor;\n";
      if (have_specular) {
        if (key._material_flags & Material::F_local) {
          text << "\t lhalf = normalize(lvec - normalize(l_eye_position.xyz));\n";
        } else {
          text << "\t lhalf = normalize(lvec - float3(0,1,0));\n";
        }
        text << "\t lspec *= lattenv;\n";
        text << "\t lspec *= pow(saturate(dot(l_eye_normal, lhalf)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    }
  }
  if (key._lighting) {
    if (key._light_ramp != nullptr) {
      switch (key._light_ramp->get_mode()) {
      case LightRampAttrib::LRT_single_threshold:
        {
          PN_stdfloat t = key._light_ramp->get_threshold(0);
          PN_stdfloat l0 = key._light_ramp->get_level(0);
          text << "\t // Single-threshold light ramp\n";
          text << "\t float lr_in = dot(tot_diffuse.rgb, float3(0.33,0.34,0.33));\n";
          text << "\t float lr_scale = (lr_in < " << t << ") ? 0.0 : (" << l0 << "/lr_in);\n";
          text << "\t tot_diffuse = tot_diffuse * lr_scale;\n";
          break;
        }
      case LightRampAttrib::LRT_double_threshold:
        {
          PN_stdfloat t0 = key._light_ramp->get_threshold(0);
          PN_stdfloat t1 = key._light_ramp->get_threshold(1);
          PN_stdfloat l0 = key._light_ramp->get_level(0);
          PN_stdfloat l1 = key._light_ramp->get_level(1);
          text << "\t // Double-threshold light ramp\n";
          text << "\t float lr_in = dot(tot_diffuse.rgb, float3(0.33,0.34,0.33));\n";
          text << "\t float lr_out = 0.0;\n";
          text << "\t if (lr_in > " << t0 << ") lr_out=" << l0 << ";\n";
          text << "\t if (lr_in > " << t1 << ") lr_out=" << l1 << ";\n";
          text << "\t tot_diffuse = tot_diffuse * (lr_out / lr_in);\n";
          break;
        }
      default:
        break;
      }
    }
    text << "\t // Begin view-space light summation\n";
    if (key._material_flags & Material::F_emission) {
      if (key._texture_flags & ShaderKey::TF_map_glow) {
        text << "\t result = attr_material[2] * saturate(2 * (tex" << map_index_glow << ".a - 0.5));\n";
      } else {
        text << "\t result = attr_material[2];\n";
      }
    } else {
      if (key._texture_flags & ShaderKey::TF_map_glow) {
        text << "\t result = saturate(2 * (tex" << map_index_glow << ".a - 0.5));\n";
      } else {
        text << "\t result = float4(0,0,0,0);\n";
      }
    }
    if (key._have_separate_ambient) {
      if (key._material_flags & Material::F_ambient) {
        text << "\t result += tot_ambient * attr_material[0];\n";
      } else if (key._color_type == ColorAttrib::T_vertex) {
        text << "\t result += tot_ambient * l_color;\n";
      } else if (key._color_type == ColorAttrib::T_flat) {
        text << "\t result += tot_ambient * attr_color;\n";
      } else {
        text << "\t result += tot_ambient;\n";
      }
    }
    if (key._material_flags & Material::F_diffuse) {
      text << "\t result += tot_diffuse * attr_material[1];\n";
    } else if (key._color_type == ColorAttrib::T_vertex) {
      text << "\t result += tot_diffuse * l_color;\n";
    } else if (key._color_type == ColorAttrib::T_flat) {
      text << "\t result += tot_diffuse * attr_color;\n";
    } else {
      text << "\t result += tot_diffuse;\n";
    }
    if (key._light_ramp == nullptr ||
        key._light_ramp->get_mode() == LightRampAttrib::LRT_default) {
      text << "\t result = saturate(result);\n";
    }
    text << "\t // End view-space light calculations\n";

    // Combine in alpha, which bypasses lighting calculations.  Use of lerp
    // here is a workaround for a radeon driver bug.
    if (key._calc_primary_alpha) {
      if (key._color_type == ColorAttrib::T_vertex) {
        text << "\t result.a = l_color.a;\n";
      } else if (key._color_type == ColorAttrib::T_flat) {
        text << "\t result.a = attr_color.a;\n";
      } else {
        text << "\t result.a = 1;\n";
      }
    }
  } else {
    if (key._color_type == ColorAttrib::T_vertex) {
      text << "\t result = l_color;\n";
    } else if (key._color_type == ColorAttrib::T_flat) {
      text << "\t result = attr_color;\n";
    } else {
      text << "\t result = float4(1, 1, 1, 1);\n";
    }
  }

  // Apply the color scale.
  text << "\t result *= attr_colorscale;\n";

  // Store these if any stages will use it.
  if (key._texture_flags & ShaderKey::TF_uses_primary_color) {
    text << "\t float4 primary_color = result;\n";
  }
  if (key._texture_flags & ShaderKey::TF_uses_last_saved_result) {
    text << "\t float4 last_saved_result = result;\n";
  }

  // Now loop through the textures to compose our magic blending formulas.
  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    TextureStage::CombineMode combine_rgb, combine_alpha;

    switch (tex._mode) {
    case TextureStage::M_modulate:
      if ((tex._flags & ShaderKey::TF_has_rgb) != 0 &&
          (tex._flags & ShaderKey::TF_has_alpha) != 0) {
        text << "\t result.rgba *= tex" << i << ".rgba;\n";
      } else if (tex._flags & ShaderKey::TF_has_alpha) {
        text << "\t result.a *= tex" << i << ".a;\n";
      } else if (tex._flags & ShaderKey::TF_has_rgb) {
        text << "\t result.rgb *= tex" << i << ".rgb;\n";
      }
      break;
    case TextureStage::M_modulate_glow:
    case TextureStage::M_modulate_gloss:
      // in the case of glow or spec we currently see the specularity evenly
      // across the surface even if transparency or masking is present not
      // sure if this is desired behavior or not.  *MOST* would construct a
      // spec map based off of what isisn't seen based on the masktransparency
      // this may have to be left alone for now agartner
      text << "\t result.rgb *= tex" << i << ";\n";
      break;
    case TextureStage::M_decal:
      text << "\t result.rgb = lerp(result, tex" << i << ", tex" << i << ".a).rgb;\n";
      break;
    case TextureStage::M_blend:
      text << "\t result.rgb = lerp(result.rgb, texcolor_" << i << ".rgb, tex" << i << ".rgb);\n";
      if (key._calc_primary_alpha) {
        text << "\t result.a *= tex" << i << ".a;\n";
      }
      break;
    case TextureStage::M_replace:
      text << "\t result = tex" << i << ";\n";
      break;
    case TextureStage::M_add:
      text << "\t result.rgb += tex" << i << ".rgb;\n";
      if (key._calc_primary_alpha) {
        text << "\t result.a   *= tex" << i << ".a;\n";
      }
      break;
    case TextureStage::M_combine:
      combine_rgb = (TextureStage::CombineMode)((tex._flags & ShaderKey::TF_COMBINE_RGB_MODE_MASK) >> ShaderKey::TF_COMBINE_RGB_MODE_SHIFT);
      combine_alpha = (TextureStage::CombineMode)((tex._flags & ShaderKey::TF_COMBINE_ALPHA_MODE_MASK) >> ShaderKey::TF_COMBINE_ALPHA_MODE_SHIFT);
      if (combine_rgb == TextureStage::CM_dot3_rgba) {
        text << "\t result = ";
        text << combine_mode_as_string(tex, combine_rgb, false, i);
        text << ";\n";
      } else {
        text << "\t result.rgb = ";
        text << combine_mode_as_string(tex, combine_rgb, false, i);
        text << ";\n\t result.a = ";
        text << combine_mode_as_string(tex, combine_alpha, true, i);
        text << ";\n";
      }
      if (tex._flags & ShaderKey::TF_rgb_scale_2) {
        text << "\t result.rgb *= 2;\n";
      }
      if (tex._flags & ShaderKey::TF_rgb_scale_4) {
        text << "\t result.rgb *= 4;\n";
      }
      if (tex._flags & ShaderKey::TF_alpha_scale_2) {
        text << "\t result.a *= 2;\n";
      }
      if (tex._flags & ShaderKey::TF_alpha_scale_4) {
        text << "\t result.a *= 4;\n";
      }
      break;
    case TextureStage::M_blend_color_scale:
      text << "\t result.rgb = lerp(result.rgb, texcolor_" << i << ".rgb * attr_colorscale.rgb, tex" << i << ".rgb);\n";
      if (key._calc_primary_alpha) {
        text << "\t result.a *= texcolor_" << i << ".a * attr_colorscale.a;\n";
      }
      break;
    default:
      break;
    }
    if (tex._flags & ShaderKey::TF_saved_result) {
      text << "\t last_saved_result = result;\n";
    }
  }

  if (key._alpha_test_mode != RenderAttrib::M_none) {
    text << "\t // Shader includes alpha test:\n";
    double ref = key._alpha_test_ref;
    switch (key._alpha_test_mode) {
    case RenderAttrib::M_never:
      text << "\t discard;\n";
      break;
    case RenderAttrib::M_less:
      text << "\t if (result.a >= " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_equal:
      text << "\t if (result.a != " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_less_equal:
      text << "\t if (result.a > " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_greater:
      text << "\t if (result.a <= " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_not_equal:
      text << "\t if (result.a == " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_greater_equal:
      text << "\t if (result.a < " << ref << ") discard;\n";
      break;
    case RenderAttrib::M_none:
    case RenderAttrib::M_always:
      break;
    }
  }

  if (key._outputs & AuxBitplaneAttrib::ABO_glow) {
    if (key._texture_flags & ShaderKey::TF_map_glow) {
      text << "\t result.a = tex" << map_index_glow << ".a;\n";
    } else {
      text << "\t result.a = 0.5;\n";
    }
  }
  if (key._outputs & AuxBitplaneAttrib::ABO_aux_glow) {
    if (key._texture_flags & ShaderKey::TF_map_glow) {
      text << "\t o_aux.a = tex" << map_index_glow << ".a;\n";
    } else {
      text << "\t o_aux.a = 0.5;\n";
    }
  }

  if (have_specular) {
    if (key._material_flags & Material::F_specular) {
      text << "\t tot_specular *= attr_material[3];\n";
    }
    if (key._texture_flags & ShaderKey::TF_map_gloss) {
      text << "\t tot_specular *= tex" << map_index_gloss << ".a;\n";
    }
    text << "\t result.rgb = result.rgb + tot_specular.rgb;\n";
  }
  if (key._light_ramp != nullptr) {
    switch (key._light_ramp->get_mode()) {
    case LightRampAttrib::LRT_hdr0:
      text << "\t result.rgb = (result*result*result + result*result + result) / (result*result*result + result*result + result + 1);\n";
      break;
    case LightRampAttrib::LRT_hdr1:
      text << "\t result.rgb = (result*result + result) / (result*result + result + 1);\n";
      break;
    case LightRampAttrib::LRT_hdr2:
      text << "\t result.rgb = result / (result + 1);\n";
      break;
    default: break;
    }
  }

  // Apply fog.
  if (key._fog_mode != 0) {
    Fog::Mode fog_mode = (Fog::Mode)(key._fog_mode - 1);
    switch (fog_mode) {
    case Fog::M_linear:
      text << "\t result.rgb = lerp(attr_fogcolor.rgb, result.rgb, saturate((attr_fog.z - l_hpos.z) * attr_fog.w));\n";
      break;
    case Fog::M_exponential: // 1.442695f = 1 / log(2)
      text << "\t result.rgb = lerp(attr_fogcolor.rgb, result.rgb, saturate(exp2(attr_fog.x * l_hpos.z * -1.442695f)));\n";
      break;
    case Fog::M_exponential_squared:
      text << "\t result.rgb = lerp(attr_fogcolor.rgb, result.rgb, saturate(exp2(attr_fog.x * attr_fog.x * l_hpos.z * l_hpos.z * -1.442695f)));\n";
      break;
    }
  }

  // The multiply is a workaround for a radeon driver bug.  It's annoying as
  // heck, since it produces an extra instruction.
  text << "\t o_color = result * 1.000001;\n";
  if (key._alpha_test_mode != RenderAttrib::M_none) {
    text << "\t // Shader subsumes normal alpha test.\n";
  }
  if (key._disable_alpha_write) {
    text << "\t // Shader disables alpha write.\n";
  }
  text << "}\n";

  if (pgraphnodes_cat.is_spam()) {
    pgraphnodes_cat.spam() << "Generated shader:\n"
      << text.str() << "\n";
  }

  // Insert the shader into the shader attrib.
  PT(Shader) shader = Shader::make(text.str(), Shader::SL_Cg);
  nassertr(shader != nullptr, nullptr);

  CPT(RenderAttrib) shattr = ShaderAttrib::make(shader);
  if (key._alpha_test_mode != RenderAttrib::M_none) {
    shattr = DCAST(ShaderAttrib, shattr)->set_flag(ShaderAttrib::F_subsume_alpha_test, true);
  }
  if (key._disable_alpha_write) {
    shattr = DCAST(ShaderAttrib, shattr)->set_flag(ShaderAttrib::F_disable_alpha_write, true);
  }

  reset_register_allocator();

  CPT(ShaderAttrib) attr = DCAST(ShaderAttrib, shattr);
  _generated_shaders[key] = attr;
  return attr;
}

/**
 * This 'synthesizes' a combine mode into a string.
 */
string ShaderGenerator::
combine_mode_as_string(const ShaderKey::TextureInfo &info, TextureStage::CombineMode c_mode, bool alpha, short texindex) {
  std::ostringstream text;
  switch (c_mode) {
  case TextureStage::CM_modulate:
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << " * ";
    text << combine_source_as_string(info, 1, alpha, texindex);
    break;
  case TextureStage::CM_add:
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << " + ";
    text << combine_source_as_string(info, 1, alpha, texindex);
    break;
  case TextureStage::CM_add_signed:
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << " + ";
    text << combine_source_as_string(info, 1, alpha, texindex);
    if (alpha) {
      text << " - 0.5";
    } else {
      text << " - float3(0.5, 0.5, 0.5)";
    }
    break;
  case TextureStage::CM_interpolate:
    text << "lerp(";
    text << combine_source_as_string(info, 1, alpha, texindex);
    text << ", ";
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << ", ";
    text << combine_source_as_string(info, 2, alpha, texindex);
    text << ")";
    break;
  case TextureStage::CM_subtract:
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << " - ";
    text << combine_source_as_string(info, 1, alpha, texindex);
    break;
  case TextureStage::CM_dot3_rgb:
  case TextureStage::CM_dot3_rgba:
    text << "4 * dot(";
    text << combine_source_as_string(info, 0, alpha, texindex);
    text << " - float3(0.5), ";
    text << combine_source_as_string(info, 1, alpha, texindex);
    text << " - float3(0.5))";
    break;
  case TextureStage::CM_replace:
  default: // Not sure if this is correct as default value.
    text << combine_source_as_string(info, 0, alpha, texindex);
    break;
  }
  return text.str();
}

/**
 * This 'synthesizes' a combine source into a string.
 */
string ShaderGenerator::
combine_source_as_string(const ShaderKey::TextureInfo &info, short num, bool alpha, short texindex) {
  TextureStage::CombineSource c_src;
  TextureStage::CombineOperand c_op;
  if (!alpha) {
    c_src = UNPACK_COMBINE_SRC(info._combine_rgb, num);
    c_op = UNPACK_COMBINE_OP(info._combine_rgb, num);
  } else {
    c_src = UNPACK_COMBINE_SRC(info._combine_alpha, num);
    c_op = UNPACK_COMBINE_OP(info._combine_alpha, num);
  }
  std::ostringstream csource;
  if (c_op == TextureStage::CO_one_minus_src_color ||
      c_op == TextureStage::CO_one_minus_src_alpha) {
    csource << "saturate(1.0f - ";
  }
  switch (c_src) {
    case TextureStage::CS_undefined:
    case TextureStage::CS_texture:
      csource << "tex" << texindex;
      break;
    case TextureStage::CS_constant:
      csource << "texcolor_" << texindex;
      break;
    case TextureStage::CS_primary_color:
      csource << "primary_color";
      break;
    case TextureStage::CS_previous:
      csource << "result";
      break;
    case TextureStage::CS_constant_color_scale:
      csource << "attr_colorscale";
      break;
    case TextureStage::CS_last_saved_result:
      csource << "last_saved_result";
      break;
  }
  if (c_op == TextureStage::CO_one_minus_src_color ||
      c_op == TextureStage::CO_one_minus_src_alpha) {
    csource << ")";
  }
  if (c_op == TextureStage::CO_src_color || c_op == TextureStage::CO_one_minus_src_color) {
    csource << ".rgb";
  } else {
    csource << ".a";
    if (!alpha) {
      // Dunno if it's legal in the FPP at all, but let's just allow it.
      return "float3(" + csource.str() + ")";
    }
  }
  return csource.str();
}

/**
 * Returns 1D, 2D, 3D or CUBE, depending on the given texture type.
 */
const char *ShaderGenerator::
texture_type_as_string(Texture::TextureType ttype) {
  switch (ttype) {
    case Texture::TT_1d_texture:
      return "1D";
      break;
    case Texture::TT_2d_texture:
      return "2D";
      break;
    case Texture::TT_3d_texture:
      return "3D";
      break;
    case Texture::TT_cube_map:
      return "CUBE";
      break;
    case Texture::TT_2d_texture_array:
      return "2DARRAY";
      break;
    default:
      pgraphnodes_cat.error() << "Unsupported texture type!\n";
      return "2D";
  }
}

/**
 * Initializes the ShaderKey to the empty state.
 */
ShaderGenerator::ShaderKey::
ShaderKey() :
  _color_type(ColorAttrib::T_vertex),
  _material_flags(0),
  _texture_flags(0),
  _lighting(false),
  _have_separate_ambient(false),
  _fog_mode(0),
  _outputs(0),
  _calc_primary_alpha(false),
  _disable_alpha_write(false),
  _alpha_test_mode(RenderAttrib::M_none),
  _alpha_test_ref(0.0),
  _num_clip_planes(0),
  _light_ramp(nullptr) {
}

/**
 * Returns true if this ShaderKey sorts less than the other one.  This is an
 * arbitrary, but consistent ordering.
 */
bool ShaderGenerator::ShaderKey::
operator < (const ShaderKey &other) const {
  if (_anim_spec != other._anim_spec) {
    return _anim_spec < other._anim_spec;
  }
  if (_color_type != other._color_type) {
    return _color_type < other._color_type;
  }
  if (_material_flags != other._material_flags) {
    return _material_flags < other._material_flags;
  }
  if (_texture_flags != other._texture_flags) {
    return _texture_flags < other._texture_flags;
  }
  if (_textures.size() != other._textures.size()) {
    return _textures.size() < other._textures.size();
  }
  for (size_t i = 0; i < _textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = _textures[i];
    const ShaderKey::TextureInfo &other_tex = other._textures[i];
    if (tex._texcoord_name != other_tex._texcoord_name) {
      return tex._texcoord_name < other_tex._texcoord_name;
    }
    if (tex._type != other_tex._type) {
      return tex._type < other_tex._type;
    }
    if (tex._mode != other_tex._mode) {
      return tex._mode < other_tex._mode;
    }
    if (tex._gen_mode != other_tex._gen_mode) {
      return tex._gen_mode < other_tex._gen_mode;
    }
    if (tex._flags != other_tex._flags) {
      return tex._flags < other_tex._flags;
    }
    if (tex._combine_rgb != other_tex._combine_rgb) {
      return tex._combine_rgb < other_tex._combine_rgb;
    }
    if (tex._combine_alpha != other_tex._combine_alpha) {
      return tex._combine_alpha < other_tex._combine_alpha;
    }
  }
  if (_lights.size() != other._lights.size()) {
    return _lights.size() < other._lights.size();
  }
  for (size_t i = 0; i < _lights.size(); ++i) {
    const ShaderKey::LightInfo &light = _lights[i];
    const ShaderKey::LightInfo &other_light = other._lights[i];
    if (light._type != other_light._type) {
      return light._type < other_light._type;
    }
    if (light._flags != other_light._flags) {
      return light._flags < other_light._flags;
    }
  }
  if (_lighting != other._lighting) {
    return _lighting < other._lighting;
  }
  if (_have_separate_ambient != other._have_separate_ambient) {
    return _have_separate_ambient < other._have_separate_ambient;
  }
  if (_fog_mode != other._fog_mode) {
    return _fog_mode < other._fog_mode;
  }
  if (_outputs != other._outputs) {
    return _outputs < other._outputs;
  }
  if (_calc_primary_alpha != other._calc_primary_alpha) {
    return _calc_primary_alpha < other._calc_primary_alpha;
  }
  if (_disable_alpha_write != other._disable_alpha_write) {
    return _disable_alpha_write < other._disable_alpha_write;
  }
  if (_alpha_test_mode != other._alpha_test_mode) {
    return _alpha_test_mode < other._alpha_test_mode;
  }
  if (_alpha_test_ref != other._alpha_test_ref) {
    return _alpha_test_ref < other._alpha_test_ref;
  }
  if (_num_clip_planes != other._num_clip_planes) {
    return _num_clip_planes < other._num_clip_planes;
  }
  return _light_ramp < other._light_ramp;
}

/**
 * Returns true if this ShaderKey is equal to the other one.
 */
bool ShaderGenerator::ShaderKey::
operator == (const ShaderKey &other) const {
  if (_anim_spec != other._anim_spec) {
    return false;
  }
  if (_color_type != other._color_type) {
    return false;
  }
  if (_material_flags != other._material_flags) {
    return false;
  }
  if (_texture_flags != other._texture_flags) {
    return false;
  }
  if (_textures.size() != other._textures.size()) {
    return false;
  }
  for (size_t i = 0; i < _textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = _textures[i];
    const ShaderKey::TextureInfo &other_tex = other._textures[i];
    if (tex._texcoord_name != other_tex._texcoord_name ||
        tex._type != other_tex._type ||
        tex._mode != other_tex._mode ||
        tex._gen_mode != other_tex._gen_mode ||
        tex._flags != other_tex._flags ||
        tex._combine_rgb != other_tex._combine_rgb ||
        tex._combine_alpha != other_tex._combine_alpha) {
      return false;
    }
  }
  if (_lights.size() != other._lights.size()) {
    return false;
  }
  for (size_t i = 0; i < _lights.size(); ++i) {
    const ShaderKey::LightInfo &light = _lights[i];
    const ShaderKey::LightInfo &other_light = other._lights[i];
    if (light._type != other_light._type ||
        light._flags != other_light._flags) {
      return false;
    }
  }
  return _lighting == other._lighting
      && _have_separate_ambient == other._have_separate_ambient
      && _fog_mode == other._fog_mode
      && _outputs == other._outputs
      && _calc_primary_alpha == other._calc_primary_alpha
      && _disable_alpha_write == other._disable_alpha_write
      && _alpha_test_mode == other._alpha_test_mode
      && _alpha_test_ref == other._alpha_test_ref
      && _num_clip_planes == other._num_clip_planes
      && _light_ramp == other._light_ramp;
}

#endif  // HAVE_CG
