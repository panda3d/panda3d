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
#include "renderModeAttrib.h"
#include "rescaleNormalAttrib.h"
#include "colorWriteAttrib.h"
#include "pointLight.h"
#include "sphereLight.h"
#include "spotlight.h"
#include "lightLensNode.h"
#include "lvector4.h"
#include "config_pgraphnodes.h"
#include "pStatTimer.h"
#include "bamCache.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "lightMutexHolder.h"
#include "virtualFileSystem.h"
#include "string_utils.h"
#include "shaderModuleSpirV.h"
#include "spirVBuilder.h"
#include "spirVModule.h"

namespace {
  using Id = SpirVId;
  using Pointer = SpirVPointer;
  using glsl::Value;
}

static ConfigVariableFilename shader_generator_cache_filename
("shader-generator-cache-filename", "",
 PRC_DESC("Sets the location of the cache file in which generated shaders are "
          "stored.  Leave empty to write it to the default location, in the "
          "model-cache.  To disable it, set shader-generator-cache-limit to 0."));

static ConfigVariableInt shader_generator_cache_limit
("shader-generator-cache-limit", 2048,
 PRC_DESC("Sets an upper limit on the number of shaders that are stored in the "
          "shader generator cache.  When the limit is reached, shaders that "
          "have not been used for a while are pruned.  This limit prevents the "
          "cache file from growing so large that it takes too long to load and "
          "write each run.  Set to 0 to disable the shader cache (it will be "
          "deleted upon the next run) or -1 not to have any limit at all."));

static ConfigVariableInt shader_generator_cache_max_age
("shader-generator-cache-max-age", 26 * 7 * 24 * 60 * 60,
 PRC_DESC("Sets a maximum, in seconds, on how long unused shaders remain in "
          "the generated shader cache.  The default is about 6 months.  Set to "
          "-1 to disable pruning the shader generator cache based on age."));

TypeHandle ShaderGenerator::_type_handle;

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

static std::string sgc_header("sgc\0\n\r", 6);

static PStatCollector lookup_collector("*:Munge:ShaderGen:Lookup");
static PStatCollector synthesize_collector("*:Munge:ShaderGen:Synthesize");

/**
 * Create a ShaderGenerator.  This has no state, except possibly to cache
 * certain results.  The parameter that must be passed is the GSG to which the
 * shader generator belongs.
 */
ShaderGenerator::
ShaderGenerator(const GraphicsStateGuardianBase *gsg) {
  uint64_t caps = gsg->get_supported_shader_capabilities();

  // Can we use point coordinates provided by the driver?
  _use_pointcoord = (caps & Shader::C_point_coord) != 0;

  // Do we want to use the ARB_shadow extension?  This also allows us to use
  // hardware shadows PCF.
  _use_shadow_filter = (caps & Shader::C_shadow_samplers) != 0 && gsg->get_supports_shadow_filter();

  // How many joints can we pass to the shader?  On older hardware (and DX9)
  // we are limited to 256 vectors, giving us only 85 3x4 matrices.
  if (gsg->_max_vertex_shader_parameter_vectors >= 500) {
    _num_indexed_transforms = 120;
  }
  else if (gsg->_max_vertex_shader_parameter_vectors >= 250) {
    _num_indexed_transforms = 80;
  }
  else if (gsg->_max_vertex_shader_parameter_vectors >= 128) {
    _num_indexed_transforms = 36;
  }
  else {
    _num_indexed_transforms = 4;
  }

  Filename fn = shader_generator_cache_filename;
  fn.set_binary();
  if (fn.empty()) {
    BamCache *cache = BamCache::get_global_ptr();
    if (cache != nullptr && cache->get_active()) {
      fn = Filename(cache->get_root(), "genshader.boo");
      fn.set_binary();
      _cache_filename = fn;
      reload_cache();
    }
  }
  else {
    _cache_filename = fn;
    reload_cache();
  }
}

/**
 * Destroy a ShaderGenerator.
 */
ShaderGenerator::
~ShaderGenerator() {
  flush_cache();
}

/**
 * Reloads the generated shader cache from disk.  The results will be combined
 * with the current generated shader map, you can call clear_generated_shaders
 * first to just get the disk cache.
 *
 * @since 1.11.0
 */
bool ShaderGenerator::
reload_cache() {
  if (shader_generator_cache_limit == 0) {
    return true;
  }

  LightMutexHolder holder(_lock);

  TrueClock *clock = TrueClock::get_global_ptr();
  double begin_time = clock->get_short_raw_time();

  DatagramInputFile din;
  if (!din.open(_cache_filename)) {
    if (pgraphnodes_cat.is_debug()) {
      pgraphnodes_cat.debug()
        << "Could not read shader generator cache file: "
        << _cache_filename << "\n";
    }
    return false;
  }

  std::string head;
  if (!din.read_header(head, sgc_header.size())) {
    if (pgraphnodes_cat.is_debug()) {
      pgraphnodes_cat.debug()
        << _cache_filename << " is not a shader generator cache file.\n";
    }
    return false;
  }

  if (head != sgc_header) {
    if (pgraphnodes_cat.is_debug()) {
      pgraphnodes_cat.debug()
        << _cache_filename << " is not a shader generator cache file.\n";
    }
    return false;
  }

  BamReader reader(&din);
  if (!reader.init()) {
    return false;
  }

  unsigned int count = 0;

  // The cache furthermore just consists of pairs of a raw ShaderKey followed
  // by the generated ShaderAttrib.
  Datagram dg;
  while (din.get_datagram(dg)) {
    DatagramIterator scan(dg);

    BamEnums::BamObjectCode boc = (BamEnums::BamObjectCode)scan.get_uint8();
    if (boc != BamEnums::BOC_ignore) {
      pgraphnodes_cat.warning()
        << _cache_filename << " is corrupted: unexpected BOC 0x"
        << std::hex << (int)boc << std::dec << "\n";
      return false;
    }

    time_t age = (time_t)scan.get_uint32();

    ShaderKey key;
    key.read_datagram(scan);

    GeneratedShader &gen_shader = _generated_shaders[std::move(key)];
    gen_shader._last_use = std::max(age, gen_shader._last_use);

    if (gen_shader._shader != nullptr) {
      // Optimization: don't read shaders that are already in memory.
      reader.skip_object();
      continue;
    }

    TypedWritable *shader = reader.read_object();
    if (!shader->is_of_type(Shader::get_class_type())) {
      pgraphnodes_cat.warning()
        << _cache_filename << " is corrupted: expected Shader instead of "
        << shader->get_type() << "\n";
      return false;
    }

    gen_shader._shader = DCAST(Shader, shader);
    ++count;
  }

  if (!reader.resolve()) {
    pgraphnodes_cat.error()
      << "Unable to fully resolve shader generator cache file.\n";
    return false;
  }

  double end_time = clock->get_short_raw_time();
  double time_delta = end_time - begin_time;

  pgraphnodes_cat.info()
    << "Read " << count << " generated shaders from "
    << _cache_filename << " in " << (time_delta * 1000) << " ms\n";

  return true;
}

/**
 * Writes all generated shaders to the cache.
 *
 * @since 1.11.0
 */
bool ShaderGenerator::
flush_cache() {
  Filename cache_filename;
  TrueClock *clock;
  double begin_time;
  VirtualFileSystem *vfs;
  int limit;
  std::vector<std::pair<ShaderKey, GeneratedShader> > gen_shaders;
  {
    LightMutexHolder holder(_lock);
    if (_generated_shaders.empty()) {
      return true;
    }
    if (_cache_filename.empty()) {
      return true;
    }
    cache_filename = _cache_filename;

    vfs = VirtualFileSystem::get_global_ptr();

    limit = shader_generator_cache_limit;
    if (limit == 0) {
      vfs->delete_file(cache_filename);
      return true;
    }

    clock = TrueClock::get_global_ptr();
    begin_time = clock->get_short_raw_time();

    time_t min_last_use = 0;
    int max_age = shader_generator_cache_max_age;
    if (max_age >= 0) {
      min_last_use = time(nullptr) - max_age;
    }

    gen_shaders.reserve(_generated_shaders.size());
    GeneratedShaders::const_iterator it;
    for (const GeneratedShaders::value_type &entry : _generated_shaders) {
      // Entries without a shader are memoized generation failures; those are
      // not written out to the cache.
      if (entry.second._shader != nullptr &&
          entry.second._last_use >= min_last_use) {
        gen_shaders.push_back(entry);
      }
    }
  }

  unsigned int count = 0;
  {
    DatagramOutputFile dout;
    if (!dout.open(cache_filename)) {
      pgraphnodes_cat.error()
        << "Unable to open " << cache_filename << "\n";
      vfs->delete_file(cache_filename);
      return false;
    }

    if (!dout.write_header(sgc_header)) {
      pgraphnodes_cat.error()
        << "Unable to write to " << cache_filename << "\n";
      vfs->delete_file(cache_filename);
      return false;
    }

    BamWriter writer(&dout);
    if (!writer.init()) {
      vfs->delete_file(cache_filename);
      return false;
    }

    if (limit > 0 && gen_shaders.size() > (size_t)limit) {
      // We need to prune down the generated shader list.  Sort by time of last
      // use, in reverse order, so we can easily eliminate the oldest shaders.
      std::sort(gen_shaders.begin(), gen_shaders.end(),
        [=](std::pair<ShaderKey, GeneratedShader> &a, std::pair<ShaderKey, GeneratedShader> &b) {
          return a.second._last_use > b.second._last_use;
        });

      gen_shaders.resize((size_t)limit);
    }

    for (std::pair<ShaderKey, GeneratedShader> &entry : gen_shaders) {
      Datagram dg;
      dg.add_uint8(BamEnums::BOC_ignore);
      dg.add_uint32(entry.second._last_use);
      entry.first.write_datagram(dg);
      dout.put_datagram(dg);

      if (!writer.write_object(entry.second._shader)) {
        break;
      }
      ++count;
    }
  }

  double end_time = clock->get_short_raw_time();
  double time_delta = end_time - begin_time;

  pgraphnodes_cat.info()
    << "Wrote " << count << " generated shaders to "
    << cache_filename << " in " << (time_delta * 1000) << " ms\n";

  return true;
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

  // Are we writing to the color channel at all?
  bool have_color_write = false;
  const ColorWriteAttrib *color_write;
  rs->get_attrib_def(color_write);
  if (color_write->get_channels() != ColorWriteAttrib::C_off) {
    have_color_write = true;
  } else {
    key._flags |= ShaderKey::F_disable_color_write;

    // Subsume alpha test so we can eliminate the color output
    if (have_alpha_test) {
      key._flags |= (alpha_test->get_mode() & 0x7) << ShaderKey::F_ALPHA_TEST_SHIFT;
      key._alpha_test_ref = alpha_test->get_reference_alpha();
    }
  }

  if (have_alpha_blend || have_alpha_test) {
    key._flags |= ShaderKey::F_calc_primary_alpha;
  }

  // Decide what to send to the framebuffer alpha, if anything.
  const AuxBitplaneAttrib *aux_bitplane;
  rs->get_attrib_def(aux_bitplane);
  int outputs = aux_bitplane->get_outputs();

  if (outputs & AuxBitplaneAttrib::ABO_glow) {
    if (have_alpha_blend) {
      key._flags |= ShaderKey::F_disable_alpha_write;
    }
    else if (have_color_write) {
      key._flags |= ShaderKey::F_out_alpha_glow;
      if (have_alpha_test) {
        // Subsume the alpha test in our shader.
        key._flags |= (alpha_test->get_mode() & 0x7) << ShaderKey::F_ALPHA_TEST_SHIFT;
        key._alpha_test_ref = alpha_test->get_reference_alpha();
      }
    }
  }

  if (outputs & AuxBitplaneAttrib::ABO_aux_normal) {
    key._flags |= ShaderKey::F_out_aux_normal;
  }
  if (outputs & AuxBitplaneAttrib::ABO_aux_glow) {
    key._flags |= ShaderKey::F_out_aux_glow;
  }

  // Determine whether or not vertex colors or flat colors are present.
  const ColorAttrib *color;
  rs->get_attrib_def(color);
  if (color->get_color_type() == ColorAttrib::T_flat) {
    key._flags |= ShaderKey::F_flat_color;
  }
  else if (color->get_color_type() == ColorAttrib::T_vertex) {
    key._flags |= ShaderKey::F_vertex_color;
  }

  // Store the material flags (not the material values itself).
  const MaterialAttrib *material;
  rs->get_attrib_def(material);
  Material *mat = material->get_material();
  if (mat != nullptr) {
    // The next time the Material flags change, the Material should cause the
    // states to be rehashed.
    mat->mark_used_by_auto_shader();
    key._flags |= (mat->get_flags() & ShaderKey::F_MATERIAL_FLAGS_MASK);

    if ((key._flags & Material::F_base_color) != 0) {
      key._flags |= (Material::F_diffuse | Material::F_specular | Material::F_ambient);
      key._flags &= ~Material::F_base_color;
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

    key._flags |= ShaderKey::F_lighting;

    if (node->is_ambient_light()) {
      have_ambient = true;
      continue;
    }

    int light_flags = 0;

    if (node->is_of_type(LightLensNode::get_class_type())) {
      const LightLensNode *llnode = (const LightLensNode *)node;

      if (llnode->is_of_type(DirectionalLight::get_class_type())) {
        light_flags |= ShaderKey::LF_type_directional;
      }
      else if (llnode->is_of_type(Spotlight::get_class_type())) {
        light_flags |= ShaderKey::LF_type_spot;
      }
      else if (llnode->is_of_type(PointLight::get_class_type())) {
        if (llnode->is_of_type(SphereLight::get_class_type())) {
          light_flags |= ShaderKey::LF_type_sphere;
        }
        else {
          light_flags |= ShaderKey::LF_type_point;
        }
      }

      if (have_color_write && shader_attrib->auto_shadow_on()) {
        if (llnode->is_shadow_caster()) {
          light_flags |= ShaderKey::LF_has_shadows;

          if (_use_shadow_filter) {
            key._flags |= ShaderKey::F_use_shadow_filter;
          }
        }

        // Make sure that the next time the shadows are toggled on this
        // light, it triggers a state rehash.
        llnode->mark_used_by_auto_shader();
      }
      if (llnode->has_specular_color()) {
        light_flags |= ShaderKey::LF_has_specular_color;
      }
    }

    key._lights.push_back(light_flags);
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
                 (key._lights.empty() && (outputs & AuxBitplaneAttrib::ABO_aux_normal) == 0)) {
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
          (key._lights.empty() && (outputs & AuxBitplaneAttrib::ABO_aux_normal) == 0)) {
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
    case TextureStage::M_emission:
      info._flags = ShaderKey::TF_map_emission;
      break;
    case TextureStage::M_occlusion:
    case TextureStage::M_occlusion_metallic_roughness:
      info._flags = ShaderKey::TF_map_occlusion;
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
    if (stage->uses_color() || stage->involves_color_scale()) {
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
    if (key._texture_flags & ShaderKey::TF_map_occlusion) {
      key._flags |= ShaderKey::F_have_separate_ambient;
    }
    else if (key._flags & Material::F_ambient) {
      key._flags |= ShaderKey::F_have_separate_ambient;
    }
    else {
      if (key._flags & Material::F_diffuse) {
        key._flags |= ShaderKey::F_have_separate_ambient;
      }
    }
  }

  if (have_color_write && shader_attrib->auto_ramp_on()) {
    const LightRampAttrib *light_ramp;
    if (rs->get_attrib(light_ramp)) {
      key._light_ramp_mode = light_ramp->get_mode();
      key._light_ramp_level[0] = light_ramp->get_level(0);
      key._light_ramp_level[1] = light_ramp->get_level(1);
      key._light_ramp_threshold[0] = light_ramp->get_threshold(0);
      key._light_ramp_threshold[1] = light_ramp->get_threshold(1);
      if (key._flags & ShaderKey::F_lighting) {
        key._flags |= ShaderKey::F_have_separate_ambient;
      }
    }
  }

  // Check for clip planes.
  const ClipPlaneAttrib *clip_plane;
  rs->get_attrib_def(clip_plane);
  key._num_clip_planes = clip_plane->get_num_on_planes();

  // Check for fog.
  const FogAttrib *fog;
  if (have_color_write && rs->get_attrib(fog) && !fog->is_off()) {
    key._flags |= ((int)fog->get_fog()->get_mode() + 1) << ShaderKey::F_FOG_MODE_SHIFT;
  }

  // Must we calculate the point size in the shader?
  const RenderModeAttrib *render_mode;
  if (rs->get_attrib(render_mode) && render_mode->get_perspective()) {
    key._flags |= ShaderKey::F_perspective_points;
  }

  // This flag must be folded into the key so that the right shader is used
  // even if the graphics card / driver is switched.
  if (!_use_pointcoord && key.needs_point_coord()) {
    key._flags |= ShaderKey::F_calc_point_coord;
  }
}

/**
 * Analyzes the RenderState prior to shader generation.  The results of the
 * analysis are stored in instance variables of the Shader Generator.
 */
void ShaderGenerator::
analyze_renderstate(ShaderKey &key, const RenderState *rs,
                    const GeomVertexAnimationSpec &anim) {

  analyze_renderstate(key, rs);

  if (anim.get_animation_type() == GeomVertexAnimationSpec::AT_hardware) {
    if (anim.get_indexed_transforms()) {
      key._flags |= ShaderKey::F_indexed_transforms;
      key._num_anim_transforms = _num_indexed_transforms;
    } else {
      key._num_anim_transforms = std::min(4, anim.get_num_transforms());
    }
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
  LightMutexHolder holder2(_lock);
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
        if (si->second._attrib != state->_generated_shader) {
          state->_generated_shader = si->second._attrib;
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
  LightMutexHolder holder2(_lock);
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
 * Returns the varying location assigned to the given texture coordinate set,
 * or -1 if no varying was assigned for it.
 */
int ShaderGenerator::ShaderPlan::
find_texcoord_location(const InternalName *name) const {
  for (const auto &pair : _texcoord_locations) {
    if (pair.first == name) {
      return pair.second;
    }
  }
  return -1;
}

/**
 * Derives from the given key which intermediate results the vertex shader
 * needs to calculate and pass along to the fragment shader, and assigns the
 * varying locations.
 *
 * This must be a pure function of the key, so that the key uniquely
 * identifies the generated modules.
 */
ShaderGenerator::ShaderPlan ShaderGenerator::
make_plan(const ShaderKey &key) {
  ShaderPlan plan;

  // Figure out whether we need to calculate any of these variables.
  bool need_world_position = (key._num_clip_planes > 0);
  bool need_world_normal = false;
  bool need_eye_position = (key._flags & ShaderKey::F_lighting) != 0;
  bool need_eye_normal = !key._lights.empty() ||
                         (key._flags & ShaderKey::F_out_aux_normal) != 0;

  if (key.needs_eye_reflection()) {
    need_eye_position = true;
    need_eye_normal = true;
  }

  int next_location = 0;

  for (size_t i = 0; i < key._textures.size(); ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];

    switch (tex._gen_mode) {
    case TexGenAttrib::M_world_normal:
      need_world_normal = true;
      break;
    case TexGenAttrib::M_eye_normal:
      need_eye_normal = true;
      break;
    case TexGenAttrib::M_world_position:
      need_world_position = true;
      break;
    case TexGenAttrib::M_eye_position:
      need_eye_position = true;
      break;
    default:
      break;
    }

    if (tex._texcoord_name != nullptr &&
        plan.find_texcoord_location(tex._texcoord_name) < 0) {
      plan._texcoord_locations.push_back(
        std::make_pair(tex._texcoord_name.p(), next_location++));
    }
  }

  // Assign the remaining varying locations.
  if (key._texture_flags & ShaderKey::TF_map_normal) {
    plan._tangent_location = next_location++;
    plan._binormal_location = next_location++;
  }
  if (key.needs_color() && (key._flags & ShaderKey::F_vertex_color) != 0) {
    plan._color_location = next_location++;
  }
  if (need_world_position) {
    plan._world_position_location = next_location++;
  }
  if (need_world_normal) {
    plan._world_normal_location = next_location++;
  }
  if (need_eye_position) {
    plan._eye_position_location = next_location++;
  }
  if (need_eye_normal) {
    plan._eye_normal_location = next_location++;
  }
  if (key._texture_flags & ShaderKey::TF_map_height) {
    plan._eyevec_location = next_location++;
  }
  if ((key._flags & ShaderKey::F_FOG_MODE_MASK) != 0) {
    plan._hpos_location = next_location++;
  }
  for (size_t i = 0; i < key._lights.size(); ++i) {
    if (key._lights[i] & ShaderKey::LF_has_shadows) {
      plan._lightcoord_locations.push_back(next_location++);
    } else {
      plan._lightcoord_locations.push_back(-1);
    }
  }
  if (key._flags & ShaderKey::F_calc_point_coord) {
    plan._pointcoord_location = next_location++;
  }

  return plan;
}

/**
 * This is the routine that implements the next-gen fixed function pipeline by
 * synthesizing a shader.  It also takes care of setting up any buffers needed
 * to produce the requested effects.
 *
 * The shader is generated directly as a pair of SPIR-V modules, using the
 * SpirVBuilder, with the same (Cg-style) parameter names as the old
 * Cg-based generator used.
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
    analyze_renderstate(key, rs, anim);

    LightMutexHolder holder(_lock);
    GeneratedShaders::iterator si;
    si = _generated_shaders.find(key);
    if (si != _generated_shaders.end()) {
      // We've already generated a shader for this state.
      GeneratedShader &gen_shader = si->second;
      if (gen_shader._attrib == nullptr) {
        // We have a shader, but haven't created a ShaderAttrib for it yet.
        CPT(ShaderAttrib) attrib = make_attrib(key, gen_shader._shader);
        gen_shader._attrib = std::move(attrib);
      }
      gen_shader._last_use = time(nullptr);
      return gen_shader._attrib;
    }
  }

  PStatTimer timer(synthesize_collector);

  if (pgraphnodes_cat.is_debug()) {
    pgraphnodes_cat.debug()
      << "Generating shader for render state " << rs << ":\n";
    rs->write(pgraphnodes_cat.debug(false), 2);
  }

  // Generate the vertex and fragment modules.  Each is generated
  // independently, from the key and the plan alone, so that they may one day
  // be generated in parallel.
  ShaderPlan plan = make_plan(key);
  PT(ShaderModule) vertex_module = generate_vertex_shader(key, plan);
  PT(ShaderModule) fragment_module = generate_fragment_shader(key, plan);
  nassertr(vertex_module != nullptr, nullptr);
  nassertr(fragment_module != nullptr, nullptr);

  // Insert the modules into a shader.
  PT(Shader) shader = new Shader(Shader::SL_Cg);
  shader->get_prepare_shader_pcollector() =
    PStatCollector("Draw:Prepare:Shader:generated-shader");
  if (!shader->add_module(std::move(vertex_module)) ||
      !shader->add_module(std::move(fragment_module)) ||
      !shader->link()) {
    pgraphnodes_cat.error()
      << "Failed to link generated shader for render state " << *rs << "\n";

    // Memoize the failure with an empty ShaderAttrib, so that we don't
    // re-attempt (and re-log) the generation every time an object with this
    // state is rendered.  Entries without a shader are not written to the
    // on-disk cache, so the generation is re-attempted in the next session.
    CPT(ShaderAttrib) attr = DCAST(ShaderAttrib, ShaderAttrib::make());
    LightMutexHolder holder(_lock);
    GeneratedShader &gen_shader = _generated_shaders[key];
    gen_shader._attrib = attr;
    gen_shader._last_use = time(nullptr);
    return attr;
  }

  shader->_subsumes_alpha_test = (key._flags & ShaderKey::F_ALPHA_TEST_MASK) != 0;

  CPT(ShaderAttrib) attr = make_attrib(key, shader);

  LightMutexHolder holder(_lock);
  GeneratedShader &gen_shader = _generated_shaders[key];
  gen_shader._shader = std::move(shader);
  gen_shader._attrib = attr;
  gen_shader._last_use = time(nullptr);
  return attr;
}

/**
 * Constructs a ShaderAttrib around the given generated Shader object.
 */
CPT(ShaderAttrib) ShaderGenerator::
make_attrib(const ShaderKey &key, Shader *shader) {
  CPT(RenderAttrib) shattr = ShaderAttrib::make(shader);
  int flags = 0;
  if ((key._flags & ShaderKey::F_ALPHA_TEST_MASK) != 0) {
    flags |= ShaderAttrib::F_subsume_alpha_test;
  }
  if (key._flags & ShaderKey::F_disable_alpha_write) {
    flags |= ShaderAttrib::F_disable_alpha_write;
  }
  if (key._flags & ShaderKey::F_perspective_points) {
    flags |= ShaderAttrib::F_shader_point_size;
  }
  if (key._num_anim_transforms != 0) {
    flags |= ShaderAttrib::F_hardware_skinning;
  }
  if (flags != 0) {
    shattr = DCAST(ShaderAttrib, shattr)->set_flag(flags, true);
  }
  return DCAST(ShaderAttrib, shattr);
}

/**
 * Generates the vertex shader module for the given key and plan.
 */
PT(ShaderModule) ShaderGenerator::
generate_vertex_shader(const ShaderKey &key, const ShaderPlan &plan) const {
  SpirVModule module;
  module.add_capability(spv::CapabilityShader);
  module.set_memory_model(spv::AddressingModelLogical, spv::MemoryModelGLSL450);

  SpirVBuilder b = module.make_entry_point(spv::ExecutionModelVertex, "main");
  Id function_id = b.get_current_function_id();

  const ShaderType *vec3_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  const ShaderType *mat3x3_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 3));
  const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
  const ShaderType *mat4x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));

  bool need_point_size = (key._flags & ShaderKey::F_perspective_points) != 0;
  bool need_normal = plan.has_eyevec() || plan.has_world_normal() ||
                     plan.has_eye_normal();
  bool need_model_to_view = plan.has_eye_position() || plan.has_tangents() ||
                            need_point_size;

  // Determine the names of the tangent and binormal vertex columns, without
  // the "vtx_" prefix, which are based on the texture coordinate set of the
  // first stage that has a normal or height map.
  std::string tangent_input, binormal_input;
  for (const ShaderKey::TextureInfo &tex : key._textures) {
    if ((tex._flags & (ShaderKey::TF_map_normal | ShaderKey::TF_map_height)) != 0) {
      PT(InternalName) tangent_name = InternalName::get_tangent();
      PT(InternalName) binormal_name = InternalName::get_binormal();

      if (tex._texcoord_name != nullptr &&
          tex._texcoord_name != InternalName::get_texcoord()) {
        tangent_name = tangent_name->append(tex._texcoord_name->get_basename());
        binormal_name = binormal_name->append(tex._texcoord_name->get_basename());
      }

      tangent_input = tangent_name->join("_");
      binormal_input = binormal_name->join("_");
      break;
    }
  }

  // Vertex inputs.
  int next_input = 0;
  Pointer vtx_position = b.define_input(vec4_type, "vtx_position", next_input++);

  Pointer vtx_normal;
  if (need_normal) {
    vtx_normal = b.define_input(vec3_type, "vtx_normal", next_input++);
  }
  Pointer vtx_color;
  if (plan.has_color()) {
    vtx_color = b.define_input(vec4_type, "vtx_color", next_input++);
  }
  Pointer vtx_tangent, vtx_binormal;
  if (!tangent_input.empty()) {
    vtx_tangent = b.define_input(vec4_type,
      "vtx_" + tangent_input, next_input++);
    vtx_binormal = b.define_input(vec4_type,
      "vtx_" + binormal_input, next_input++);
  }
  Pointer vtx_transform_weight, vtx_transform_index;
  if (key._num_anim_transforms > 0) {
    vtx_transform_weight = b.define_input(vec4_type,
      "vtx_transform_weight", next_input++);
    if (key._flags & ShaderKey::F_indexed_transforms) {
      const ShaderType *ivec4_type = ShaderType::register_type(
        ShaderType::Vector(ShaderType::ST_int, 4));
      vtx_transform_index = b.define_input(ivec4_type,
        "vtx_transform_index", next_input++);
    }
  }
  pvector<std::pair<Pointer, Pointer> > texcoord_vars;
  for (const auto &pair : plan._texcoord_locations) {
    std::string name = pair.first->join("_");
    Pointer in_var = b.define_input(vec4_type, "vtx_" + name, next_input++);
    Pointer out_var = b.define_output(vec4_type, "l_" + name, pair.second);
    texcoord_vars.push_back(std::make_pair(in_var, out_var));
  }

  // Uniforms.
  Pointer trans_model_to_world, trans_model_to_view, tpose_view_to_model;
  Pointer mspos_view, attr_pointparams, tbl_transforms;
  if (plan.has_world_position() || plan.has_world_normal()) {
    trans_model_to_world = b.define_uniform(mat3x4_type, "trans_model_to_world");
  }
  if (need_model_to_view) {
    trans_model_to_view = b.define_uniform(mat3x4_type, "trans_model_to_view");
  }
  if (plan.has_eye_normal()) {
    tpose_view_to_model = b.define_uniform(mat3x3_type, "tpose_view_to_model");
  }
  if (plan.has_eyevec()) {
    mspos_view = b.define_uniform(vec4_type, "mspos_view");
  }
  size_t num_lightcoords = plan._lightcoord_locations.size();
  pvector<Pointer> mat_shadow(num_lightcoords);
  for (size_t i = 0; i < num_lightcoords; ++i) {
    if (plan._lightcoord_locations[i] >= 0) {
      mat_shadow[i] = b.define_uniform(mat4x4_type, "mat_shadow_" + format_string(i));
    }
  }
  if (key._num_anim_transforms > 0) {
    const ShaderType *table_type = ShaderType::register_type(
      ShaderType::Array(mat3x4_type, key._num_anim_transforms));
    tbl_transforms = b.define_uniform(table_type, "tbl_transforms");
  }
  if (need_point_size || plan.calc_point_coord()) {
    attr_pointparams = b.define_uniform(vec3_type, "attr_pointparams");
  }
  Pointer mat_modelproj = b.define_uniform(mat4x4_type, "mat_modelproj");

  // Outputs.
  Pointer o_position = b.ensure_builtin_output(spv::BuiltInPosition);
  Pointer o_point_size;
  if (need_point_size) {
    o_point_size = b.ensure_builtin_output(spv::BuiltInPointSize);
  }
  Pointer o_hpos, o_world_position, o_world_normal, o_eye_position, o_eye_normal;
  Pointer o_tangent, o_binormal, o_color, o_eyevec, o_pointcoord;
  if (plan.has_hpos()) {
    o_hpos = b.define_output(vec4_type, "l_hpos", plan._hpos_location);
  }
  if (plan.has_world_position()) {
    o_world_position = b.define_output(vec4_type,
      "l_world_position", plan._world_position_location);
  }
  if (plan.has_world_normal()) {
    o_world_normal = b.define_output(vec4_type,
      "l_world_normal", plan._world_normal_location);
  }
  if (plan.has_eye_position()) {
    o_eye_position = b.define_output(vec4_type,
      "l_eye_position", plan._eye_position_location);
  }
  if (plan.has_eye_normal()) {
    o_eye_normal = b.define_output(vec3_type,
      "l_eye_normal", plan._eye_normal_location);
  }
  if (plan.has_tangents()) {
    o_tangent = b.define_output(vec4_type, "l_tangent", plan._tangent_location);
    o_binormal = b.define_output(vec4_type, "l_binormal", plan._binormal_location);
  }
  if (plan.has_color()) {
    o_color = b.define_output(vec4_type, "l_color", plan._color_location);
  }
  if (plan.has_eyevec()) {
    o_eyevec = b.define_output(vec3_type, "l_eyevec", plan._eyevec_location);
  }
  if (plan.calc_point_coord()) {
    o_pointcoord = b.define_output(vec3_type,
      "l_pointcoord", plan._pointcoord_location);
  }
  pvector<Pointer> o_lightcoord(num_lightcoords);
  for (size_t i = 0; i < num_lightcoords; ++i) {
    if (plan._lightcoord_locations[i] >= 0) {
      o_lightcoord[i] = b.define_output(vec4_type,
        "l_lightcoord" + format_string(i), plan._lightcoord_locations[i]);
    }
  }

  // Now generate the function body.
  auto position = vtx_position.load();
  Value normal;
  if (vtx_normal) {
    normal = vtx_normal.load();
  }
  Value tangent, binormal;
  if (vtx_tangent) {
    tangent = vtx_tangent.load().get_xyz();
    binormal = vtx_binormal.load().get_xyz();
  }

  if (key._num_anim_transforms > 0) {
    // Hardware skinning: blend the transform table entries together using
    // the given weights, then transform the position and normal by it.
    auto weights = vtx_transform_weight.load();
    Value indices;
    int num_taps = 4;
    if (vtx_transform_index) {
      indices = vtx_transform_index.load();
    } else {
      num_taps = std::min(4, key._num_anim_transforms);
    }

    // We can't add matrices together, so we do it per row.
    Value row0, row1, row2;
    for (int i = 0; i < num_taps; ++i) {
      Pointer entry;
      if (vtx_transform_index) {
        entry = tbl_transforms[indices[(uint32_t)i]];
      } else {
        entry = tbl_transforms[(uint32_t)i];
      }
      auto matrix = entry.load();
      matrix *= weights[(uint32_t)i];

      if (i == 0) {
        row0 = matrix[0];
        row1 = matrix[1];
        row2 = matrix[2];
      } else {
        row0 += matrix[0];
        row1 += matrix[1];
        row2 += matrix[2];
      }
    }
    auto vtxmat = mat3x4(row0, row1, row2);

    position = vec4(position * vtxmat, 1.0f);
    if (vtx_normal) {
      normal = normal * mat3(vtxmat);
    }
  }

  auto clip_pos = position * mat_modelproj.load();
  o_position.store(clip_pos);
  if (o_hpos) {
    o_hpos.store(clip_pos);
  }

  if (trans_model_to_world) {
    auto model_to_world = trans_model_to_world.load();
    if (o_world_position) {
      o_world_position.store(vec4(position * model_to_world, 1.0f));
    }
    if (o_world_normal) {
      auto world_normal = vec4(normal, 0.0f) * model_to_world;
      o_world_normal.store(vec4(world_normal, 0.0f));
    }
  }

  Value model_to_view;
  if (trans_model_to_view) {
    model_to_view = trans_model_to_view.load();
  }

  Value eye_position3, eye_position4;
  if (plan.has_eye_position() || need_point_size) {
    eye_position3 = position * model_to_view;
    if (o_eye_position) {
      eye_position4 = vec4(eye_position3, 1.0f);
      o_eye_position.store(eye_position4);
    }
  }

  Value pointparams;
  if (attr_pointparams) {
    pointparams = attr_pointparams.load();
  }
  Value point_size;
  if (need_point_size) {
    point_size = pointparams[1] + pointparams[2] / length(eye_position3);
    o_point_size.store(point_size);
  }
  if (plan.calc_point_coord()) {
    // The driver does not provide point coordinates, so we pass along what
    // the fragment shader needs to calculate them from the position.
    auto double_w = clip_pos[3] * 2.0f;
    auto xy = clip_pos.swizzle({0, 1}) / double_w + 0.5f;
    Value inv_size;
    if (need_point_size) {
      inv_size = 1.0f / point_size;
    } else {
      // Static point size (non-perspective points)
      inv_size = 1.0f / pointparams[0];
    }
    o_pointcoord.store(vec3(xy, inv_size));
  }

  // Pass through all texcoord inputs as-is.
  for (const auto &pair : texcoord_vars) {
    pair.second.store(pair.first.load());
  }
  if (vtx_color) {
    o_color.store(vtx_color.load());
  }
  if (plan.has_tangents()) {
    auto model_to_view3 = mat3(model_to_view);
    auto vs_tangent = normalize(tangent * model_to_view3);
    o_tangent.store(vec4(vs_tangent, 0.0f));
    auto vs_binormal = normalize(-binormal * model_to_view3);
    o_binormal.store(vec4(vs_binormal, 0.0f));
  }
  for (size_t i = 0; i < num_lightcoords; ++i) {
    if (o_lightcoord[i]) {
      o_lightcoord[i].store(eye_position4 * mat_shadow[i].load());
    }
  }
  if (plan.has_eyevec()) {
    auto eyedir = mspos_view.load().get_xyz() - position.get_xyz();
    auto eyevec = vec3(dot(tangent, eyedir),
                       dot(binormal, eyedir),
                       dot(normal, eyedir));
    o_eyevec.store(normalize(eyevec));
  }
  if (plan.has_eye_normal()) {
    o_eye_normal.store(normalize(normal * tpose_view_to_model.load()));
  }

  b.op_return();

  ShaderModuleSpirV *shader_module = new ShaderModuleSpirV(Shader::Stage::VERTEX, std::move(module));
  if (pgraphnodes_cat.is_spam()) {
    std::ostream &out = pgraphnodes_cat.spam()
      << "Generated vertex shader:\n";
    shader_module->_instructions.disassemble(out);
  }
  return shader_module;
}

/**
 * Generates the fragment shader module for the given key and plan.
 */
PT(ShaderModule) ShaderGenerator::
generate_fragment_shader(const ShaderKey &key, const ShaderPlan &plan) const {
  SpirVModule module;
  module.add_capability(spv::CapabilityShader);
  module.set_memory_model(spv::AddressingModelLogical, spv::MemoryModelGLSL450);

  SpirVBuilder b = module.make_entry_point(spv::ExecutionModelFragment, "main");
  Id function_id = b.get_current_function_id();

  module.add_execution_mode(function_id, spv::ExecutionModeOriginUpperLeft);

  const ShaderType *vec2_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));
  const ShaderType *vec3_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  const ShaderType *mat3x3_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 3));
  const ShaderType *mat4x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));

  bool have_fog = (key._flags & ShaderKey::F_FOG_MODE_MASK) != 0;
  bool need_heightmap = (key._texture_flags & ShaderKey::TF_map_height) != 0;
  bool have_color_write = (key._flags & ShaderKey::F_disable_color_write) == 0;
  bool have_aux = (key._flags & (ShaderKey::F_out_aux_normal | ShaderKey::F_out_aux_glow)) != 0;
  bool use_shadow_filter = (key._flags & ShaderKey::F_use_shadow_filter) != 0;
  bool have_lighting = (key._flags & ShaderKey::F_lighting) != 0;
  bool have_separate_ambient = (key._flags & ShaderKey::F_have_separate_ambient) != 0;
  bool have_specular = key.has_specular();
  bool need_eye_reflection = key.needs_eye_reflection();
  size_t num_textures = key._textures.size();

  // Locate the special maps within the texture list.
  int map_index_glow = key.find_map_index(ShaderKey::TF_map_glow);
  int map_index_gloss = key.find_map_index(ShaderKey::TF_map_gloss);
  int map_index_emission = key.find_map_index(ShaderKey::TF_map_emission);

  // Inputs.
  Pointer l_hpos, l_world_position, l_world_normal, l_eye_position, l_eye_normal;
  Pointer l_tangent, l_binormal, l_color, l_eyevec, l_pointcoord;
  Pointer pointcoord_builtin, fragcoord_builtin;
  if (plan.has_hpos()) {
    l_hpos = b.define_input(vec4_type, "l_hpos", plan._hpos_location);
  }
  if (key.needs_point_coord()) {
    if (!plan.calc_point_coord()) {
      // The driver provides point coordinates.
      pointcoord_builtin = b.ensure_builtin_input(spv::BuiltInPointCoord);
    } else {
      // We need to calculate them from the fragment coordinates.
      l_pointcoord = b.define_input(vec3_type,
        "l_pointcoord", plan._pointcoord_location);
      fragcoord_builtin = b.ensure_builtin_input(spv::BuiltInFragCoord);
    }
  }
  if (plan.has_world_position()) {
    l_world_position = b.define_input(vec4_type,
      "l_world_position", plan._world_position_location);
  }
  if (plan.has_world_normal()) {
    l_world_normal = b.define_input(vec4_type,
      "l_world_normal", plan._world_normal_location);
  }
  if (plan.has_eye_position()) {
    l_eye_position = b.define_input(vec4_type,
      "l_eye_position", plan._eye_position_location);
  }
  if (plan.has_eye_normal()) {
    l_eye_normal = b.define_input(vec3_type,
      "l_eye_normal", plan._eye_normal_location);
  }
  pvector<Pointer> texcoord_inputs;
  for (const auto &pair : plan._texcoord_locations) {
    texcoord_inputs.push_back(
      b.define_input(vec4_type, "l_" + pair.first->join("_"), pair.second));
  }
  if (plan.has_tangents()) {
    l_tangent = b.define_input(vec4_type, "l_tangent", plan._tangent_location);
    l_binormal = b.define_input(vec4_type, "l_binormal", plan._binormal_location);
  }
  if (plan.has_eyevec()) {
    l_eyevec = b.define_input(vec3_type, "l_eyevec", plan._eyevec_location);
  }
  if (plan.has_color()) {
    l_color = b.define_input(vec4_type, "l_color", plan._color_location);
  }
  pvector<Pointer> l_lightcoord(key._lights.size());
  for (size_t i = 0; i < key._lights.size(); ++i) {
    if (plan._lightcoord_locations[i] >= 0) {
      l_lightcoord[i] = b.define_input(vec4_type,
        "l_lightcoord" + format_string(i), plan._lightcoord_locations[i]);
    }
  }

  // Uniforms.
  Pointer attr_fog, attr_fogcolor, sys_windowsize, trans_view_to_world;
  if (have_fog) {
    attr_fog = b.define_uniform(vec4_type, "attr_fog");
    attr_fogcolor = b.define_uniform(vec4_type, "attr_fogcolor");
  }
  if (plan.calc_point_coord()) {
    sys_windowsize = b.define_uniform(vec2_type, "sys_windowsize");
  }
  if (key.needs_view_to_world()) {
    trans_view_to_world = b.define_uniform(mat3x3_type, "trans_view_to_world");
  }

  pvector<Pointer> tex_vars(num_textures);
  pvector<Pointer> texscale_vars(num_textures);
  pvector<Pointer> texmat_vars(num_textures);
  pvector<Pointer> texcolor_vars(num_textures);
  pvector<Pointer> texconst_vars(num_textures);
  for (size_t i = 0; i < num_textures; ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    const ShaderType *sampler_type = ShaderType::register_type(
      ShaderType::SampledImage(tex._type, ShaderType::ST_float, false));

    tex_vars[i] = b.define_uniform(sampler_type, "tex_" + format_string(i));

    if (tex._flags & ShaderKey::TF_has_texscale) {
      texscale_vars[i] = b.define_uniform(vec3_type, "texscale_" + format_string(i));
    } else if (tex._flags & ShaderKey::TF_has_texmat) {
      texmat_vars[i] = b.define_uniform(mat4x4_type, "texmat_" + format_string(i));
    }
    if (tex._flags & ShaderKey::TF_uses_color) {
      texcolor_vars[i] = b.define_uniform(vec4_type, "texcolor_" + format_string(i));
    }
    if (tex._gen_mode == TexGenAttrib::M_constant) {
      texconst_vars[i] = b.define_uniform(vec4_type, "texconst_" + format_string(i));
    }
  }

  pvector<Pointer> attr_light(key._lights.size());
  pvector<Pointer> attr_lspec(key._lights.size());
  pvector<Pointer> shadow_vars(key._lights.size());
  for (size_t i = 0; i < key._lights.size(); ++i) {
    int light_flags = key._lights[i];
    attr_light[i] = b.define_uniform(mat4x4_type, "attr_light" + format_string(i));

    if (light_flags & ShaderKey::LF_has_shadows) {
      ShaderKey::LightFlags light_type =
        (ShaderKey::LightFlags)(light_flags & ShaderKey::LF_TYPE_MASK);
      bool is_cube = (light_type == ShaderKey::LF_type_point ||
                      light_type == ShaderKey::LF_type_sphere);

      const ShaderType *sampler_type = ShaderType::register_type(
        ShaderType::SampledImage(is_cube ? Texture::TT_cube_map : Texture::TT_2d_texture, ShaderType::ST_float, true));

      shadow_vars[i] = b.define_uniform(sampler_type, "shadow_" + format_string(i));
    }
    if (light_flags & ShaderKey::LF_has_specular_color) {
      attr_lspec[i] = b.define_uniform(vec4_type, "attr_lspec" + format_string(i));
    }
  }

  Pointer attr_material;
  if (key._flags & (Material::F_ambient | Material::F_diffuse |
                    Material::F_emission | Material::F_specular)) {
    attr_material = b.define_uniform(mat4x4_type, "attr_material");
  }
  Pointer attr_color_var;
  if (key.needs_color() && (key._flags & ShaderKey::F_flat_color) != 0) {
    attr_color_var = b.define_uniform(vec4_type, "attr_color");
  }
  pvector<Pointer> clipplane_vars;
  for (int i = 0; i < key._num_clip_planes; ++i) {
    clipplane_vars.push_back(b.define_uniform(vec4_type, "clipplane_" + format_string(i)));
  }
  Pointer attr_ambient;
  if (have_lighting) {
    attr_ambient = b.define_uniform(vec4_type, "attr_ambient");
  }
  Pointer attr_colorscale_var = b.define_uniform(vec4_type, "attr_colorscale");

  // Outputs.
  Pointer o_color, o_aux;
  if (have_color_write) {
    o_color = b.define_output(vec4_type, "o_color", 0);
  }
  if (have_aux) {
    o_aux = b.define_output(vec4_type, "o_aux", 1);
  }

  // Now generate the function body.
  Value world_position;
  if (l_world_position) {
    world_position = l_world_position.load();
  }
  Value eye_position;
  if (l_eye_position) {
    eye_position = l_eye_position.load();
  }

  // Clipping first!
  for (int i = 0; i < key._num_clip_planes; ++i) {
    auto plane = clipplane_vars[i].load();
    auto dist = dot(world_position.get_xyz(), plane.get_xyz()) + plane[3];
    b.kill_if(less_than_equal(dist, 0.0f));
  }

  Value eye_normal;
  if (l_eye_normal) {
    // Correct the surface normal for interpolation effects.
    eye_normal = normalize(l_eye_normal.load());
  }
  Value norm_eye_position;
  if (need_eye_reflection ||
      (plan.has_eye_position() && have_specular &&
       (key._flags & Material::F_local) != 0 && !key._lights.empty())) {
    norm_eye_position = normalize(eye_position.get_xyz());
  }
  Value eye_reflection;
  if (need_eye_reflection) {
    auto ndot = dot(eye_normal, norm_eye_position);
    eye_reflection = norm_eye_position - eye_normal * (ndot * 2.0f);
  }

  Value aux_rgb, aux_a;
  if (have_aux) {
    aux_rgb = b.const_vec3(0.0f, 0.0f, 0.0f);
    aux_a = b.const_float(0.0f);
  }

  auto find_texcoord_input = [&](const InternalName *name) -> Pointer {
    for (size_t ti = 0; ti < plan._texcoord_locations.size(); ++ti) {
      if (plan._texcoord_locations[ti].first == name) {
        return texcoord_inputs[ti];
      }
    }
    return Pointer();
  };

  // Now generate any texture coordinates according to TexGenAttrib.  If it
  // has a TexMatrixAttrib, also transform them.
  pvector<Value> texcoords(num_textures);
  for (size_t i = 0; i < num_textures; ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    Value texcoord;
    switch (tex._gen_mode) {
    case TexGenAttrib::M_off:
      texcoord = find_texcoord_input(tex._texcoord_name).load();
      break;
    case TexGenAttrib::M_eye_sphere_map:
      {
        auto offset_refl = eye_reflection + b.const_vec3(0.0f, -1.0f, 0.0f);
        auto scale = 1.0f / (length(offset_refl) * 2.0f);
        auto xz = eye_reflection.swizzle({0, 2}) * scale + 0.5f;
        texcoord = vec4(xz, 0.0f, 1.0f);
      }
      break;
    case TexGenAttrib::M_world_cube_map:
      texcoord = vec4(eye_reflection * trans_view_to_world.load(), 1.0f);
      break;
    case TexGenAttrib::M_eye_cube_map:
      texcoord = vec4(eye_reflection, 1.0f);
      break;
    case TexGenAttrib::M_world_normal:
      texcoord = l_world_normal.load();
      break;
    case TexGenAttrib::M_eye_normal:
      texcoord = vec4(eye_normal, 1.0f);
      break;
    case TexGenAttrib::M_world_position:
      texcoord = world_position;
      break;
    case TexGenAttrib::M_eye_position:
      texcoord = vec4(eye_position.get_xyz(), 1.0f);
      break;
    case TexGenAttrib::M_point_sprite:
      if (!plan.calc_point_coord()) {
        texcoord = vec4(pointcoord_builtin.load(), 0.0f, 1.0f);
      } else {
        auto pointcoord = l_pointcoord.load();
        auto fragcoord = fragcoord_builtin.load().swizzle({0, 1});
        auto xy = fragcoord - pointcoord.swizzle({0, 1}) * sys_windowsize.load();
        xy = xy * pointcoord[2];
        xy = xy * b.const_vec2(1.0f, -1.0f);
        xy = xy - 0.5f;
        texcoord = vec4(xy, 0.0f, 1.0f);
      }
      break;
    case TexGenAttrib::M_constant:
      texcoord = texconst_vars[i].load();
      break;
    default:
      texcoord = b.const_vec4(0.0f, 0.0f, 0.0f, 0.0f);
      pgraphnodes_cat.error()
        << "Unsupported TexGenAttrib mode: " << tex._gen_mode << "\n";
    }

    if (tex._flags & ShaderKey::TF_has_texscale) {
      texcoord.set_xyz(texcoord.get_xyz() * texscale_vars[i].load());
    } else if (tex._flags & ShaderKey::TF_has_texmat) {
      texcoord = texcoord * texmat_vars[i].load();
      texcoord.set_xyz(texcoord.get_xyz() / vec3(texcoord[3]));
    }
    texcoords[i] = texcoord;
  }

  // Samples the given texture stage with the given texture coordinates.
  auto sample_texture = [&](const ShaderKey::TextureInfo &tex, Pointer var,
                            Value texcoord, bool allow_proj) -> Value {
    auto image = var.load();
    switch (tex._type) {
    case Texture::TT_cube_map_array:
      return texture(image, texcoord);
    case Texture::TT_cube_map:
    case Texture::TT_3d_texture:
    case Texture::TT_2d_texture_array:
      return texture(image, texcoord.get_xyz());
    case Texture::TT_2d_texture:
      if (allow_proj) {
        // Regular sampling is done projectively, dividing by w.
        return texture_proj(image, texcoord.swizzle({0, 1, 3}));
      }
      return texture(image, texcoord.swizzle({0, 1}));
    case Texture::TT_1d_texture_array:
      return texture(image, texcoord.swizzle({0, 1}));
    case Texture::TT_1d_texture:
      return texture(image, texcoord[0]);
    case Texture::TT_buffer_texture:
      return texel_fetch(image, texcoord[0].convert(ShaderType::ST_int));
    default:
      pgraphnodes_cat.error() << "Unsupported texture type!\n";
      return texture(image, texcoord.swizzle({0, 1}));
    }
  };

  // Fetch the height maps first and apply parallax mapping.
  pvector<Value> tex_vals(num_textures);
  Value parallax_offset;
  if (need_heightmap) {
    auto eyevec = l_eyevec.load();

    for (size_t i = 0; i < num_textures; ++i) {
      const ShaderKey::TextureInfo &tex = key._textures[i];
      if ((tex._flags & ShaderKey::TF_map_height) == 0) {
        continue;
      }
      auto sample = sample_texture(tex, tex_vars[i], texcoords[i], false);
      tex_vals[i] = sample;

      Value height;
      if (tex._mode == TextureStage::M_normal_height ||
          (tex._flags & ShaderKey::TF_has_alpha) != 0) {
        height = vec3(sample[3]);
      } else {
        height = sample.get_xyz();
      }
      // parallax_offset = l_eyevec.xyz * (height * 2 - 1) * scale
      auto expanded = height * 2.0f - 1.0f;
      parallax_offset = eyevec * expanded * (double)parallax_mapping_scale;
      // Additional samples
      for (int j = 0; j < (int)parallax_mapping_samples - 1; ++j) {
        parallax_offset = eyevec * (parallax_offset + expanded) *
                          (0.5 * parallax_mapping_scale);
      }
    }
  }

  // Fetch all the other textures.
  for (size_t i = 0; i < num_textures; ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    if (tex._flags & ShaderKey::TF_map_height) {
      // Already sampled above.
      continue;
    }
    auto texcoord = texcoords[i];
    if (need_heightmap) {
      // Parallax mapping pushes the texture coordinates of the other
      // textures away from the camera.
      texcoord.set_xyz(texcoord.get_xyz() - parallax_offset);
    }
    tex_vals[i] = sample_texture(tex, tex_vars[i], texcoord, true);
  }

  if (plan.has_tangents()) {
    // Translate tangent-space normal in map to view-space.  Use Reoriented
    // Normal Mapping to blend additional normal maps.
    Value tsnormal;
    bool is_first = true;
    for (size_t i = 0; i < num_textures; ++i) {
      const ShaderKey::TextureInfo &tex = key._textures[i];
      if ((tex._flags & ShaderKey::TF_map_normal) == 0) {
        continue;
      }
      auto expanded = tex_vals[i].get_xyz() * 2.0f - 1.0f;
      if (is_first) {
        if (tex._flags & ShaderKey::TF_has_texscale) {
          tsnormal = normalize(expanded * texscale_vars[i].load());
        } else if (tex._flags & ShaderKey::TF_has_texmat) {
          tsnormal = normalize((vec4(expanded, 0.0f) * texmat_vars[i].load()).get_xyz());
        } else {
          tsnormal = normalize(expanded);
        }
        is_first = false;
        continue;
      }
      tsnormal.set_z(tsnormal[2] + 1.0f);
      auto tmp = tex_vals[i].get_xyz() * b.const_vec3(-2.0f, -2.0f, 2.0f) +
                 b.const_vec3(1.0f, 1.0f, -1.0f);
      if (tex._flags & ShaderKey::TF_has_texscale) {
        tmp = tmp * texscale_vars[i].load();
      } else if (tex._flags & ShaderKey::TF_has_texmat) {
        tmp = (vec4(tmp, 0.0f) * texmat_vars[i].load()).get_xyz();
      }
      tsnormal = normalize(tsnormal * dot(tsnormal, tmp) - tmp * tsnormal[2]);
    }
    eye_normal = eye_normal * tsnormal[2];
    eye_normal += normalize(l_tangent.load().get_xyz()) * tsnormal[0];
    eye_normal += normalize(l_binormal.load().get_xyz()) * tsnormal[1];
    eye_normal = normalize(eye_normal);
  }
  if (key._flags & ShaderKey::F_out_aux_normal) {
    // Output the camera-space surface normal.
    aux_rgb = eye_normal * 0.5f + 0.5f;
  }

  Value material;
  if (attr_material) {
    material = attr_material.load();
  }
  Value color_val;
  if (l_color) {
    color_val = l_color.load();
  } else if (attr_color_var) {
    color_val = attr_color_var.load();
  }

  // Begin view-space light calculations.
  Value tot_ambient, tot_diffuse, tot_specular, shininess;
  if (have_lighting) {
    auto ambient = attr_ambient.load();
    if (have_separate_ambient) {
      tot_ambient = ambient;
      tot_diffuse = b.const_vec4(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
      tot_diffuse = ambient;
    }
    if (have_specular) {
      tot_specular = b.const_vec4(0.0f, 0.0f, 0.0f, 0.0f);
      if (key._flags & Material::F_specular) {
        shininess = material[3][3];
      } else {
        // No shininess specified, use default.
        shininess = b.const_float(50.0f);
      }
    }
  }

  for (size_t i = 0; i < key._lights.size(); ++i) {
    int light_flags = key._lights[i];
    ShaderKey::LightFlags light_type =
      (ShaderKey::LightFlags)(light_flags & ShaderKey::LF_TYPE_MASK);
    bool has_shadows = (light_flags & ShaderKey::LF_has_shadows) != 0;

    auto light = attr_light[i].load();
    Value lightcoord;
    if (has_shadows) {
      lightcoord = l_lightcoord[i].load();
    }

    auto lcolor = light[0];
    Value lspec;
    if (have_specular) {
      if (light_flags & ShaderKey::LF_has_specular_color) {
        lspec = attr_lspec[i].load();
      } else {
        lspec = lcolor;
      }
    }

    // Samples the shadow map of a directional or spot light.
    auto sample_shadow_2d = [&]() -> Value {
      if (use_shadow_filter) {
        return texture_proj_dref(shadow_vars[i].load(),
                                 lightcoord.swizzle({0, 1, 3}),
                                 lightcoord[2]);
      } else {
        auto sample = texture_proj(shadow_vars[i].load(),
                                   lightcoord.swizzle({0, 1, 3}));
        return greater_than(sample[0], lightcoord[2] / lightcoord[3])
          .convert(ShaderType::ST_float);
      }
    };

    if (light_type == ShaderKey::LF_type_directional) {
      auto lvec = light[3].get_xyz();
      lcolor = lcolor * clamp(dot(eye_normal, lvec), 0.0f, 1.0f);
      if (has_shadows) {
        auto lshad = sample_shadow_2d();
        lcolor *= lshad;
        if (have_specular) {
          lspec *= lshad;
        }
      }
      tot_diffuse += lcolor;
      if (have_specular) {
        Value lhalf;
        if (key._flags & Material::F_local) {
          lhalf = normalize(lvec - norm_eye_position);
        } else {
          lhalf = normalize(lvec - b.const_vec3(0.0f, 1.0f, 0.0f));
        }
        lspec *= pow(clamp(dot(eye_normal, lhalf), 0.0f, 1.0f), shininess);
        tot_specular += lspec;
      }
    }
    else if (light_type == ShaderKey::LF_type_point ||
             light_type == ShaderKey::LF_type_sphere) {
      auto latten = light[1];
      auto lpoint = light[3];
      auto lvec = lpoint.get_xyz() - eye_position.get_xyz();
      auto ldist = length(lvec);
      lvec = lvec / ldist;
      if (light_type == ShaderKey::LF_type_sphere) {
        ldist = max(ldist, light[2][3]);
      }
      auto lattenv = 1.0f / (latten[0] + latten[1] * ldist +
                             latten[2] * (ldist * ldist));
      lcolor = lcolor * (lattenv * clamp(dot(eye_normal, lvec), 0.0f, 1.0f));
      if (has_shadows) {
        // Reconstruct the depth value from the cube map direction vector.
        auto dist = max(abs(lightcoord[0]),
                    max(abs(lightcoord[1]),
                        abs(lightcoord[2])));
        auto far_plane = latten[3];
        auto near_plane = lpoint[3];
        auto range = far_plane - near_plane;
        dist = (far_plane + near_plane) / range +
               (far_plane * near_plane * -2.0f) / (dist * range);
        auto dref = dist * 0.5f + 0.5f;
        Value lshad;
        if (use_shadow_filter) {
          lshad = texture_dref(shadow_vars[i].load(), lightcoord.get_xyz(), dref);
        } else {
          auto sample = texture(shadow_vars[i].load(), lightcoord.get_xyz());
          lshad = greater_than_equal(sample[0], dref).convert(ShaderType::ST_float);
        }
        lcolor *= lshad;
        if (have_specular) {
          lspec *= lshad;
        }
      }
      tot_diffuse += lcolor;
      if (have_specular) {
        Value lhalf;
        if (key._flags & Material::F_local) {
          lhalf = normalize(lvec - norm_eye_position);
        } else {
          lhalf = normalize(lvec - b.const_vec3(0.0f, 1.0f, 0.0f));
        }
        lspec *= lattenv;
        lspec *= pow(clamp(dot(eye_normal, lhalf), 0.0f, 1.0f), shininess);
        tot_specular += lspec;
      }
    }
    else if (light_type == ShaderKey::LF_type_spot) {
      auto latten = light[1];
      auto ldir = light[2];
      auto lpoint = light[3];
      auto lvec = lpoint.get_xyz() - eye_position.get_xyz();
      auto ldist = length(lvec);
      lvec = lvec / ldist;
      auto langle = clamp(dot(ldir.get_xyz(), lvec), 0.0f, 1.0f);
      auto lattenv = 1.0f / (latten[0] + latten[1] * ldist +
                             latten[2] * (ldist * ldist));
      lattenv *= pow(langle, latten[3]);
      lattenv = mix(lattenv, 0.0f, less_than(langle, ldir[3]));
      lcolor = lcolor * (lattenv * clamp(dot(eye_normal, lvec), 0.0f, 1.0f));
      if (has_shadows) {
        auto lshad = sample_shadow_2d();
        lcolor *= lshad;
        if (have_specular) {
          lspec *= lshad;
        }
      }
      tot_diffuse += lcolor;
      if (have_specular) {
        Value lhalf;
        if (key._flags & Material::F_local) {
          lhalf = normalize(lvec - norm_eye_position);
        } else {
          lhalf = normalize(lvec - b.const_vec3(0.0f, 1.0f, 0.0f));
        }
        lspec *= lattenv;
        lspec *= pow(clamp(dot(eye_normal, lhalf), 0.0f, 1.0f), shininess);
        tot_specular += lspec;
      }
    }
  }

  bool have_color_val = l_color || attr_color_var;

  Value result;
  if (have_lighting) {
    switch (key._light_ramp_mode) {
    case LightRampAttrib::LRT_single_threshold:
      {
        // Single-threshold light ramp.
        auto lr_in = dot(tot_diffuse.get_xyz(), b.const_vec3(0.33f, 0.34f, 0.33f));
        auto lr_scale = mix(
          (double)key._light_ramp_level[0] / lr_in, 0.0f,
          less_than(lr_in, (double)key._light_ramp_threshold[0]));
        tot_diffuse = tot_diffuse * lr_scale;
        break;
      }
    case LightRampAttrib::LRT_double_threshold:
      {
        // Double-threshold light ramp.
        auto lr_in = dot(tot_diffuse.get_xyz(), b.const_vec3(0.33f, 0.34f, 0.33f));
        auto lr_out = mix(
          b.const_float(0.0f), (double)key._light_ramp_level[0],
          greater_than(lr_in, (double)key._light_ramp_threshold[0]));
        lr_out = mix(
          lr_out, (double)key._light_ramp_level[1],
          greater_than(lr_in, (double)key._light_ramp_threshold[1]));
        tot_diffuse = tot_diffuse * (lr_out / lr_in);
        break;
      }
    default:
      break;
    }

    // Begin view-space light summation.
    if (key._flags & Material::F_emission) {
      result = material[2];
    } else if (key._texture_flags & (ShaderKey::TF_map_glow | ShaderKey::TF_map_emission)) {
      result = b.const_vec4(1.0f, 1.0f, 1.0f, 0.0f);
    } else {
      result = b.const_vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    if (key._texture_flags & ShaderKey::TF_map_emission) {
      result.set_xyz(result.get_xyz() * tex_vals[map_index_emission].get_xyz());
    }
    if (key._texture_flags & ShaderKey::TF_map_glow) {
      result = result * clamp((tex_vals[map_index_glow][3] - 0.5f) * 2.0f,
                              0.0f, 1.0f);
    }
    if (have_separate_ambient) {
      for (size_t i = 0; i < num_textures; ++i) {
        if (key._textures[i]._flags & ShaderKey::TF_map_occlusion) {
          tot_ambient = tot_ambient * tex_vals[i][0];
        }
      }
      if (key._flags & Material::F_ambient) {
        result += tot_ambient * material[0];
      } else if (have_color_val) {
        result += tot_ambient * color_val;
      } else {
        result += tot_ambient;
      }
    }
    if (key._flags & Material::F_diffuse) {
      result += tot_diffuse * material[1];
    } else if (have_color_val) {
      result += tot_diffuse * color_val;
    } else {
      result += tot_diffuse;
    }
    if (key._light_ramp_mode == LightRampAttrib::LRT_default) {
      result = clamp(result, 0.0f, 1.0f);
    }
    // End view-space light calculations.

    // Combine in alpha, which bypasses lighting calculations.
    if (key._flags & ShaderKey::F_calc_primary_alpha) {
      Value alpha;
      if (key._flags & Material::F_diffuse) {
        alpha = material[1][3];
      } else if (have_color_val) {
        alpha = color_val[3];
      } else {
        alpha = b.const_float(1.0f);
      }
      result.set_w(alpha);
    }
  } else {
    if (have_color_val) {
      result = color_val;
    } else {
      result = b.const_vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
  }

  // Apply the color scale.
  auto colorscale = attr_colorscale_var.load();
  result *= colorscale;

  // Store these if any stages will use it.
  Value primary_color, last_saved_result;
  if (key._texture_flags & ShaderKey::TF_uses_primary_color) {
    primary_color = result;
  }
  if (key._texture_flags & ShaderKey::TF_uses_last_saved_result) {
    last_saved_result = result;
  }

  pvector<Id> texcolor_vals(num_textures, Id());
  auto get_texcolor = [&](size_t i) -> Value {
    if (!texcolor_vals[i]) {
      texcolor_vals[i] = texcolor_vars[i].load();
    }
    return Value(b, texcolor_vals[i]);
  };

  // Emits one source of a combine mode, as a vec3 (for the rgb combiner) or
  // a scalar (for the alpha combiner).
  auto combine_source = [&](const ShaderKey::TextureInfo &tex, int n,
                            bool alpha, size_t i) -> Value {
    TextureStage::CombineSource c_src;
    TextureStage::CombineOperand c_op;
    if (!alpha) {
      c_src = UNPACK_COMBINE_SRC(tex._combine_rgb, n);
      c_op = UNPACK_COMBINE_OP(tex._combine_rgb, n);
    } else {
      c_src = UNPACK_COMBINE_SRC(tex._combine_alpha, n);
      c_op = UNPACK_COMBINE_OP(tex._combine_alpha, n);
    }
    Value src;
    switch (c_src) {
    case TextureStage::CS_undefined:
    case TextureStage::CS_texture:
      src = tex_vals[i];
      break;
    case TextureStage::CS_constant:
      src = get_texcolor(i);
      break;
    case TextureStage::CS_primary_color:
      src = primary_color;
      break;
    case TextureStage::CS_previous:
      src = result;
      break;
    case TextureStage::CS_constant_color_scale:
      src = colorscale * get_texcolor(i);
      break;
    case TextureStage::CS_last_saved_result:
      src = last_saved_result;
      break;
    }
    bool one_minus = (c_op == TextureStage::CO_one_minus_src_color ||
                      c_op == TextureStage::CO_one_minus_src_alpha);
    bool use_color = (c_op == TextureStage::CO_src_color ||
                      c_op == TextureStage::CO_one_minus_src_color);
    if (alpha || !use_color) {
      auto value = src[3];
      if (one_minus) {
        value = clamp(1.0f - value, 0.0f, 1.0f);
      }
      if (alpha) {
        return value;
      }
      // Dunno if it's legal in the FFP at all, but let's just allow it.
      return vec3(value);
    } else {
      auto value = src.get_xyz();
      if (one_minus) {
        value = clamp(1.0f - value, 0.0f, 1.0f);
      }
      return value;
    }
  };

  // Emits the scalar dot3 combine calculation, using the sources of the rgb
  // or alpha combiner as indicated.  Scalar (alpha) sources are broadcast,
  // like Cg promoted them in the original generator.
  auto combine_dot3 = [&](const ShaderKey::TextureInfo &tex, bool alpha, size_t i) -> Value {
    auto src0 = combine_source(tex, 0, alpha, i);
    auto src1 = combine_source(tex, 1, alpha, i);
    if (alpha) {
      src0 = vec3(src0);
      src1 = vec3(src1);
    }
    return dot(src0 - 0.5f, src1 - 0.5f) * 4.0f;
  };

  // Emits a combine mode, as a vec3 (rgb combiner) or scalar (alpha).
  auto combine_mode = [&](const ShaderKey::TextureInfo &tex,
                          TextureStage::CombineMode c_mode,
                          bool alpha, size_t i) -> Value {
    switch (c_mode) {
    case TextureStage::CM_modulate:
      {
        auto src0 = combine_source(tex, 0, alpha, i);
        auto src1 = combine_source(tex, 1, alpha, i);
        return src0 * src1;
      }
    case TextureStage::CM_add:
      {
        auto src0 = combine_source(tex, 0, alpha, i);
        auto src1 = combine_source(tex, 1, alpha, i);
        return src0 + src1;
      }
    case TextureStage::CM_add_signed:
      {
        auto src0 = combine_source(tex, 0, alpha, i);
        auto src1 = combine_source(tex, 1, alpha, i);
        return src0 + src1 - 0.5f;
      }
    case TextureStage::CM_interpolate:
      {
        auto src0 = combine_source(tex, 0, alpha, i);
        auto src1 = combine_source(tex, 1, alpha, i);
        auto src2 = combine_source(tex, 2, alpha, i);
        return mix(src1, src0, src2);
      }
    case TextureStage::CM_subtract:
      {
        auto src0 = combine_source(tex, 0, alpha, i);
        auto src1 = combine_source(tex, 1, alpha, i);
        return src0 - src1;
      }
    case TextureStage::CM_dot3_rgb:
    case TextureStage::CM_dot3_rgba:
      {
        auto value = combine_dot3(tex, alpha, i);
        return alpha ? value : vec3(value);
      }
    case TextureStage::CM_replace:
    default: // Not sure if this is correct as default value.
      return combine_source(tex, 0, alpha, i);
    }
  };

  // Now loop through the textures to compose our magic blending formulas.
  for (size_t i = 0; i < num_textures; ++i) {
    const ShaderKey::TextureInfo &tex = key._textures[i];
    if (tex._mode == TextureStage::M_modulate && tex._flags == 0) {
      // Skip this stage.
      continue;
    }
    switch (tex._mode) {
    case TextureStage::M_modulate:
      if ((tex._flags & ShaderKey::TF_has_rgb) != 0 &&
          (tex._flags & ShaderKey::TF_has_alpha) != 0) {
        result *= tex_vals[i];
      } else if (tex._flags & ShaderKey::TF_has_alpha) {
        result.set_w(result[3] * tex_vals[i][3]);
      } else if (tex._flags & ShaderKey::TF_has_rgb) {
        result.set_xyz(result.get_xyz() * tex_vals[i].get_xyz());
      }
      break;
    case TextureStage::M_modulate_glow:
    case TextureStage::M_modulate_gloss:
      // in the case of glow or spec we currently see the specularity evenly
      // across the surface even if transparency or masking is present not
      // sure if this is desired behavior or not.  *MOST* would construct a
      // spec map based off of what isisn't seen based on the masktransparency
      // this may have to be left alone for now agartner
      result.set_xyz(result.get_xyz() * tex_vals[i].get_xyz());
      break;
    case TextureStage::M_decal:
      result.set_xyz(mix(result.get_xyz(), tex_vals[i].get_xyz(), tex_vals[i][3]));
      break;
    case TextureStage::M_blend:
      result.set_xyz(mix(result.get_xyz(), get_texcolor(i).get_xyz(), tex_vals[i].get_xyz()));
      if (key._flags & ShaderKey::F_calc_primary_alpha) {
        result.set_w(result[3] * tex_vals[i][3]);
      }
      break;
    case TextureStage::M_replace:
      result = tex_vals[i];
      break;
    case TextureStage::M_add:
      result.set_xyz(result.get_xyz() + tex_vals[i].get_xyz());
      if (key._flags & ShaderKey::F_calc_primary_alpha) {
        result.set_w(result[3] * tex_vals[i][3]);
      }
      break;
    case TextureStage::M_combine:
      {
        TextureStage::CombineMode combine_rgb = (TextureStage::CombineMode)
          ((tex._flags & ShaderKey::TF_COMBINE_RGB_MODE_MASK) >> ShaderKey::TF_COMBINE_RGB_MODE_SHIFT);
        TextureStage::CombineMode combine_alpha = (TextureStage::CombineMode)
          ((tex._flags & ShaderKey::TF_COMBINE_ALPHA_MODE_MASK) >> ShaderKey::TF_COMBINE_ALPHA_MODE_SHIFT);
        int rgb_scale = 1, alpha_scale = 1;
        if (tex._flags & ShaderKey::TF_rgb_scale_2) {
          rgb_scale *= 2;
        }
        if (tex._flags & ShaderKey::TF_rgb_scale_4) {
          rgb_scale *= 4;
        }
        if (tex._flags & ShaderKey::TF_alpha_scale_2) {
          alpha_scale *= 2;
        }
        if (tex._flags & ShaderKey::TF_alpha_scale_4) {
          alpha_scale *= 4;
        }
        if (combine_rgb == TextureStage::CM_dot3_rgba) {
          auto value = vec4(combine_dot3(tex, false, i));
          if (rgb_scale != 1 || alpha_scale != 1) {
            value *= b.const_vec4((float)rgb_scale, (float)rgb_scale,
                                  (float)rgb_scale, (float)alpha_scale);
          }
          result = clamp(value, 0.0f, 1.0f);
        } else {
          auto rgb = combine_mode(tex, combine_rgb, false, i);
          if (rgb_scale != 1) {
            rgb *= (double)rgb_scale;
          }
          rgb = clamp(rgb, 0.0f, 1.0f);
          auto alpha = combine_mode(tex, combine_alpha, true, i);
          if (alpha_scale != 1) {
            alpha *= (double)alpha_scale;
          }
          alpha = clamp(alpha, 0.0f, 1.0f);
          result.set_xyz(rgb);
          result.set_w(alpha);
        }
      }
      break;
    case TextureStage::M_blend_color_scale:
      result.set_xyz(mix(result.get_xyz(),
                         get_texcolor(i).get_xyz() * colorscale.get_xyz(),
                         tex_vals[i].get_xyz()));
      if (key._flags & ShaderKey::F_calc_primary_alpha) {
        result.set_w(result[3] * tex_vals[i][3]);
      }
      break;
    default:
      break;
    }
    if (tex._flags & ShaderKey::TF_saved_result) {
      last_saved_result = result;
    }
  }

  // Shader subsumes the alpha test.
  if ((key._flags & ShaderKey::F_ALPHA_TEST_MASK) != 0) {
    // The comparison under which the fragment is *discarded*: the negation
    // of the mode under which it passes.
    double ref = key._alpha_test_ref;
    auto alpha = result[3];
    switch (key.get_alpha_test_mode()) {
    case RenderAttrib::M_never:
      b.kill_if(module.define_constant(ShaderType::BOOL, 1));
      break;
    case RenderAttrib::M_less:
      b.kill_if(greater_than_equal(alpha, ref));
      break;
    case RenderAttrib::M_equal:
      b.kill_if(not_equal(alpha, ref));
      break;
    case RenderAttrib::M_less_equal:
      b.kill_if(greater_than(alpha, ref));
      break;
    case RenderAttrib::M_greater:
      b.kill_if(less_than_equal(alpha, ref));
      break;
    case RenderAttrib::M_not_equal:
      b.kill_if(equal(alpha, ref));
      break;
    case RenderAttrib::M_greater_equal:
      b.kill_if(less_than(alpha, ref));
      break;
    case RenderAttrib::M_none:
    case RenderAttrib::M_always:
      break;
    }
  }

  if (key._flags & ShaderKey::F_out_alpha_glow) {
    if (key._texture_flags & ShaderKey::TF_map_glow) {
      result.set_w(tex_vals[map_index_glow][3]);
    } else {
      result.set_w(b.const_float(0.5f));
    }
  }
  if (key._flags & ShaderKey::F_out_aux_glow) {
    if (key._texture_flags & ShaderKey::TF_map_glow) {
      aux_a = tex_vals[map_index_glow][3];
    } else {
      aux_a = b.const_float(0.5f);
    }
  }

  if (have_specular) {
    if (key._flags & Material::F_specular) {
      tot_specular *= material[3];
    }
    if (key._texture_flags & ShaderKey::TF_map_gloss) {
      tot_specular *= tex_vals[map_index_gloss][3];
    }
    result.set_xyz(result.get_xyz() + tot_specular.get_xyz());
  }

  // Apply the HDR light ramps.
  switch (key._light_ramp_mode) {
  case LightRampAttrib::LRT_hdr0:
    {
      auto rgb = result.get_xyz();
      auto rgb2 = rgb * rgb;
      auto rgb3 = rgb2 * rgb;
      auto numerator = rgb3 + rgb2 + rgb;
      result.set_xyz(numerator / (numerator + 1.0f));
      break;
    }
  case LightRampAttrib::LRT_hdr1:
    {
      auto rgb = result.get_xyz();
      auto numerator = rgb * rgb + rgb;
      result.set_xyz(numerator / (numerator + 1.0f));
      break;
    }
  case LightRampAttrib::LRT_hdr2:
    {
      auto rgb = result.get_xyz();
      result.set_xyz(rgb / (rgb + 1.0f));
      break;
    }
  default:
    break;
  }

  // Apply fog.
  if (have_fog) {
    Fog::Mode fog_mode = (Fog::Mode)
      (((key._flags & ShaderKey::F_FOG_MODE_MASK) >> ShaderKey::F_FOG_MODE_SHIFT) - 1);
    auto fog_params = attr_fog.load();
    auto fog_color = attr_fogcolor.load().get_xyz();
    auto hpos_z = l_hpos.load()[2];

    Value factor;
    switch (fog_mode) {
    case Fog::M_linear:
      factor = clamp((fog_params[2] - hpos_z) * fog_params[3], 0.0f, 1.0f);
      break;
    case Fog::M_exponential: // 1.442695f = 1 / log(2)
      factor = clamp(exp2(fog_params[0] * hpos_z * -1.442695f), 0.0f, 1.0f);
      break;
    case Fog::M_exponential_squared:
      {
        auto density = fog_params[0];
        auto value = (density * density) * (hpos_z * hpos_z);
        factor = clamp(exp2(value * -1.442695f), 0.0f, 1.0f);
        break;
      }
    default:
      factor = b.const_float(1.0f);
      break;
    }
    result.set_xyz(mix(fog_color, result.get_xyz(), factor));
  }

  if (o_color) {
    o_color.store(result);
  }
  if (o_aux) {
    o_aux.store(vec4(aux_rgb, aux_a));
  }

  b.op_return();

  ShaderModuleSpirV *shader_module = new ShaderModuleSpirV(Shader::Stage::FRAGMENT, std::move(module));
  if (pgraphnodes_cat.is_spam()) {
    std::ostream &out = pgraphnodes_cat.spam()
      << "Generated fragment shader:\n";
    shader_module->_instructions.disassemble(out);
  }
  return shader_module;
}

/**
 * Initializes the ShaderKey to the empty state.
 */
ShaderGenerator::ShaderKey::
ShaderKey() :
  _flags(0),
  _texture_flags(0),
  _alpha_test_ref(0.0),
  _num_clip_planes(0),
  _light_ramp_mode(LightRampAttrib::LRT_default),
  _light_ramp_level{0, 0},
  _light_ramp_threshold{0, 0},
  _num_anim_transforms(0) {
}

/**
 * Returns true if this ShaderKey sorts less than the other one.  This is an
 * arbitrary, but consistent ordering.
 */
bool ShaderGenerator::ShaderKey::
operator < (const ShaderKey &other) const {
  if (_flags != other._flags) {
    return _flags < other._flags;
  }
  if (_texture_flags != other._texture_flags) {
    return _texture_flags < other._texture_flags;
  }
  if (_textures.size() != other._textures.size()) {
    return _textures.size() < other._textures.size();
  }
  if (_lights.size() != other._lights.size()) {
    return _lights.size() < other._lights.size();
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
  for (size_t i = 0; i < _lights.size(); ++i) {
    int light_flags = _lights[i];
    int other_light_flags = other._lights[i];
    if (light_flags != other_light_flags) {
      return light_flags < other_light_flags;
    }
  }
  if (_alpha_test_ref != other._alpha_test_ref) {
    return _alpha_test_ref < other._alpha_test_ref;
  }
  if (_num_clip_planes != other._num_clip_planes) {
    return _num_clip_planes < other._num_clip_planes;
  }
  if (_light_ramp_mode != other._light_ramp_mode) {
    return _light_ramp_mode < other._light_ramp_mode;
  }
  if (_light_ramp_mode != LightRampAttrib::LRT_default &&
      _light_ramp_mode != LightRampAttrib::LRT_identity) {
    if (_light_ramp_level[0] != other._light_ramp_level[0]) {
      return _light_ramp_level[0] < other._light_ramp_level[0];
    }
    if (_light_ramp_level[1] != other._light_ramp_level[1]) {
      return _light_ramp_level[1] < other._light_ramp_level[1];
    }
    if (_light_ramp_threshold[0] != other._light_ramp_threshold[0]) {
      return _light_ramp_threshold[0] < other._light_ramp_threshold[0];
    }
    if (_light_ramp_threshold[1] != other._light_ramp_threshold[1]) {
      return _light_ramp_threshold[1] < other._light_ramp_threshold[1];
    }
  }
  return _num_anim_transforms < other._num_anim_transforms;
}

/**
 * Returns true if this ShaderKey is equal to the other one.
 */
bool ShaderGenerator::ShaderKey::
operator == (const ShaderKey &other) const {
  if (_flags != other._flags) {
    return false;
  }
  if (_texture_flags != other._texture_flags) {
    return false;
  }
  if (_textures.size() != other._textures.size()) {
    return false;
  }
  if (_lights.size() != other._lights.size()) {
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
  for (size_t i = 0; i < _lights.size(); ++i) {
    int light_flags = _lights[i];
    int other_light_flags = other._lights[i];
    if (light_flags != other_light_flags) {
      return false;
    }
  }
  return _alpha_test_ref == other._alpha_test_ref
      && _num_clip_planes == other._num_clip_planes
      && _light_ramp_mode == other._light_ramp_mode
      && _light_ramp_level[0] == other._light_ramp_level[0]
      && _light_ramp_level[1] == other._light_ramp_level[1]
      && _light_ramp_threshold[0] == other._light_ramp_threshold[0]
      && _light_ramp_threshold[1] == other._light_ramp_threshold[1]
      && _num_anim_transforms == other._num_anim_transforms;
}

/**
 * Deserializes this shader key.
 */
void ShaderGenerator::ShaderKey::
read_datagram(DatagramIterator &scan) {
  _flags = scan.get_uint32();

  _texture_flags = 0;

  _textures.resize(scan.get_uint16());
  for (ShaderKey::TextureInfo &tex : _textures) {
    tex._texcoord_name = scan.get_string();
    tex._type = (Texture::TextureType)scan.get_uint8();
    tex._mode = (TextureStage::Mode)scan.get_uint8();
    tex._gen_mode = (TexGenAttrib::Mode)scan.get_uint8();
    tex._flags = scan.get_uint32();
    if (tex._mode == TextureStage::M_combine) {
      tex._combine_rgb = scan.get_uint16();
      tex._combine_alpha = scan.get_uint16();
    } else {
      tex._combine_rgb = 0u;
      tex._combine_alpha = 0u;
    }
    _texture_flags |= tex._flags;
  }

  _lights.resize(scan.get_uint16());
  for (int &light_flags : _lights) {
    light_flags = (int)scan.get_uint8();
  }

  if ((_flags & ShaderKey::F_ALPHA_TEST_MASK) != 0) {
    _alpha_test_ref = scan.get_float32();
  } else {
    _alpha_test_ref = 0;
  }

  _num_clip_planes = scan.get_uint16();
  _light_ramp_mode = (LightRampAttrib::LightRampMode)scan.get_uint8();
  if (_light_ramp_mode != LightRampAttrib::LRT_default &&
      _light_ramp_mode != LightRampAttrib::LRT_identity) {
    _light_ramp_level[0] = scan.get_float32();
    _light_ramp_level[1] = scan.get_float32();
    _light_ramp_threshold[0] = scan.get_float32();
    _light_ramp_threshold[1] = scan.get_float32();
  }
  _num_anim_transforms = scan.get_int32();
}

/**
 * Serializes this shader key.
 */
void ShaderGenerator::ShaderKey::
write_datagram(Datagram &dg) const {
  dg.add_uint32(_flags);

  dg.add_uint16(_textures.size());
  for (const ShaderKey::TextureInfo &tex : _textures) {
    dg.add_string(tex._texcoord_name->get_name());
    dg.add_uint8(tex._type);
    dg.add_uint8(tex._mode);
    dg.add_uint8(tex._gen_mode);
    dg.add_uint32(tex._flags);
    if (tex._mode == TextureStage::M_combine) {
      dg.add_uint16(tex._combine_rgb);
      dg.add_uint16(tex._combine_alpha);
    }
  }

  dg.add_uint16(_lights.size());
  for (int light_flags : _lights) {
    dg.add_uint8(light_flags);
  }

  if ((_flags & ShaderKey::F_ALPHA_TEST_MASK) != 0) {
    dg.add_float32(_alpha_test_ref);
  }

  dg.add_uint16(_num_clip_planes);
  dg.add_uint8(_light_ramp_mode);
  if (_light_ramp_mode != LightRampAttrib::LRT_default &&
      _light_ramp_mode != LightRampAttrib::LRT_identity) {
    dg.add_float32(_light_ramp_level[0]);
    dg.add_float32(_light_ramp_level[1]);
    dg.add_float32(_light_ramp_threshold[0]);
    dg.add_float32(_light_ramp_threshold[1]);
  }
  dg.add_int32(_num_anim_transforms);
}
