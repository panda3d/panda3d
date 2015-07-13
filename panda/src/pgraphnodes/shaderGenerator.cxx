// Filename: shaderGenerator.cxx
// Created by: jyelon (15Dec07)
// Updated by: weifengh, PandaSE(15Apr10)
// Updated by: agartner, PandaSE(16Apr10) - bug fix to synthesize_shader;
// TextureStage::M_modulate (before this, separate textures formatted as
// alpha wiped color off resulting rgb)
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
#include "spotlight.h"
#include "lightLensNode.h"
#include "lvector4.h"
#include "config_pgraphnodes.h"

TypeHandle ShaderGenerator::_type_handle;

#ifdef HAVE_CG

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::Constructor
//       Access: Published
//  Description: Create a ShaderGenerator.  This has no state,
//               except possibly to cache certain results.
//               The parameter that must be passed is the GSG to
//               which the shader generator belongs.
////////////////////////////////////////////////////////////////////
ShaderGenerator::
ShaderGenerator(GraphicsStateGuardianBase *gsg, GraphicsOutputBase *host) :
  _gsg(gsg), _host(host) {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::Destructor
//       Access: Published, Virtual
//  Description: Destroy a ShaderGenerator.
////////////////////////////////////////////////////////////////////
ShaderGenerator::
~ShaderGenerator() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::reset_register_allocator
//       Access: Protected
//  Description: Clears the register allocator.  Initially, the pool
//               of available registers is empty.  You have to add
//               some if you want there to be any.
////////////////////////////////////////////////////////////////////
void ShaderGenerator::
reset_register_allocator() {
  _vtregs_used = 0;
  _vcregs_used = 0;
  _ftregs_used = 0;
  _fcregs_used = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::alloc_vreg
//       Access: Protected
//  Description: Allocate a vreg.
////////////////////////////////////////////////////////////////////
const char *ShaderGenerator::
alloc_vreg() {
  // The ATTR# input sseem to map to generic vertex attributes in
  // both arbvp1 and glslv, which behave more consistently.
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
  // These don't exist in arbvp1, though they're reportedly
  // supported by other profiles.
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

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::alloc_freg
//       Access: Protected
//  Description: Allocate a freg.
////////////////////////////////////////////////////////////////////
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
  // We really shouldn't rely on COLOR fregs,
  // since the clamping can have unexpected side-effects.
  //switch (_fcregs_used) {
  //case  0: _fcregs_used += 1; return "COLOR0";
  //case  1: _fcregs_used += 1; return "COLOR1";
  //}
  // These don't exist in arbvp1/arbfp1, though they're
  // reportedly supported by other profiles.
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

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::analyze_renderstate
//       Access: Protected
//  Description: Analyzes the RenderState prior to shader generation.
//               The results of the analysis are stored in instance
//               variables of the Shader Generator.
////////////////////////////////////////////////////////////////////
void ShaderGenerator::
analyze_renderstate(const RenderState *rs) {
  clear_analysis();

  //  verify_enforce_attrib_lock();
  _state = rs;
  const AuxBitplaneAttrib *aux_bitplane = DCAST(AuxBitplaneAttrib, rs->get_attrib_def(AuxBitplaneAttrib::get_class_slot()));
  int outputs = aux_bitplane->get_outputs();

  // Decide whether or not we need alpha testing or alpha blending.

  const AlphaTestAttrib *alpha_test = DCAST(AlphaTestAttrib, rs->get_attrib_def(AlphaTestAttrib::get_class_slot()));
  if ((alpha_test->get_mode() != RenderAttrib::M_none)&&
      (alpha_test->get_mode() != RenderAttrib::M_always)) {
    _have_alpha_test = true;
  }
  const ColorBlendAttrib *color_blend = DCAST(ColorBlendAttrib, rs->get_attrib_def(ColorBlendAttrib::get_class_slot()));
  if (color_blend->get_mode() != ColorBlendAttrib::M_none) {
    _have_alpha_blend = true;
  }
  const TransparencyAttrib *transparency = DCAST(TransparencyAttrib, rs->get_attrib_def(TransparencyAttrib::get_class_slot()));
  if ((transparency->get_mode() == TransparencyAttrib::M_alpha)||
      (transparency->get_mode() == TransparencyAttrib::M_dual)) {
    _have_alpha_blend = true;
  }

  // Decide what to send to the framebuffer alpha, if anything.

  if (outputs & AuxBitplaneAttrib::ABO_glow) {
    if (_have_alpha_blend) {
      _calc_primary_alpha = true;
      _out_primary_glow = false;
      _disable_alpha_write = true;
    } else if (_have_alpha_test) {
      _calc_primary_alpha = true;
      _out_primary_glow = true;
      _subsume_alpha_test = true;
    } else {
      _calc_primary_alpha = false;
      _out_primary_glow = true;
    }
  } else {
    if (_have_alpha_blend || _have_alpha_test) {
      _calc_primary_alpha = true;
    }
  }

  // Determine what to put into the aux bitplane.

  _out_aux_normal = (outputs & AuxBitplaneAttrib::ABO_aux_normal) ? true:false;
  _out_aux_glow = (outputs & AuxBitplaneAttrib::ABO_aux_glow) ? true:false;
  _out_aux_any = (_out_aux_normal || _out_aux_glow);

  if (_out_aux_normal) {
    _need_eye_normal = true;
  }

  // Count number of textures.

  const TextureAttrib *texture = DCAST(TextureAttrib, rs->get_attrib_def(TextureAttrib::get_class_slot()));
  _num_textures = texture->get_num_on_stages();

  // Determine whether or not vertex colors or flat colors are present.

  const ColorAttrib *color = DCAST(ColorAttrib, rs->get_attrib_def(ColorAttrib::get_class_slot()));
  if (color->get_color_type() == ColorAttrib::T_vertex) {
    _vertex_colors = true;
  } else if (color->get_color_type() == ColorAttrib::T_flat) {
    _flat_colors = true;
  }

  // Break out the lights by type.

  _shadows = false;
  const LightAttrib *la = DCAST(LightAttrib, rs->get_attrib_def(LightAttrib::get_class_slot()));
  for (int i=0; i<la->get_num_on_lights(); i++) {
    NodePath light = la->get_on_light(i);
    nassertv(!light.is_empty());
    PandaNode *light_obj = light.node();
    nassertv(light_obj != (PandaNode *)NULL);

    if (light_obj->get_type() == AmbientLight::get_class_type()) {
      _alights_np.push_back(light);
      _alights.push_back((AmbientLight*)light_obj);
    }
    else if (light_obj->get_type() == DirectionalLight::get_class_type()) {
      _dlights_np.push_back(light);
      _dlights.push_back((DirectionalLight*)light_obj);
      if (DCAST(LightLensNode, light_obj)->is_shadow_caster()) {
        _shadows = true;
      }
    }
    else if (light_obj->get_type() == PointLight::get_class_type()) {
      _plights_np.push_back(light);
      _plights.push_back((PointLight*)light_obj);
    }
    else if (light_obj->get_type() == Spotlight::get_class_type()) {
      _slights_np.push_back(light);
      _slights.push_back((Spotlight*)light_obj);
      if (DCAST(LightLensNode, light_obj)->is_shadow_caster()) {
        _shadows = true;
      }
    }
  }

  // See if there is a normal map, height map, gloss map, or glow map.
  // Also check if anything has TexGen.

  const TexGenAttrib *tex_gen = DCAST(TexGenAttrib, rs->get_attrib_def(TexGenAttrib::get_class_slot()));
  for (int i=0; i<_num_textures; i++) {
    TextureStage *stage = texture->get_on_stage(i);
    TextureStage::Mode mode = stage->get_mode();
    if ((mode == TextureStage::M_normal)||
        (mode == TextureStage::M_normal_height)||
        (mode == TextureStage::M_normal_gloss)) {
      _map_index_normal = i;
    }
    if ((mode == TextureStage::M_height)||(mode == TextureStage::M_normal_height)) {
      _map_index_height = i;
    }
    if ((mode == TextureStage::M_glow)||(mode == TextureStage::M_modulate_glow)) {
      _map_index_glow = i;
    }
    if ((mode == TextureStage::M_gloss)||
        (mode == TextureStage::M_modulate_gloss)||
        (mode == TextureStage::M_normal_gloss)) {
      _map_index_gloss = i;
    }
    if (mode == TextureStage::M_height) {
      _map_height_in_alpha = false;
    }
    if (mode == TextureStage::M_normal_height) {
      _map_height_in_alpha = true;
    }
    if (tex_gen->has_stage(stage)) {
      switch (tex_gen->get_mode(stage)) {
      case TexGenAttrib::M_world_position:
        _need_world_position = true;
        break;
      case TexGenAttrib::M_world_normal:
        _need_world_normal = true;
        break;
      case TexGenAttrib::M_eye_position:
        _need_eye_position = true;
        break;
      case TexGenAttrib::M_eye_normal:
        _need_eye_normal = true;
        break;
      default:
        break;
      }
    }
  }

  // Determine whether lighting is needed.

  if (la->get_num_on_lights() > 0) {
    _lighting = true;
    _need_eye_normal = true;
  }

  // Determine whether we should normalize the normals.
  const RescaleNormalAttrib *rescale;
  rs->get_attrib_def(rescale);

  _normalize_normals = (rescale->get_mode() != RescaleNormalAttrib::M_none);

  // Find the material.

  const MaterialAttrib *material = DCAST(MaterialAttrib, rs->get_attrib_def(MaterialAttrib::get_class_slot()));

  if (!material->is_off()) {
    _material = material->get_material();
  } else {
    _material = Material::get_default();
  }

  // Decide which material modes need to be calculated.

  if (_lighting && (_alights.size() > 0)) {
    if (_material->has_ambient()) {
      LColor a = _material->get_ambient();
      if ((a[0]!=0.0)||(a[1]!=0.0)||(a[2]!=0.0)) {
        _have_ambient = true;
      }
    } else {
      _have_ambient = true;
    }
  }

  if (_lighting && (_dlights.size() + _plights.size() + _slights.size())) {
    if (_material->has_diffuse()) {
      LColor d = _material->get_diffuse();
      if ((d[0]!=0.0)||(d[1]!=0.0)||(d[2]!=0.0)) {
        _have_diffuse = true;
      }
    } else {
      _have_diffuse = true;
    }
  }

  if (_lighting && (_material->has_emission())) {
    LColor e = _material->get_emission();
    if ((e[0]!=0.0)||(e[1]!=0.0)||(e[2]!=0.0)) {
      _have_emission = true;
    }
  }

  if (_lighting && (_dlights.size() + _plights.size() + _slights.size())) {
    if (_material->has_specular()) {
      LColor s = _material->get_specular();
      if ((s[0]!=0.0)||(s[1]!=0.0)||(s[2]!=0.0)) {
        _have_specular = true;
      }
    } else if (_map_index_gloss >= 0) {
      _have_specular = true;
    }

    if (_plights.size() + _slights.size() > 0) {
      _need_eye_position = true;

    } else if (_have_specular && _material->get_local()) {
      _need_eye_position = true;
    }
  }

  // Decide whether to separate ambient and diffuse calculations.

  if (_have_ambient && _have_diffuse) {
    if (_material->has_ambient()) {
      if (_material->has_diffuse()) {
        _separate_ambient_diffuse = _material->get_ambient() != _material->get_diffuse();
      } else {
        _separate_ambient_diffuse = true;
      }
    } else {
      if (_material->has_diffuse()) {
        _separate_ambient_diffuse = true;
      } else {
        _separate_ambient_diffuse = false;
      }
    }
  }

  const LightRampAttrib *light_ramp = DCAST(LightRampAttrib, rs->get_attrib_def(LightRampAttrib::get_class_slot()));
  if (_lighting &&
      (light_ramp->get_mode() != LightRampAttrib::LRT_identity)) {
    _separate_ambient_diffuse = true;
  }

  // Do we want to use the ARB_shadow extension?
  // This also allows us to use hardware shadows / PCF.

  _use_shadow_filter = _gsg->get_supports_shadow_filter();

  // Does the shader need material properties as input?

  _need_material_props =
    (_have_ambient  && (_material->has_ambient()))||
    (_have_diffuse  && (_material->has_diffuse()))||
    (_have_emission && (_material->has_emission()))||
    (_have_specular && (_material->has_specular()));

  // Check for clip planes.

  const ClipPlaneAttrib *clip_plane = DCAST(ClipPlaneAttrib, rs->get_attrib_def(ClipPlaneAttrib::get_class_slot()));
  _num_clip_planes = clip_plane->get_num_on_planes();
  if (_num_clip_planes > 0) {
    _need_world_position = true;
  }

  const ShaderAttrib *shader_attrib = DCAST(ShaderAttrib, rs->get_attrib_def(ShaderAttrib::get_class_slot()));
  if (shader_attrib->auto_shader()) {
    _auto_normal_on = shader_attrib->auto_normal_on();
    _auto_glow_on   = shader_attrib->auto_glow_on();
    _auto_gloss_on  = shader_attrib->auto_gloss_on();
    _auto_ramp_on   = shader_attrib->auto_ramp_on();
    _auto_shadow_on = shader_attrib->auto_shadow_on();
  }

  // Check for fog.
  const FogAttrib *fog = DCAST(FogAttrib, rs->get_attrib_def(FogAttrib::get_class_slot()));
  if (!fog->is_off()) {
    _fog = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::clear_analysis
//       Access: Protected
//  Description: Called after analyze_renderstate to discard all
//               the results of the analysis.  This is generally done
//               after shader generation is complete.
////////////////////////////////////////////////////////////////////
void ShaderGenerator::
clear_analysis() {
  _vertex_colors = false;
  _flat_colors = false;
  _lighting = false;
  _shadows = false;
  _fog = false;
  _have_ambient = false;
  _have_diffuse = false;
  _have_emission = false;
  _have_specular = false;
  _separate_ambient_diffuse = false;
  _map_index_normal = -1;
  _map_index_glow = -1;
  _map_index_gloss = -1;
  _map_index_height = -1;
  _map_height_in_alpha = false;
  _calc_primary_alpha = false;
  _have_alpha_test = false;
  _have_alpha_blend = false;
  _subsume_alpha_test = false;
  _disable_alpha_write = false;
  _num_clip_planes = 0;
  _use_shadow_filter = false;
  _out_primary_glow  = false;
  _out_aux_normal   = false;
  _out_aux_glow     = false;
  _out_aux_any      = false;
  _material = (Material*)NULL;
  _need_material_props = false;
  _need_world_position = false;
  _need_world_normal = false;
  _need_eye_position = false;
  _need_eye_normal = false;
  _normalize_normals = false;
  _auto_normal_on = false;
  _auto_glow_on   = false;
  _auto_gloss_on  = false;
  _auto_ramp_on   = false;
  _auto_shadow_on = false;

  _alights.clear();
  _dlights.clear();
  _plights.clear();
  _slights.clear();
  _alights_np.clear();
  _dlights_np.clear();
  _plights_np.clear();
  _slights_np.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::create_shader_attrib
//       Access: Protected
//  Description: Creates a ShaderAttrib given a generated shader's
//               body.  Also inserts the lights into the shader
//               attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderGenerator::
create_shader_attrib(const string &txt) {
  PT(Shader) shader = Shader::make(txt, Shader::SL_Cg);
  CPT(RenderAttrib) shattr = ShaderAttrib::make();
  shattr = DCAST(ShaderAttrib, shattr)->set_shader(shader);
  if (_lighting) {
    for (int i=0; i < (int)_alights.size(); i++) {
      shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("alight", i), _alights_np[i]);
    }
    for (int i=0; i < (int)_dlights.size(); i++) {
      shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("dlight", i), _dlights_np[i]);
      if (_shadows && _dlights[i]->_shadow_caster) {
        PT(Texture) tex = update_shadow_buffer(_dlights_np[i]);
        if (tex == NULL) {
          pgraph_cat.error() << "Failed to create shadow buffer for DirectionalLight '" << _dlights[i]->get_name() << "'!\n";
        }
        shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("dlighttex", i), tex);
      } else {
        _dlights[i]->clear_shadow_buffers();
      }
    }
    for (int i=0; i < (int)_plights.size(); i++) {
      shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("plight", i), _plights_np[i]);
    }
    for (int i=0; i < (int)_slights.size(); i++) {
      shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("slight", i), _slights_np[i]);
      if (_shadows && _slights[i]->_shadow_caster) {
        PT(Texture) tex = update_shadow_buffer(_slights_np[i]);
        if (tex == NULL) {
          pgraph_cat.error() << "Failed to create shadow buffer for Spotlight '" << _slights[i]->get_name() << "'!\n";
        }
        shattr = DCAST(ShaderAttrib, shattr)->set_shader_input(InternalName::make("slighttex", i), tex);
      } else {
        _slights[i]->clear_shadow_buffers();
      }
    }
  }
  return shattr;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::update_shadow_buffer
//       Access: Protected, Virtual
//  Description: Updates the depth buffer of the specified light,
//               if it is configured to cast shadows.
//               Only call this function for DirectionalLights
//               and Spotlights. Returns the depth texture.
////////////////////////////////////////////////////////////////////
PT(Texture) ShaderGenerator::
update_shadow_buffer(NodePath light_np) {
  // Make sure everything is valid.
  nassertr(light_np.node()->is_of_type(DirectionalLight::get_class_type()) ||
           light_np.node()->is_of_type(Spotlight::get_class_type()), NULL);
  PT(LightLensNode) light = DCAST(LightLensNode, light_np.node());
  if (light == NULL || !light->_shadow_caster) {
    return NULL;
  }

  // See if we already have a buffer. If not, create one.
  Texture *tex;
  if (light->_sbuffers.count(_gsg) == 0) {
    // Nope, the light doesn't have a buffer for our GSG. Make one.
    tex = _gsg->make_shadow_buffer(light_np, _host);
  } else {
    // There's already a buffer - use that.
    tex = light->_sbuffers[_gsg]->get_texture();
  }
  nassertr(tex != NULL, NULL);

  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::synthesize_shader
//       Access: Published, Virtual
//  Description: This is the routine that implements the next-gen
//               fixed function pipeline by synthesizing a shader.
//               It also takes care of setting up any buffers
//               needed to produce the requested effects.
//
//               Currently supports:
//               - flat colors
//               - vertex colors
//               - lighting
//               - normal maps, but not multiple
//               - gloss maps, but not multiple
//               - glow maps, but not multiple
//               - materials, but not updates to materials
//               - 2D textures
//               - all texture stage modes, including combine modes
//               - color scale attrib
//               - light ramps (for cartoon shading)
//               - shadow mapping
//               - most texgen modes
//               - texmatrix
//               - 1D/2D/3D textures, cube textures, 2D tex arrays
//               - linear/exp/exp2 fog
//               - animation
//
//               Not yet supported:
//               - dot3_rgb and dot3_rgba combine modes
//
//               Potential optimizations
//               - omit attenuation calculations if attenuation off
//
////////////////////////////////////////////////////////////////////
CPT(ShaderAttrib) ShaderGenerator::
synthesize_shader(const RenderState *rs, const GeomVertexAnimationSpec &anim) {
  analyze_renderstate(rs);
  reset_register_allocator();

  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Generating shader for render state " << *rs << "\n";
  }

  // These variables will hold the results of register allocation.

  const char *tangent_freg = 0;
  const char *binormal_freg = 0;
  string tangent_input;
  string binormal_input;
  pmap<const InternalName *, const char *> texcoord_fregs;
  pvector<const char *> dlightcoord_fregs;
  pvector<const char *> slightcoord_fregs;
  const char *world_position_freg = 0;
  const char *world_normal_freg = 0;
  const char *eye_position_freg = 0;
  const char *eye_normal_freg = 0;
  const char *hpos_freg = 0;

  if (_vertex_colors) {
    // Reserve COLOR0
    _vcregs_used = 1;
    _fcregs_used = 1;
  }

  // Generate the shader's text.

  ostringstream text;

  text << "//Cg\n";

  text << "/* Generated shader for render state:\n";
  rs->write(text, 2);
  text << "*/\n";

  text << "void vshader(\n";
  const TextureAttrib *texture = DCAST(TextureAttrib, rs->get_attrib_def(TextureAttrib::get_class_slot()));
  const TexGenAttrib *tex_gen = DCAST(TexGenAttrib, rs->get_attrib_def(TexGenAttrib::get_class_slot()));
  for (int i = 0; i < _num_textures; ++i) {
    TextureStage *stage = texture->get_on_stage(i);
    if (!tex_gen->has_stage(stage)) {
      const InternalName *texcoord_name = stage->get_texcoord_name();

      if (texcoord_fregs.count(texcoord_name) == 0) {
        const char *freg = alloc_freg();
        string tcname = texcoord_name->join("_");
        texcoord_fregs[texcoord_name] = freg;

        text << "\t in float4 vtx_" << tcname << " : " << alloc_vreg() << ",\n";
        text << "\t out float4 l_" << tcname << " : " << freg << ",\n";
      }
    }

    if ((_map_index_normal == i && (_lighting || _out_aux_normal) && _auto_normal_on) || _map_index_height == i) {
      const InternalName *texcoord_name = stage->get_texcoord_name();
      PT(InternalName) tangent_name = InternalName::get_tangent();
      PT(InternalName) binormal_name = InternalName::get_binormal();

      if (texcoord_name != InternalName::get_texcoord()) {
        tangent_name = tangent_name->append(texcoord_name->get_basename());
        binormal_name = binormal_name->append(texcoord_name->get_basename());
      }
      tangent_input = tangent_name->join("_");
      binormal_input = binormal_name->join("_");

      text << "\t in float4 vtx_" << tangent_input << " : " << alloc_vreg() << ",\n";
      text << "\t in float4 vtx_" << binormal_input << " : " << alloc_vreg() << ",\n";

      if (_map_index_normal == i && (_lighting || _out_aux_normal) && _auto_normal_on) {
        tangent_freg = alloc_freg();
        binormal_freg = alloc_freg();
        text << "\t out float4 l_tangent : " << tangent_freg << ",\n";
        text << "\t out float4 l_binormal : " << binormal_freg << ",\n";
      }
    }
  }
  if (_vertex_colors) {
    text << "\t in float4 vtx_color : ATTR3,\n";
    text << "\t out float4 l_color : COLOR0,\n";
  }
  if (_need_world_position || _need_world_normal) {
    text << "\t uniform float4x4 trans_model_to_world,\n";
  }
  if (_need_world_position) {
    world_position_freg = alloc_freg();
    text << "\t out float4 l_world_position : " << world_position_freg << ",\n";
  }
  if (_need_world_normal) {
    world_normal_freg = alloc_freg();
    text << "\t out float4 l_world_normal : " << world_normal_freg << ",\n";
  }
  if (_need_eye_position) {
    text << "\t uniform float4x4 trans_model_to_view,\n";
    eye_position_freg = alloc_freg();
    text << "\t out float4 l_eye_position : " << eye_position_freg << ",\n";
  } else if ((_lighting || _out_aux_normal) && (_map_index_normal >= 0 && _auto_normal_on)) {
    text << "\t uniform float4x4 trans_model_to_view,\n";
  }
  if (_need_eye_normal) {
    eye_normal_freg = alloc_freg();
    text << "\t uniform float4x4 tpose_view_to_model,\n";
    text << "\t out float4 l_eye_normal : " << eye_normal_freg << ",\n";
  }
  if (_map_index_height >= 0 || _need_world_normal || _need_eye_normal) {
    text << "\t in float3 vtx_normal : ATTR2,\n";
  }
  if (_map_index_height >= 0) {
    text << "\t uniform float4 mspos_view,\n";
    text << "\t out float3 l_eyevec,\n";
  }
  if (_lighting) {
    if (_shadows && _auto_shadow_on) {
      for (int i=0; i < (int)_dlights.size(); i++) {
        if (_dlights[i]->_shadow_caster) {
          dlightcoord_fregs.push_back(alloc_freg());
          text << "\t uniform float4x4 trans_model_to_clip_of_dlight" << i << ",\n";
          text << "\t out float4 l_dlightcoord" << i << " : " << dlightcoord_fregs[i] << ",\n";
        } else {
          dlightcoord_fregs.push_back(NULL);
        }
      }
      for (int i=0; i < (int)_slights.size(); i++) {
        if (_slights[i]->_shadow_caster) {
          slightcoord_fregs.push_back(alloc_freg());
          text << "\t uniform float4x4 trans_model_to_clip_of_slight" << i << ",\n";
          text << "\t out float4 l_slightcoord" << i << " : " << slightcoord_fregs[i] << ",\n";
        } else {
          slightcoord_fregs.push_back(NULL);
        }
      }
    }
  }
  if (_fog) {
    hpos_freg = alloc_freg();
    text << "\t out float4 l_hpos : " << hpos_freg << ",\n";
  }
  if (anim.get_animation_type() == GeomEnums::AT_hardware &&
      anim.get_num_transforms() > 0) {
    int num_transforms;
    if (anim.get_indexed_transforms()) {
      num_transforms = 120;
    } else {
      num_transforms = anim.get_num_transforms();
    }
    text << "\t uniform float4x4 tbl_transforms[" << num_transforms << "],\n";
    text << "\t in float4 vtx_transform_weight : ATTR1,\n";
    if (anim.get_indexed_transforms()) {
      text << "\t in uint4 vtx_transform_index : ATTR7,\n";
    }
  }
  text << "\t in float4 vtx_position : ATTR0,\n";
  text << "\t out float4 l_position : POSITION,\n";
  text << "\t uniform float4x4 mat_modelproj\n";
  text << ") {\n";

  if (anim.get_animation_type() == GeomEnums::AT_hardware &&
      anim.get_num_transforms() > 0) {

    if (!anim.get_indexed_transforms()) {
      text << "\t const uint4 vtx_transform_index = uint4(0, 1, 2, 3);\n";
    }

    text << "\t float4x4 matrix = tbl_transforms[vtx_transform_index.x] * vtx_transform_weight.x";
    if (anim.get_num_transforms() > 1) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.y] * vtx_transform_weight.y";
    }
    if (anim.get_num_transforms() > 2) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.z] * vtx_transform_weight.z";
    }
    if (anim.get_num_transforms() > 3) {
      text << "\n\t                 + tbl_transforms[vtx_transform_index.w] * vtx_transform_weight.w";
    }
    text << ";\n";

    text << "\t vtx_position = mul(matrix, vtx_position);\n";
    if (_need_world_normal || _need_eye_normal) {
      text << "\t vtx_normal = mul((float3x3)matrix, vtx_normal);\n";
    }
  }

  text << "\t l_position = mul(mat_modelproj, vtx_position);\n";
  if (_fog) {
    text << "\t l_hpos = l_position;\n";
  }
  if (_need_world_position) {
    text << "\t l_world_position = mul(trans_model_to_world, vtx_position);\n";
  }
  if (_need_world_normal) {
    text << "\t l_world_normal = mul(trans_model_to_world, float4(vtx_normal, 0));\n";
  }
  if (_need_eye_position) {
    text << "\t l_eye_position = mul(trans_model_to_view, vtx_position);\n";
  }
  if (_need_eye_normal) {
    if (_normalize_normals) {
      text << "\t l_eye_normal.xyz = normalize(mul((float3x3)tpose_view_to_model, vtx_normal));\n";
    } else {
      text << "\t l_eye_normal.xyz = mul((float3x3)tpose_view_to_model, vtx_normal);\n";
    }
    text << "\t l_eye_normal.w = 0;\n";
  }
  pmap<const InternalName *, const char *>::const_iterator it;
  for (it = texcoord_fregs.begin(); it != texcoord_fregs.end(); ++it) {
    // Pass through all texcoord inputs as-is.
    string tcname = it->first->join("_");
    text << "\t l_" << tcname << " = vtx_" << tcname << ";\n";
  }
  if (_vertex_colors) {
    text << "\t l_color = vtx_color;\n";
  }
  if ((_lighting || _out_aux_normal) && (_map_index_normal >= 0 && _auto_normal_on)) {
    text << "\t l_tangent.xyz = normalize(mul((float3x3)trans_model_to_view, vtx_" << tangent_input << ".xyz));\n";
    text << "\t l_tangent.w = 0;\n";
    text << "\t l_binormal.xyz = normalize(mul((float3x3)trans_model_to_view, -vtx_" << binormal_input << ".xyz));\n";
    text << "\t l_binormal.w = 0;\n";
  }
  if (_shadows && _auto_shadow_on) {
    text << "\t float4x4 biasmat = {0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f};\n";
    for (int i=0; i < (int)_dlights.size(); i++) {
      if (_dlights[i]->_shadow_caster) {
        text << "\t l_dlightcoord" << i << " = mul(biasmat, mul(trans_model_to_clip_of_dlight" << i << ", vtx_position));\n";
      }
    }
    for (int i=0; i < (int)_slights.size(); i++) {
      if (_slights[i]->_shadow_caster) {
        text << "\t l_slightcoord" << i << " = mul(biasmat, mul(trans_model_to_clip_of_slight" << i << ", vtx_position));\n";
      }
    }
  }
  if (_map_index_height >= 0) {
    text << "\t float3 eyedir = mspos_view.xyz - vtx_position.xyz;\n";
    text << "\t l_eyevec.x = dot(vtx_" << tangent_input << ".xyz, eyedir);\n";
    text << "\t l_eyevec.y = dot(vtx_" << binormal_input << ".xyz, eyedir);\n";
    text << "\t l_eyevec.z = dot(vtx_normal, eyedir);\n";
    text << "\t l_eyevec = normalize(l_eyevec);\n";
  }
  text << "}\n\n";

  // Fragment shader

  text << "void fshader(\n";
  if (_fog) {
    text << "\t in float4 l_hpos : " << hpos_freg << ",\n";
    text << "\t in uniform float4 attr_fog,\n";
    text << "\t in uniform float4 attr_fogcolor,\n";
  }
  if (_need_world_position) {
    text << "\t in float4 l_world_position : " << world_position_freg << ",\n";
  }
  if (_need_world_normal) {
    text << "\t in float4 l_world_normal : " << world_normal_freg << ",\n";
  }
  if (_need_eye_position) {
    text << "\t in float4 l_eye_position : " << eye_position_freg << ",\n";
  }
  if (_need_eye_normal) {
    text << "\t in float4 l_eye_normal : " << eye_normal_freg << ",\n";
  }
  for (it = texcoord_fregs.begin(); it != texcoord_fregs.end(); ++it) {
    text << "\t in float4 l_" << it->first->join("_") << " : " << it->second << ",\n";
  }
  const TexMatrixAttrib *tex_matrix = DCAST(TexMatrixAttrib, rs->get_attrib_def(TexMatrixAttrib::get_class_slot()));
  for (int i=0; i<_num_textures; i++) {
    TextureStage *stage = texture->get_on_stage(i);
    Texture *tex = texture->get_on_texture(stage);
    nassertr(tex != NULL, NULL);
    text << "\t uniform sampler" << texture_type_as_string(tex->get_texture_type()) << " tex_" << i << ",\n";
    if (tex_matrix->has_stage(stage)) {
      text << "\t uniform float4x4 texmat_" << i << ",\n";
    }
  }
  if ((_lighting || _out_aux_normal) && (_map_index_normal >= 0 && _auto_normal_on)) {
    text << "\t in float3 l_tangent : " << tangent_freg << ",\n";
    text << "\t in float3 l_binormal : " << binormal_freg << ",\n";
  }
  if (_lighting) {
    for (int i=0; i < (int)_alights.size(); i++) {
      text << "\t uniform float4 alight_alight" << i << ",\n";
    }
    for (int i=0; i < (int)_dlights.size(); i++) {
      text << "\t uniform float4x4 dlight_dlight" << i << "_rel_view,\n";
      if (_shadows && _dlights[i]->_shadow_caster && _auto_shadow_on) {
        if (_use_shadow_filter) {
          text << "\t uniform sampler2DShadow k_dlighttex" << i << ",\n";
        } else {
          text << "\t uniform sampler2D k_dlighttex" << i << ",\n";
        }
        text << "\t in float4 l_dlightcoord" << i << " : " << dlightcoord_fregs[i] << ",\n";
      }
    }
    for (int i=0; i < (int)_plights.size(); i++) {
      text << "\t uniform float4x4 plight_plight" << i << "_rel_view,\n";
    }
    for (int i=0; i < (int)_slights.size(); i++) {
      text << "\t uniform float4x4 slight_slight" << i << "_rel_view,\n";
      text << "\t uniform float4   satten_slight" << i << ",\n";
      if (_shadows && _slights[i]->_shadow_caster && _auto_shadow_on) {
        if (_use_shadow_filter) {
          text << "\t uniform sampler2DShadow k_slighttex" << i << ",\n";
        } else {
          text << "\t uniform sampler2D k_slighttex" << i << ",\n";
        }
        text << "\t in float4 l_slightcoord" << i << " : " << slightcoord_fregs[i] << ",\n";
      }
    }
    if (_need_material_props) {
      text << "\t uniform float4x4 attr_material,\n";
    }
    if (_have_specular) {
      if (_material->get_local()) {
        text << "\t uniform float4 mspos_view,\n";
      } else {
        text << "\t uniform float4 row1_view_to_model,\n";
      }
    }
  }
  if (_map_index_height >= 0) {
    text << "\t float3 l_eyevec,\n";
  }
  if (_out_aux_any) {
    text << "\t out float4 o_aux : COLOR1,\n";
  }
  text << "\t out float4 o_color : COLOR0,\n";
  if (_vertex_colors) {
    text << "\t in float4 l_color : COLOR0,\n";
  } else {
    text << "\t uniform float4 attr_color,\n";
  }
  for (int i=0; i<_num_clip_planes; ++i) {
    text << "\t uniform float4 clipplane_" << i << ",\n";
  }
  text << "\t uniform float4 attr_colorscale\n";
  text << ") {\n";
  // Clipping first!
  for (int i=0; i<_num_clip_planes; ++i) {
    text << "\t if (l_world_position.x * clipplane_" << i << ".x + l_world_position.y ";
    text << "* clipplane_" << i << ".y + l_world_position.z * clipplane_" << i << ".z + clipplane_" << i << ".w <= 0) {\n";
    text << "\t discard;\n";
    text << "\t }\n";
  }
  text << "\t float4 result;\n";
  if (_out_aux_any) {
    text << "\t o_aux = float4(0, 0, 0, 0);\n";
  }
  // Now generate any texture coordinates according to TexGenAttrib. If it has a TexMatrixAttrib, also transform them.
  for (int i=0; i<_num_textures; i++) {
    TextureStage *stage = texture->get_on_stage(i);
    if (tex_gen != NULL && tex_gen->has_stage(stage)) {
      switch (tex_gen->get_mode(stage)) {
        case TexGenAttrib::M_world_position:
          text << "\t float4 texcoord" << i << " = l_world_position;\n";
          break;
        case TexGenAttrib::M_world_normal:
          text << "\t float4 texcoord" << i << " = l_world_normal;\n";
          break;
        case TexGenAttrib::M_eye_position:
          text << "\t float4 texcoord" << i << " = l_eye_position;\n";
          break;
        case TexGenAttrib::M_eye_normal:
          text << "\t float4 texcoord" << i << " = l_eye_normal;\n";
          text << "\t texcoord" << i << ".w = 1.0f;\n";
          break;
        default:
          pgraph_cat.error() << "Unsupported TexGenAttrib mode\n";
          text << "\t float4 texcoord" << i << " = float4(0, 0, 0, 0);\n";
      }
    } else {
      // Cg seems to be able to optimize this temporary away when appropriate.
      const InternalName *texcoord_name = stage->get_texcoord_name();
      text << "\t float4 texcoord" << i << " = l_" << texcoord_name->join("_") << ";\n";
    }
    if (tex_matrix != NULL && tex_matrix->has_stage(stage)) {
      text << "\t texcoord" << i << " = mul(texmat_" << i << ", texcoord" << i << ");\n";
      text << "\t texcoord" << i << ".xyz /= texcoord" << i << ".w;\n";
    }
  }
  text << "\t // Fetch all textures.\n";
  if (_map_index_height >= 0 && parallax_mapping_samples > 0) {
    Texture *tex = texture->get_on_texture(texture->get_on_stage(_map_index_height));
    nassertr(tex != NULL, NULL);
    text << "\t float4 tex" << _map_index_height << " = tex" << texture_type_as_string(tex->get_texture_type());
    text << "(tex_" << _map_index_height << ", texcoord" << _map_index_height << ".";
    switch (tex->get_texture_type()) {
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
    text << ");\n\t float3 parallax_offset = l_eyevec.xyz * (tex" << _map_index_height;
    if (_map_height_in_alpha) {
      text << ".aaa";
    } else {
      text << ".rgb";
    }
    text << " * 2.0 - 1.0) * " << parallax_mapping_scale << ";\n";
    // Additional samples
    for (int i=0; i<parallax_mapping_samples-1; i++) {
      text << "\t parallax_offset = l_eyevec.xyz * (parallax_offset + (tex" << _map_index_height;
      if (_map_height_in_alpha) {
        text << ".aaa";
      } else {
        text << ".rgb";
      }
      text << " * 2.0 - 1.0)) * " << 0.5 * parallax_mapping_scale << ";\n";
    }
  }
  for (int i=0; i<_num_textures; i++) {
    if (i != _map_index_height) {
      Texture *tex = texture->get_on_texture(texture->get_on_stage(i));
      nassertr(tex != NULL, NULL);
      // Parallax mapping pushes the texture coordinates of the other textures away from the camera.
      if (_map_index_height >= 0 && parallax_mapping_samples > 0) {
        text << "\t texcoord" << i << ".xyz -= parallax_offset;\n";
      }
      text << "\t float4 tex" << i << " = tex" << texture_type_as_string(tex->get_texture_type());
      text << "(tex_" << i << ", texcoord" << i << ".";
      switch(tex->get_texture_type()) {
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
  if (_lighting || _out_aux_normal) {
    if (_map_index_normal >= 0 && _auto_normal_on) {
      text << "\t // Translate tangent-space normal in map to view-space.\n";
      text << "\t float3 tsnormal = ((float3)tex" << _map_index_normal << " * 2) - 1;\n";
      text << "\t l_eye_normal.xyz *= tsnormal.z;\n";
      text << "\t l_eye_normal.xyz += l_tangent * tsnormal.x;\n";
      text << "\t l_eye_normal.xyz += l_binormal * tsnormal.y;\n";
    }
  }
  if (_need_eye_normal) {
    text << "\t // Correct the surface normal for interpolation effects\n";
    text << "\t l_eye_normal.xyz = normalize(l_eye_normal.xyz);\n";
  }
  if (_out_aux_normal) {
    text << "\t // Output the camera-space surface normal\n";
    text << "\t o_aux.rgb = (l_eye_normal.xyz*0.5) + float3(0.5,0.5,0.5);\n";
  }
  if (_lighting) {
    text << "\t // Begin view-space light calculations\n";
    text << "\t float ldist,lattenv,langle;\n";
    text << "\t float4 lcolor,lspec,lvec,lpoint,latten,ldir,leye,lhalf;\n";
    if (_shadows && _auto_shadow_on) {
      text << "\t float lshad;\n";
    }
    if (_separate_ambient_diffuse) {
      if (_have_ambient) {
        text << "\t float4 tot_ambient = float4(0,0,0,0);\n";
      }
      if (_have_diffuse) {
        text << "\t float4 tot_diffuse = float4(0,0,0,0);\n";
      }
    } else {
      if (_have_ambient || _have_diffuse) {
        text << "\t float4 tot_diffuse = float4(0,0,0,0);\n";
      }
    }
    if (_have_specular) {
      text << "\t float4 tot_specular = float4(0,0,0,0);\n";
      if (_material->has_specular()) {
        text << "\t float shininess = attr_material[3].w;\n";
      } else {
        text << "\t float shininess = 50; // no shininess specified, using default\n";
      }
    }
    for (int i=0; i < (int)_alights.size(); i++) {
      text << "\t // Ambient Light " << i << "\n";
      text << "\t lcolor = alight_alight" << i << ";\n";
      if (_separate_ambient_diffuse && _have_ambient) {
        text << "\t tot_ambient += lcolor;\n";
      } else if(_have_diffuse) {
        text << "\t tot_diffuse += lcolor;\n";
      }
    }
    for (int i=0; i < (int)_dlights.size(); i++) {
      text << "\t // Directional Light " << i << "\n";
      text << "\t lcolor = dlight_dlight" << i << "_rel_view[0];\n";
      text << "\t lspec  = dlight_dlight" << i << "_rel_view[1];\n";
      text << "\t lvec   = dlight_dlight" << i << "_rel_view[2];\n";
      text << "\t lcolor *= saturate(dot(l_eye_normal.xyz, lvec.xyz));\n";
      if (_shadows && _dlights[i]->_shadow_caster && _auto_shadow_on) {
        if (_use_shadow_filter) {
          text << "\t lshad = shadow2DProj(k_dlighttex" << i << ", l_dlightcoord" << i << ").r;\n";
        } else {
          text << "\t lshad = tex2Dproj(k_dlighttex" << i << ", l_dlightcoord" << i << ").r > l_dlightcoord" << i << ".z / l_dlightcoord" << i << ".w;\n";
        }
        text << "\t lcolor *= lshad;\n";
        text << "\t lspec *= lshad;\n";
      }
      if (_have_diffuse) {
        text << "\t tot_diffuse += lcolor;\n";
      }
      if (_have_specular) {
        if (_material->get_local()) {
          text << "\t lhalf  = normalize(lvec - normalize(l_eye_position));\n";
        } else {
          text << "\t lhalf = dlight_dlight" << i << "_rel_view[3];\n";
        }
        text << "\t lspec *= pow(saturate(dot(l_eye_normal.xyz, lhalf.xyz)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    }
    for (int i=0; i < (int)_plights.size(); i++) {
      text << "\t // Point Light " << i << "\n";
      text << "\t lcolor = plight_plight" << i << "_rel_view[0];\n";
      text << "\t lspec  = plight_plight" << i << "_rel_view[1];\n";
      text << "\t lpoint = plight_plight" << i << "_rel_view[2];\n";
      text << "\t latten = plight_plight" << i << "_rel_view[3];\n";
      text << "\t lvec   = lpoint - l_eye_position;\n";
      text << "\t ldist = length(float3(lvec));\n";
      text << "\t lvec /= ldist;\n";
      text << "\t lattenv = 1/(latten.x + latten.y*ldist + latten.z*ldist*ldist);\n";
      text << "\t lcolor *= lattenv * saturate(dot(l_eye_normal.xyz, lvec.xyz));\n";
      if (_have_diffuse) {
        text << "\t tot_diffuse += lcolor;\n";
      }
      if (_have_specular) {
        if (_material->get_local()) {
          text << "\t lhalf  = normalize(lvec - normalize(l_eye_position));\n";
        } else {
          text << "\t lhalf = normalize(lvec - float4(0, 1, 0, 0));\n";
        }
        text << "\t lspec *= lattenv;\n";
        text << "\t lspec *= pow(saturate(dot(l_eye_normal.xyz, lhalf.xyz)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    }
    for (int i=0; i < (int)_slights.size(); i++) {
      text << "\t // Spot Light " << i << "\n";
      text << "\t lcolor = slight_slight" << i << "_rel_view[0];\n";
      text << "\t lspec  = slight_slight" << i << "_rel_view[1];\n";
      text << "\t lpoint = slight_slight" << i << "_rel_view[2];\n";
      text << "\t ldir   = slight_slight" << i << "_rel_view[3];\n";
      text << "\t latten = satten_slight" << i << ";\n";
      text << "\t lvec   = lpoint - l_eye_position;\n";
      text << "\t ldist  = length(float3(lvec));\n";
      text << "\t lvec /= ldist;\n";
      text << "\t langle = saturate(dot(ldir.xyz, lvec.xyz));\n";
      text << "\t lattenv = 1/(latten.x + latten.y*ldist + latten.z*ldist*ldist);\n";
      text << "\t lattenv *= pow(langle, latten.w);\n";
      text << "\t if (langle < ldir.w) lattenv = 0;\n";
      text << "\t lcolor *= lattenv * saturate(dot(l_eye_normal.xyz, lvec.xyz));\n";
      if (_shadows && _slights[i]->_shadow_caster && _auto_shadow_on) {
        if (_use_shadow_filter) {
          text << "\t lshad = shadow2DProj(k_slighttex" << i << ", l_slightcoord" << i << ").r;\n";
        } else {
          text << "\t lshad = tex2Dproj(k_slighttex" << i << ", l_slightcoord" << i << ").r > l_slightcoord" << i << ".z / l_slightcoord" << i << ".w;\n";
        }
        text << "\t lcolor *= lshad;\n";
        text << "\t lspec *= lshad;\n";
      }

      if (_have_diffuse) {
        text << "\t tot_diffuse += lcolor;\n";
      }
      if (_have_specular) {
        if (_material->get_local()) {
          text << "\t lhalf  = normalize(lvec - normalize(l_eye_position));\n";
        } else {
          text << "\t lhalf = normalize(lvec - float4(0,1,0,0));\n";
        }
        text << "\t lspec *= lattenv;\n";
        text << "\t lspec *= pow(saturate(dot(l_eye_normal.xyz, lhalf.xyz)), shininess);\n";
        text << "\t tot_specular += lspec;\n";
      }
    }
    const LightRampAttrib *light_ramp = DCAST(LightRampAttrib, rs->get_attrib_def(LightRampAttrib::get_class_slot()));
    if (_auto_ramp_on && _have_diffuse) {
      switch (light_ramp->get_mode()) {
      case LightRampAttrib::LRT_single_threshold:
        {
          PN_stdfloat t = light_ramp->get_threshold(0);
          PN_stdfloat l0 = light_ramp->get_level(0);
          text << "\t // Single-threshold light ramp\n";
          text << "\t float lr_in = dot(tot_diffuse.rgb, float3(0.33,0.34,0.33));\n";
          text << "\t float lr_scale = (lr_in < " << t << ") ? 0.0 : (" << l0 << "/lr_in);\n";
          text << "\t tot_diffuse = tot_diffuse * lr_scale;\n";
          break;
        }
      case LightRampAttrib::LRT_double_threshold:
        {
          PN_stdfloat t0 = light_ramp->get_threshold(0);
          PN_stdfloat t1 = light_ramp->get_threshold(1);
          PN_stdfloat l0 = light_ramp->get_level(0);
          PN_stdfloat l1 = light_ramp->get_level(1);
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
    if (_have_emission) {
      if (_map_index_glow >= 0 && _auto_glow_on) {
        text << "\t result = attr_material[2] * saturate(2 * (tex" << _map_index_glow << ".a - 0.5));\n";
      } else {
        text << "\t result = attr_material[2];\n";
      }
    } else {
      if (_map_index_glow >= 0 && _auto_glow_on) {
        text << "\t result = saturate(2 * (tex" << _map_index_glow << ".a - 0.5));\n";
      } else {
        text << "\t result = float4(0,0,0,0);\n";
      }
    }
    if ((_have_ambient)&&(_separate_ambient_diffuse)) {
      if (_material->has_ambient()) {
        text << "\t result += tot_ambient * attr_material[0];\n";
      } else if (_vertex_colors) {
        text << "\t result += tot_ambient * l_color;\n";
      } else if (_flat_colors) {
        text << "\t result += tot_ambient * attr_color;\n";
      } else {
        text << "\t result += tot_ambient;\n";
      }
    }
    if (_have_diffuse) {
      if (_material->has_diffuse()) {
        text << "\t result += tot_diffuse * attr_material[1];\n";
      } else if (_vertex_colors) {
        text << "\t result += tot_diffuse * l_color;\n";
      } else if (_flat_colors) {
        text << "\t result += tot_diffuse * attr_color;\n";
      } else {
        text << "\t result += tot_diffuse;\n";
      }
    }
    if (light_ramp->get_mode() == LightRampAttrib::LRT_default) {
      text << "\t result = saturate(result);\n";
    }
    text << "\t // End view-space light calculations\n";

    // Combine in alpha, which bypasses lighting calculations.
    // Use of lerp here is a workaround for a radeon driver bug.
    if (_calc_primary_alpha) {
      if (_vertex_colors) {
        text << "\t result.a = l_color.a;\n";
      } else if (_flat_colors) {
        text << "\t result.a = attr_color.a;\n";
      } else {
        text << "\t result.a = 1;\n";
      }
    }
  } else {
    if (_vertex_colors) {
      text << "\t result = l_color;\n";
    } else if (_flat_colors) {
      text << "\t result = attr_color;\n";
    } else {
      text << "\t result = float4(1, 1, 1, 1);\n";
    }
  }

  // Loop first to see if something is using primary_color or last_saved_result.
  bool have_saved_result = false;
  bool have_primary_color = false;
  for (int i=0; i<_num_textures; i++) {
    TextureStage *stage = texture->get_on_stage(i);
    if (stage->get_mode() != TextureStage::M_combine) continue;
    if (stage->uses_primary_color() && !have_primary_color) {
      text << "\t float4 primary_color = result;\n";
      have_primary_color = true;
    }
    if (stage->uses_last_saved_result() && !have_saved_result) {
      text << "\t float4 last_saved_result = result;\n";
      have_saved_result = true;
    }
  }

  // Now loop through the textures to compose our magic blending formulas.
  for (int i=0; i<_num_textures; i++) {
    TextureStage *stage = texture->get_on_stage(i);
    switch (stage->get_mode()) {
    case TextureStage::M_modulate: {
      int num_components = texture->get_on_texture(texture->get_on_stage(i))->get_num_components();

      if (num_components == 1) {
        text << "\t result.a *= tex" << i << ".a;\n";
      } else if (num_components == 3) {
        text << "\t result.rgb *= tex" << i << ".rgb;\n";
      } else {
        text << "\t result.rgba *= tex" << i << ".rgba;\n";
      }

      break; }
    case TextureStage::M_modulate_glow:
    case TextureStage::M_modulate_gloss:
      //in the case of glow or spec we currently see the specularity evenly across the surface
      //even if transparency or masking is present
      //not sure if this is desired behavior or not.
      //*MOST* would construct a spec map based off of
      //what is/isn't seen based on the mask/transparency
      //this may have to be left alone for now
      //agartner
      text << "\t result.rgb *= tex" << i << ";\n";
      break;
    case TextureStage::M_decal:
      text << "\t result.rgb = lerp(result, tex" << i << ", tex" << i << ".a).rgb;\n";
      break;
    case TextureStage::M_blend: {
      LVecBase4 c = stage->get_color();
      text << "\t result.rgb = lerp(result, tex" << i << " * float4("
           << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << "), tex" << i << ".r).rgb;\n";
      break; }
    case TextureStage::M_replace:
      text << "\t result = tex" << i << ";\n";
      break;
    case TextureStage::M_add:
      text << "\t result.rgb += tex" << i << ".rgb;\n";
      if (_calc_primary_alpha) {
        text << "\t result.a   *= tex" << i << ".a;\n";
      }
      break;
    case TextureStage::M_combine:
      text << "\t result.rgb = ";
      if (stage->get_combine_rgb_mode() != TextureStage::CM_undefined) {
        text << combine_mode_as_string(stage, stage->get_combine_rgb_mode(), false, i);
      } else {
        text << "tex" << i << ".rgb";
      }
      if (stage->get_rgb_scale() != 1) {
        text << " * " << stage->get_rgb_scale();
      }
      text << ";\n\t result.a = ";
      if (stage->get_combine_alpha_mode() != TextureStage::CM_undefined) {
        text << combine_mode_as_string(stage, stage->get_combine_alpha_mode(), true, i);
      } else {
        text << "tex" << i << ".a";
      }
      if (stage->get_alpha_scale() != 1) {
        text << " * " << stage->get_alpha_scale();
      }
      text << ";\n";
      break;
    case TextureStage::M_blend_color_scale:
      text << "\t result.rgb = lerp(result, tex" << i << " * attr_colorscale, tex" << i << ".r).rgb;\n";
      break;
    default:
      break;
    }
    if (stage->get_saved_result() && have_saved_result) {
      text << "\t last_saved_result = result;\n";
    }
  }
  // Apply the color scale.
  text << "\t result *= attr_colorscale;\n";

  if (_subsume_alpha_test) {
    const AlphaTestAttrib *alpha_test = DCAST(AlphaTestAttrib, rs->get_attrib_def(AlphaTestAttrib::get_class_slot()));
    text << "\t // Shader includes alpha test:\n";
    double ref = alpha_test->get_reference_alpha();
    switch (alpha_test->get_mode()) {
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
    default:
      break;
    }
  }

  if (_out_primary_glow) {
    if (_map_index_glow >= 0 && _auto_glow_on) {
      text << "\t result.a = tex" << _map_index_glow << ".a;\n";
    } else {
      text << "\t result.a = 0.5;\n";
    }
  }
  if (_out_aux_glow) {
    if (_map_index_glow >= 0 && _auto_glow_on) {
      text << "\t o_aux.a = tex" << _map_index_glow << ".a;\n";
    } else {
      text << "\t o_aux.a = 0.5;\n";
    }
  }

  if (_lighting) {
    if (_have_specular) {
      if (_material->has_specular()) {
        text << "\t tot_specular *= attr_material[3];\n";
      }
      if (_map_index_gloss >= 0 && _auto_gloss_on) {
        text << "\t tot_specular *= tex" << _map_index_gloss << ".a;\n";
      }
      text << "\t result.rgb = result.rgb + tot_specular.rgb;\n";
    }
  }
  if (_auto_ramp_on) {
    const LightRampAttrib *light_ramp = DCAST(LightRampAttrib, rs->get_attrib_def(LightRampAttrib::get_class_slot()));
    switch (light_ramp->get_mode()) {
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
  if (_fog) {
    const FogAttrib *fog_attr = DCAST(FogAttrib, rs->get_attrib_def(FogAttrib::get_class_slot()));
    Fog *fog = fog_attr->get_fog();

    switch (fog->get_mode()) {
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

  // The multiply is a workaround for a radeon driver bug.
  // It's annoying as heck, since it produces an extra instruction.
  text << "\t o_color = result * 1.000001;\n";
  if (_subsume_alpha_test) {
    text << "\t // Shader subsumes normal alpha test.\n";
  }
  if (_disable_alpha_write) {
    text << "\t // Shader disables alpha write.\n";
  }
  text << "}\n";

  // Insert the shader into the shader attrib.
  CPT(RenderAttrib) shattr = create_shader_attrib(text.str());
  if (_subsume_alpha_test) {
    shattr = DCAST(ShaderAttrib, shattr)->set_flag(ShaderAttrib::F_subsume_alpha_test, true);
  }
  if (_disable_alpha_write) {
    shattr = DCAST(ShaderAttrib, shattr)->set_flag(ShaderAttrib::F_disable_alpha_write, true);
  }
  clear_analysis();
  reset_register_allocator();
  return DCAST(ShaderAttrib, shattr);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::combine_mode_as_string
//       Access: Protected, Static
//  Description: This 'synthesizes' a combine mode into a string.
////////////////////////////////////////////////////////////////////
const string ShaderGenerator::
combine_mode_as_string(CPT(TextureStage) stage, TextureStage::CombineMode c_mode, bool alpha, short texindex) {
  ostringstream text;
  switch (c_mode) {
    case TextureStage::CM_modulate:
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      text << " * ";
      text << combine_source_as_string(stage, 1, alpha, alpha, texindex);
      break;
    case TextureStage::CM_add:
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      text << " + ";
      text << combine_source_as_string(stage, 1, alpha, alpha, texindex);
      break;
    case TextureStage::CM_add_signed:
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      text << " + ";
      text << combine_source_as_string(stage, 1, alpha, alpha, texindex);
      if (alpha) {
        text << " - 0.5";
      } else {
        text << " - float3(0.5, 0.5, 0.5)";
      }
      break;
    case TextureStage::CM_interpolate:
      text << "lerp(";
      text << combine_source_as_string(stage, 1, alpha, alpha, texindex);
      text << ", ";
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      text << ", ";
      text << combine_source_as_string(stage, 2, alpha, true, texindex);
      text << ")";
      break;
    case TextureStage::CM_subtract:
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      text << " + ";
      text << combine_source_as_string(stage, 1, alpha, alpha, texindex);
      break;
    case TextureStage::CM_dot3_rgb:
      pgraph_cat.error() << "TextureStage::CombineMode DOT3_RGB not yet supported in per-pixel mode.\n";
      break;
    case TextureStage::CM_dot3_rgba:
      pgraph_cat.error() << "TextureStage::CombineMode DOT3_RGBA not yet supported in per-pixel mode.\n";
      break;
    case TextureStage::CM_replace:
    default: // Not sure if this is correct as default value.
      text << combine_source_as_string(stage, 0, alpha, alpha, texindex);
      break;
  }
  return text.str();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::combine_source_as_string
//       Access: Protected, Static
//  Description: This 'synthesizes' a combine source into a string.
////////////////////////////////////////////////////////////////////
const string ShaderGenerator::
combine_source_as_string(CPT(TextureStage) stage, short num, bool alpha, bool single_value, short texindex) {
  TextureStage::CombineSource c_src = TextureStage::CS_undefined;
  TextureStage::CombineOperand c_op = TextureStage::CO_undefined;
  if (alpha) {
    switch (num) {
      case 0:
        c_src = stage->get_combine_alpha_source0();
        c_op = stage->get_combine_alpha_operand0();
        break;
      case 1:
        c_src = stage->get_combine_alpha_source1();
        c_op = stage->get_combine_alpha_operand1();
        break;
      case 2:
        c_src = stage->get_combine_alpha_source2();
        c_op = stage->get_combine_alpha_operand2();
        break;
    }
  } else {
    switch (num) {
      case 0:
        c_src = stage->get_combine_rgb_source0();
        c_op = stage->get_combine_rgb_operand0();
        break;
      case 1:
        c_src = stage->get_combine_rgb_source1();
        c_op = stage->get_combine_rgb_operand1();
        break;
      case 2:
        c_src = stage->get_combine_rgb_source2();
        c_op = stage->get_combine_rgb_operand2();
        break;
    }
  }
  ostringstream csource;
  if (c_op == TextureStage::CO_one_minus_src_color ||
      c_op == TextureStage::CO_one_minus_src_alpha) {
    csource << "1.0f - ";
  }
  switch (c_src) {
    case TextureStage::CS_texture:
      csource << "tex" << texindex;
      break;
    case TextureStage::CS_constant: {
      LVecBase4 c = stage->get_color();
      csource << "float4(" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << ")";
      break; }
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
    case TextureStage::CS_undefined:
      break;
  }
  if (c_op == TextureStage::CO_src_color || c_op == TextureStage::CO_one_minus_src_color) {
    if (single_value) {
      // Let's take the red channel.
      csource << ".r";
    } else {
      csource << ".rgb";
    }
  } else {
    csource << ".a";
    if (!single_value) {
      // Dunno if it's legal in the FPP at all, but let's just allow it.
      return "float3(" + csource.str() + ")";
    }
  }
  return csource.str();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::texture_type_as_string
//       Access: Protected, Static
//  Description: Returns 1D, 2D, 3D or CUBE, depending on the given
//               texture type.
////////////////////////////////////////////////////////////////////
const string ShaderGenerator::
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
      pgraph_cat.error() << "Unsupported texture type!\n";
      return "2D";
  }
}

#endif  // HAVE_CG
