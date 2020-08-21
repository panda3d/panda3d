/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeMaterials.cxx
 * @author rdb
 * @date 2008-10-03
 */

#include "daeMaterials.h"
#include "config_daeegg.h"
#include "fcollada_utils.h"

#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDMaterial.h>
#include <FCDocument/FCDEffect.h>
#include <FCDocument/FCDTexture.h>
#include <FCDocument/FCDEffectParameterSampler.h>
#include <FCDocument/FCDImage.h>

#include "filename.h"
#include "string_utils.h"

using std::endl;
using std::string;

TypeHandle DaeMaterials::_type_handle;

// luminance function, based on the ISOCIE color standards see ITU-R
// Recommendation BT.709-4
#define luminance(c) ((c[0] * 0.212671 + c[1] * 0.715160 + c[2] * 0.072169))

/**
 *
 */
DaeMaterials::
DaeMaterials(const FCDGeometryInstance* geometry_instance) {
  for (size_t mi = 0; mi < geometry_instance->GetMaterialInstanceCount(); ++mi) {
    add_material_instance(geometry_instance->GetMaterialInstance(mi));
  }
}

/**
 * Adds a material instance.  Normally automatically done by constructor.
 */
void DaeMaterials::add_material_instance(const FCDMaterialInstance* instance) {
  nassertv(instance != nullptr);
  const string semantic (FROM_FSTRING(instance->GetSemantic()));
  if (_materials.count(semantic) > 0) {
    daeegg_cat.warning() << "Ignoring duplicate material with semantic " << semantic << endl;
    return;
  }
  _materials[semantic] = new DaeMaterial();

  // Load in the uvsets
  for (size_t vib = 0; vib < instance->GetVertexInputBindingCount(); ++vib) {
    const FCDMaterialInstanceBindVertexInput* mivib = instance->GetVertexInputBinding(vib);
    assert(mivib != nullptr);
    PT(DaeVertexInputBinding) bvi = new DaeVertexInputBinding();
    bvi->_input_set = mivib->inputSet;
#if FCOLLADA_VERSION >= 0x00030005
    bvi->_input_semantic = mivib->GetInputSemantic();
    bvi->_semantic = *mivib->semantic;
#else
    bvi->_input_semantic = mivib->inputSemantic;
    bvi->_semantic = FROM_FSTRING(mivib->semantic);
#endif
    _materials[semantic]->_uvsets.push_back(bvi);
  }

  // Handle the material stuff
  if (daeegg_cat.is_spam()) {
    daeegg_cat.spam() << "Trying to process material with semantic " << semantic << endl;
  }
  PT_EggMaterial egg_material = new EggMaterial(semantic);
  pvector<PT_EggTexture> egg_textures;
  const FCDEffect* effect = instance->GetMaterial()->GetEffect();
  if (effect == nullptr) {
    if (daeegg_cat.is_debug()) {
      daeegg_cat.debug() << "Ignoring material (semantic: " << semantic << ") without assigned effect" << endl;
    }
  } else {
    // Grab the common profile effect
    const FCDEffectStandard* effect_common = (FCDEffectStandard *)effect->FindProfile(FUDaeProfileType::COMMON);
    if (effect_common == nullptr) {
      daeegg_cat.info() << "Ignoring effect referenced by material with semantic " << semantic
                         << " because it has no common profile" << endl;
    } else {
      if (daeegg_cat.is_spam()) {
        daeegg_cat.spam() << "Processing effect, material semantic is " << semantic << endl;
      }
      // Set the material parameters
      egg_material->set_amb(TO_COLOR(effect_common->GetAmbientColor()));
      // We already process transparency using blend modes LVecBase4 diffuse =
      // TO_COLOR(effect_common->GetDiffuseColor());
      // diffuse.set_w(diffuse.get_w() * (1.0f -
      // effect_common->GetOpacity())); egg_material->set_diff(diffuse);
      egg_material->set_diff(TO_COLOR(effect_common->GetDiffuseColor()));
      egg_material->set_emit(TO_COLOR(effect_common->GetEmissionColor()) * effect_common->GetEmissionFactor());
      egg_material->set_shininess(effect_common->GetShininess());
      egg_material->set_spec(TO_COLOR(effect_common->GetSpecularColor()));
      // Now try to load in the textures
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::DIFFUSE, EggTexture::ET_modulate);
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::BUMP, EggTexture::ET_normal);
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::SPECULAR, EggTexture::ET_modulate_gloss);
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::SPECULAR_LEVEL, EggTexture::ET_gloss);
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::TRANSPARENT, EggTexture::ET_unspecified, EggTexture::F_alpha);
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::EMISSION, EggTexture::ET_add);
#if FCOLLADA_VERSION < 0x00030005
      process_texture_bucket(semantic, effect_common, FUDaeTextureChannel::OPACITY, EggTexture::ET_unspecified, EggTexture::F_alpha);
#endif
      // Now, calculate the color blend stuff.
      _materials[semantic]->_blend = convert_blend(effect_common->GetTransparencyMode(),
                                          TO_COLOR(effect_common->GetTranslucencyColor()),
                                                   effect_common->GetTranslucencyFactor());
    }
    // Find an <extra> tag to support some extra stuff from extensions
    process_extra(semantic, effect->GetExtra());
  }
  if (daeegg_cat.is_spam()) {
    daeegg_cat.spam() << "Found " << egg_textures.size() << " textures in material" << endl;
  }
  _materials[semantic]->_egg_material = egg_material;
}

/**
 * Processes the given texture bucket and gives the textures in it the given
 * envtype and format.
 */
void DaeMaterials::
process_texture_bucket(const string semantic, const FCDEffectStandard* effect_common, FUDaeTextureChannel::Channel bucket, EggTexture::EnvType envtype, EggTexture::Format format) {
  for (size_t tx = 0; tx < effect_common->GetTextureCount(bucket); ++tx) {
    const FCDImage* image = effect_common->GetTexture(bucket, tx)->GetImage();
    if (image == nullptr) {
      daeegg_cat.warning() << "Texture references a nonexisting image!" << endl;
    } else {
      const FCDEffectParameterSampler* sampler = effect_common->GetTexture(bucket, tx)->GetSampler();
      // FCollada only supplies absolute paths.  We need to grab the document
      // location ourselves and make the image path absolute.
      Filename texpath;
      if (image->GetDocument()) {
        Filename docpath = Filename::from_os_specific(FROM_FSTRING(image->GetDocument()->GetFileUrl()));
        docpath.make_canonical();
        texpath = Filename::from_os_specific(FROM_FSTRING(image->GetFilename()));
        texpath.make_canonical();
        texpath.make_relative_to(docpath.get_dirname(), true);
        if (daeegg_cat.is_debug()) {
          daeegg_cat.debug() << "Found texture with path " << texpath << endl;
        }
      } else {
        // Never mind.
        texpath = Filename::from_os_specific(FROM_FSTRING(image->GetFilename()));
      }
      PT_EggTexture egg_texture = new EggTexture(FROM_FSTRING(image->GetDaeId()), texpath.to_os_generic());
      // Find a set of UV coordinates
      const FCDEffectParameterInt* uvset = effect_common->GetTexture(bucket, tx)->GetSet();
      if (uvset != nullptr) {
        if (daeegg_cat.is_debug()) {
          daeegg_cat.debug() << "Texture has uv name '" << FROM_FSTRING(uvset->GetSemantic()) << "'\n";
        }
        string uvset_semantic (FROM_FSTRING(uvset->GetSemantic()));

        // Only set the UV name if this UV set actually exists.
        for (size_t i = 0; i < _materials[semantic]->_uvsets.size(); ++i) {
          if (_materials[semantic]->_uvsets[i]->_semantic == uvset_semantic) {
            egg_texture->set_uv_name(uvset_semantic);
            break;
          }
        }
      }
      // Apply sampler stuff
      if (sampler != nullptr) {
        egg_texture->set_texture_type(convert_texture_type(sampler->GetSamplerType()));
        egg_texture->set_wrap_u(convert_wrap_mode(sampler->GetWrapS()));
        if (sampler->GetSamplerType() != FCDEffectParameterSampler::SAMPLER1D) {
          egg_texture->set_wrap_v(convert_wrap_mode(sampler->GetWrapT()));
        }
        if (sampler->GetSamplerType() == FCDEffectParameterSampler::SAMPLER3D) {
          egg_texture->set_wrap_w(convert_wrap_mode(sampler->GetWrapP()));
        }
        egg_texture->set_minfilter(convert_filter_type(sampler->GetMinFilter()));
        egg_texture->set_magfilter(convert_filter_type(sampler->GetMagFilter()));
        if (envtype != EggTexture::ET_unspecified) {
          egg_texture->set_env_type(envtype);
        }
        if (format != EggTexture::F_unspecified) {
          egg_texture->set_format(format);
        }
      }
      _materials[semantic]->_egg_textures.push_back(egg_texture);
    }
  }
}

/**
 * Processes the extra data in the given <extra> tag.  If the given element is
 * NULL, it just silently returns.
 */
void DaeMaterials::
process_extra(const string semantic, const FCDExtra* extra) {
  if (extra == nullptr) return;
  const FCDEType* etype = extra->GetDefaultType();
  if (etype == nullptr) return;
  for (size_t et = 0; et < etype->GetTechniqueCount(); ++et) {
    const FCDENode* enode = ((const FCDENode*)(etype->GetTechnique(et)))->FindChildNode("double_sided");
    if (enode != nullptr) {
      string content = trim(enode->GetContent());
      if (content == "1" || content == "true") {
        _materials[semantic]->_double_sided = true;
      } else if (content == "0" || content == "false") {
        _materials[semantic]->_double_sided = false;
      } else {
        daeegg_cat.warning() << "Expected <double_sided> tag to be either 1 or 0, found '" << content << "' instead" << endl;
      }
    }
  }
}

/**
 * Applies the stuff to the given EggPrimitive.
 */
void DaeMaterials::
apply_to_primitive(const string semantic, const PT(EggPrimitive) to) {
  if (_materials.count(semantic) > 0) {
    to->set_material(_materials[semantic]->_egg_material);
    for (pvector<PT_EggTexture>::iterator it = _materials[semantic]->_egg_textures.begin(); it != _materials[semantic]->_egg_textures.end(); ++it) {
      if (daeegg_cat.is_spam()) {
        daeegg_cat.spam() << "Applying texture " << (*it)->get_name() << " from material with semantic " << semantic << endl;
      }
      to->add_texture(*it);
    }
    to->set_bface_flag(_materials[semantic]->_double_sided);
  }
}

/**
 * Applies the colorblend stuff to the given EggGroup.
 */
void DaeMaterials::
apply_to_group(const string semantic, const PT(EggGroup) to, bool invert_transparency) {
  if (_materials.count(semantic) > 0) {
    PT(DaeBlendSettings) blend = _materials[semantic]->_blend;
    if (blend && blend->_enabled) {
      to->set_blend_mode(EggGroup::BM_add);
      to->set_blend_color(blend->_color);
      if (invert_transparency) {
        to->set_blend_operand_a(blend->_operand_b);
        to->set_blend_operand_b(blend->_operand_a);
      } else {
        to->set_blend_operand_a(blend->_operand_a);
        to->set_blend_operand_b(blend->_operand_b);
      }
    } else if (blend && invert_transparency) {
      to->set_blend_mode(EggGroup::BM_add);
      to->set_blend_color(blend->_color);
      to->set_blend_operand_a(blend->_operand_b);
      to->set_blend_operand_b(blend->_operand_a);
    }
  }
}

/**
 * Returns the semantic of the uvset with the specified input set, or an empty
 * string if the given material has no input set.
 */
const string DaeMaterials::
get_uvset_name(const string semantic, FUDaeGeometryInput::Semantic input_semantic, int32 input_set) {
  if (_materials.count(semantic) > 0) {
    if (input_set == -1 && _materials[semantic]->_uvsets.size() == 1) {
      return _materials[semantic]->_uvsets[0]->_semantic;
    } else {
      for (size_t i = 0; i < _materials[semantic]->_uvsets.size(); ++i) {
        if (_materials[semantic]->_uvsets[i]->_input_set == input_set &&
            _materials[semantic]->_uvsets[i]->_input_semantic == input_semantic) {
          return _materials[semantic]->_uvsets[i]->_semantic;
        }
      }
      // If we can't find it, let's look again, but don't care for the
      // input_semantic this time.  The reason for this is that some tools
      // export textangents and texbinormals bound to a uvset with input
      // semantic TEXCOORD.
      for (size_t i = 0; i < _materials[semantic]->_uvsets.size(); ++i) {
        if (_materials[semantic]->_uvsets[i]->_input_set == input_set) {
          if (daeegg_cat.is_debug()) {
            daeegg_cat.debug() << "Using uv set with non-matching input semantic " << _materials[semantic]->_uvsets[i]->_semantic << "\n";
          }
          return _materials[semantic]->_uvsets[i]->_semantic;
        }
      }
      if (daeegg_cat.is_debug()) {
        daeegg_cat.debug() << "No uv set binding found for input set " << input_set << "\n";
      }
    }
  }
  return "";
}

/**
 * Converts an FCollada sampler type to the EggTexture texture type
 * equivalent.
 */
EggTexture::TextureType DaeMaterials::
convert_texture_type(const FCDEffectParameterSampler::SamplerType orig_type) {
  switch (orig_type) {
    case FCDEffectParameterSampler::SAMPLER1D:
      return EggTexture::TT_1d_texture;
    case FCDEffectParameterSampler::SAMPLER2D:
      return EggTexture::TT_2d_texture;
    case FCDEffectParameterSampler::SAMPLER3D:
      return EggTexture::TT_3d_texture;
    case FCDEffectParameterSampler::SAMPLERCUBE:
      return EggTexture::TT_cube_map;
    default:
      daeegg_cat.warning() << "Invalid sampler type found" << endl;
  }
  return EggTexture::TT_unspecified;
}

/**
 * Converts an FCollada wrap mode to the EggTexture wrap mode equivalent.
 */
EggTexture::WrapMode DaeMaterials::
convert_wrap_mode(const FUDaeTextureWrapMode::WrapMode orig_mode) {
  switch (orig_mode) {
    case FUDaeTextureWrapMode::NONE:
      // FIXME: this shouldnt be unspecified
      return EggTexture::WM_unspecified;
    case FUDaeTextureWrapMode::WRAP:
      return EggTexture::WM_repeat;
    case FUDaeTextureWrapMode::MIRROR:
      return EggTexture::WM_mirror;
    case FUDaeTextureWrapMode::CLAMP:
      return EggTexture::WM_clamp;
    case FUDaeTextureWrapMode::BORDER:
      return EggTexture::WM_border_color;
    case FUDaeTextureWrapMode::UNKNOWN:
      return EggTexture::WM_unspecified;
    default:
      daeegg_cat.warning() << "Invalid wrap mode found: " << FUDaeTextureWrapMode::ToString(orig_mode) << endl;
  }
  return EggTexture::WM_unspecified;
}

/**
 * Converts an FCollada filter function to the EggTexture wrap type
 * equivalent.
 */
EggTexture::FilterType DaeMaterials::
convert_filter_type(const FUDaeTextureFilterFunction::FilterFunction orig_type) {
  switch (orig_type) {
    case FUDaeTextureFilterFunction::NONE:
      // FIXME: this shouldnt be unspecified
      return EggTexture::FT_unspecified;
    case FUDaeTextureFilterFunction::NEAREST:
      return EggTexture::FT_nearest;
    case FUDaeTextureFilterFunction::LINEAR:
      return EggTexture::FT_linear;
    case FUDaeTextureFilterFunction::NEAREST_MIPMAP_NEAREST:
      return EggTexture::FT_nearest_mipmap_nearest;
    case FUDaeTextureFilterFunction::LINEAR_MIPMAP_NEAREST:
      return EggTexture::FT_linear_mipmap_nearest;
    case FUDaeTextureFilterFunction::NEAREST_MIPMAP_LINEAR:
      return EggTexture::FT_nearest_mipmap_linear;
    case FUDaeTextureFilterFunction::LINEAR_MIPMAP_LINEAR:
      return EggTexture::FT_linear_mipmap_linear;
    case FUDaeTextureFilterFunction::UNKNOWN:
      return EggTexture::FT_unspecified;
    default:
      daeegg_cat.warning() << "Unknown filter type found: " << FUDaeTextureFilterFunction::ToString(orig_type) << endl;
  }
  return EggTexture::FT_unspecified;
}

/**
 * Converts collada blend attribs to Panda's equivalents.
 */
PT(DaeMaterials::DaeBlendSettings) DaeMaterials::
convert_blend(FCDEffectStandard::TransparencyMode mode, const LColor &transparent, double transparency) {
  // Create the DaeBlendSettings and fill it with some defaults.
  PT(DaeBlendSettings) blend = new DaeBlendSettings();
  blend->_enabled = true;
  blend->_color = LColor::zero();
  blend->_operand_a = EggGroup::BO_unspecified;
  blend->_operand_b = EggGroup::BO_unspecified;

  // First fill in the color value.
  if (mode == FCDEffectStandard::A_ONE) {// || mode == FCDEffectStandard::A_ZERO) {
    double value = transparent[3] * transparency;
    blend->_color = LColor(value, value, value, value);
  } else if (mode == FCDEffectStandard::RGB_ZERO) {//|| mode == FCDEffectStandard::RGB_ONE) {
    blend->_color = transparent * transparency;
    blend->_color[3] = luminance(blend->_color);
  } else {
    daeegg_cat.error() << "Unknown opaque type found!" << endl;
    blend->_enabled = false;
    return blend;
  }

  // Now figure out the operands.
  if (mode == FCDEffectStandard::RGB_ZERO) {// || mode == FCDEffectStandard::A_ZERO) {
    blend->_operand_a = EggGroup::BO_one_minus_constant_color;
    blend->_operand_b = EggGroup::BO_constant_color;
  } else if (mode == FCDEffectStandard::A_ONE) {// || mode == FCDEffectStandard::RGB_ONE) {
    blend->_operand_a = EggGroup::BO_constant_color;
    blend->_operand_b = EggGroup::BO_one_minus_constant_color;
  } else {
    daeegg_cat.error() << "Unknown opaque type found!" << endl;
    blend->_enabled = false;
    return blend;
  }

  // See if we can optimize out the color.
  if (blend->_operand_a == EggGroup::BO_constant_color) {
    if (blend->_color == LColor::zero()) {
      blend->_operand_a = EggGroup::BO_zero;
    } else if (blend->_color == LColor(1, 1, 1, 1)) {
      blend->_operand_a = EggGroup::BO_one;
    }
  }
  if (blend->_operand_b == EggGroup::BO_constant_color) {
    if (blend->_color == LColor::zero()) {
      blend->_operand_b = EggGroup::BO_zero;
    } else if (blend->_color == LColor(1, 1, 1, 1)) {
      blend->_operand_b = EggGroup::BO_one;
    }
  }
  if (blend->_operand_a == EggGroup::BO_one_minus_constant_color) {
    if (blend->_color == LColor::zero()) {
      blend->_operand_a = EggGroup::BO_one;
    } else if (blend->_color == LColor(1, 1, 1, 1)) {
      blend->_operand_a = EggGroup::BO_zero;
    }
  }
  if (blend->_operand_b == EggGroup::BO_one_minus_constant_color) {
    if (blend->_color == LColor::zero()) {
      blend->_operand_b = EggGroup::BO_one;
    } else if (blend->_color == LColor(1, 1, 1, 1)) {
      blend->_operand_b = EggGroup::BO_zero;
    }
  }

  // See if we can entirely disable the blend.
  if (blend->_operand_a == EggGroup::BO_one && blend->_operand_b == EggGroup::BO_zero) {
    blend->_enabled = false;
  }
  return blend;
}
