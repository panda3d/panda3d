/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaShader.cxx
 * @author drose
 * @date 2000-02-01
 * Modified 19Mar10 by ETC PandaSE team (see
 *   header comment for mayaToEgg.cxx for details)
 */

#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_maya.h"
#include "string_utils.h"
#include "pnmImageHeader.h"  // for lumin_red, etc.
#include "pset.h"

#include "pre_maya_include.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MColor.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include "post_maya_include.h"

using std::endl;
using std::string;

/**
 * Reads the Maya "shading engine" to determine the relevant shader
 * properties.
 */
MayaShader::
MayaShader(MObject engine, bool legacy_shader) {
  MFnDependencyNode engine_fn(engine);

  set_name(engine_fn.name().asChar());

  if (maya_cat.is_debug()) {
    maya_cat.debug()
      << "Reading shading engine " << get_name() << "\n";
  }
  _legacy_mode = false;
  _flat_color.set(1,1,1,1);

  MPlug shader_plug = engine_fn.findPlug("surfaceShader");
  bool found_shader = false;
  if (!shader_plug.isNull()) {
    MPlugArray shader_pa;
    shader_plug.connectedTo(shader_pa, true, false);
    maya_cat.spam() << "shader plug connected to: " << shader_pa.length() << endl;
    for (size_t i = 0; i < shader_pa.length() && !found_shader; i++) {
      MObject shader = shader_pa[0].node();
      if (shader.hasFn(MFn::kPhong)) {
        if (legacy_shader) {
          found_shader = find_textures_legacy(shader);
        } else {
          found_shader = find_textures_modern(shader);
        }
      } else if (shader.hasFn(MFn::kLambert)) {
        found_shader = find_textures_legacy(shader);
        if (found_shader) {
          _legacy_mode = true;
        }
      } else if (shader.hasFn(MFn::kSurfaceShader)) {
        found_shader = find_textures_legacy(shader);
        if (found_shader) {
          _legacy_mode = true;
        }
      } else {
        maya_cat.warning() <<
          "Unrecognized shader type: only lambert and phong supported (lambert deprecated).\n";
      }
    }
  }
}

/**
 *
 */
MayaShader::
~MayaShader() {
}

/**
 *
 */
void MayaShader::
output(std::ostream &out) const {
  out << "Shader " << get_name();
}

/**
 *
 */
void MayaShader::
write(std::ostream &out) const {
  out << "Shader " << get_name() << "\n";
}

/**
 * This is part of the deprecated codepath.  return the color def i.e.
 * texture at idx
 */
MayaShaderColorDef *MayaShader::
get_color_def(size_t idx) const {
  if (_color.size() > 0)
    return _color[idx];
  else
    return nullptr;
}
/**
 * Returns the overall color of the shader as a single-precision rgba value,
 * where the alpha component represents transparency according to the Panda
 * convention.  If no overall color is specified (_has_flat_color is not
 * true), this returns white.
 *
 * Normally, Maya makes texture color override the flat color, so if a texture
 * is also applied (_has_texture is true), this value is not used by Maya.
 */
LColor MayaShader::
get_rgba(size_t idx) const {
  LColor rgba(1.0f, 1.0f, 1.0f, 1.0f);

  if (_color.size() && _color[idx]->_has_flat_color) {
    rgba[0] = (PN_stdfloat)_color[idx]->_flat_color[0];
    rgba[1] = (PN_stdfloat)_color[idx]->_flat_color[1];
    rgba[2] = (PN_stdfloat)_color[idx]->_flat_color[2];
  }

  if (_transparency._has_flat_color) {
    // Maya supports colored transparency, but we only support grayscale
    // transparency.  Use the pnmimage constants to convert color to
    // grayscale.
    double trans =
      _transparency._flat_color[0] * lumin_red +
      _transparency._flat_color[1] * lumin_grn +
      _transparency._flat_color[2] * lumin_blu;
    rgba[3] = 1.0f - (PN_stdfloat)trans;
  }

  return rgba;
}

/**
 * Recalculates the all_maps list.
 */
void MayaShader::
collect_maps() {
  _all_maps.clear();

  for (size_t i=0; i<_color_maps.size(); i++) {
    _all_maps.push_back(_color_maps[i]);
  }
  for (size_t i=0; i<_trans_maps.size(); i++) {
    _all_maps.push_back(_trans_maps[i]);
  }
  for (size_t i=0; i<_normal_maps.size(); i++) {
    _all_maps.push_back(_normal_maps[i]);
  }
  for (size_t i=0; i<_glow_maps.size(); i++) {
    _all_maps.push_back(_glow_maps[i]);
  }
  for (size_t i=0; i<_gloss_maps.size(); i++) {
    _all_maps.push_back(_gloss_maps[i]);
  }
  for (size_t i=0; i<_height_maps.size(); i++) {
    _all_maps.push_back(_height_maps[i]);
  }

  for (size_t i=0; i<_color.size(); i++) {
    if (_color[i]->_has_texture) {
      _all_maps.push_back(_color[i]);
    }
  }
  if (_transparency._has_texture) {
    _all_maps.push_back(&_transparency);
  }
}

/**
 * Locates all file textures leading into the given shader.
 */
bool MayaShader::
find_textures_modern(MObject shader) {
  if (!shader.hasFn(MFn::kPhong)) {
    maya_cat.warning()
      << "The new codepath expects to see phong shaders only.\n";
    return false;
  }
  MStatus status;
  MFnPhongShader phong_fn(shader);
  MFnDependencyNode shader_fn(shader);

  if (maya_cat.is_spam()) {
    maya_cat.spam()
      << "  Reading modern surface shader " << shader_fn.name().asChar() << "\n";
  }

  string n = shader_fn.name().asChar();

  MayaShaderColorDef::find_textures_modern(n, _color_maps,  shader_fn.findPlug("color"), false);
  if (_color_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _color_maps,  shader_fn.findPlug("colorR"), false);
  }
  MayaShaderColorDef::find_textures_modern(n, _trans_maps,  shader_fn.findPlug("transparency"), true);
  if (_trans_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _trans_maps,  shader_fn.findPlug("transparencyR"), true);
  }
  MayaShaderColorDef::find_textures_modern(n, _normal_maps, shader_fn.findPlug("normalCamera"), false);
  if (_normal_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _normal_maps, shader_fn.findPlug("normalCameraR"), false);
  }
  MayaShaderColorDef::find_textures_modern(n, _gloss_maps,  shader_fn.findPlug("specularColor"), true);
  if (_gloss_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _gloss_maps,  shader_fn.findPlug("specularColorR"), true);
  }
  MayaShaderColorDef::find_textures_modern(n, _glow_maps,  shader_fn.findPlug("incandescence"), true);
  if (_glow_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _glow_maps,  shader_fn.findPlug("incandescenceR"), true);
  }
  MayaShaderColorDef::find_textures_modern(n, _height_maps,  shader_fn.findPlug("surfaceThickness"), true);
  if (_height_maps.size() == 0) {
    MayaShaderColorDef::find_textures_modern(n, _height_maps,  shader_fn.findPlug("surfaceThicknessR"), true);
  }

  collect_maps();

  MColor color = phong_fn.color(&status);
  if (status) {
    _flat_color.set(color.r, color.g, color.b, color.a);
  }

  color = phong_fn.transparency(&status);
  if (status) {
    _flat_color[3] = 1.0 - ((color[0] + color[1] + color[2]) * (1.0/3.0));
  }
  return true;
}

/**
 * Assigns the uvset_name of each MayaShaderColorDef using the given file-to-
 * uvset map.
 */
void MayaShader::
bind_uvsets(MayaFileToUVSetMap &map) {
  for (size_t i=0; i<_all_maps.size(); i++) {
    MayaShaderColorDef *def = _all_maps[i];
    MayaFileToUVSetMap::iterator p = map.find(def->_texture_name);
    if (p == map.end()) {
      def->_uvset_name = "map1";
    } else {
      def->_uvset_name = (*p).second;
    }
  }

  calculate_pairings();
}

/**
 * For each Alpha texture, try to find an RGB texture that has the same
 * properties.  Attempt to make it so that the alpha texture isn't a separate
 * texture, but rather, an Alpha-Filename associated with an existing texture.
 */
void MayaShader::
calculate_pairings() {

  if (_legacy_mode) {
    return;
  }

  for (size_t i=0; i<_all_maps.size(); i++) {
    _all_maps[i]->_opposite = 0;
  }

  bool using_transparency = (_trans_maps.size() > 0);

  for (int retry=0; retry<2; retry++) {
    bool perfect=(retry==0);
    for (size_t i=0; i<_color_maps.size(); i++) {
      if ((_color_maps[i]->_blend_type == MayaShaderColorDef::BT_modulate)||
          (_color_maps[i]->_blend_type == MayaShaderColorDef::BT_unspecified)) {
        for (size_t j=0; j<_trans_maps.size(); j++) {
          try_pair(_color_maps[i], _trans_maps[j], perfect);
        }
      }
    }
  }

  if (!using_transparency) {
    for (int retry=0; retry<2; retry++) {
      bool perfect=(retry==0);
      for (size_t i=0; i<_color_maps.size(); i++) {
        for (size_t j=0; j<_glow_maps.size(); j++) {
          try_pair(_color_maps[i], _glow_maps[j], perfect);
        }
        for (size_t j=0; j<_gloss_maps.size(); j++) {
          try_pair(_color_maps[i], _gloss_maps[j], perfect);
        }
      }
    }
  }

  for (int retry=0; retry<2; retry++) {
    bool perfect=(retry==0);
    for (size_t i=0; i<_normal_maps.size(); i++) {
      for (size_t j=0; j<_height_maps.size(); j++) {
        try_pair(_normal_maps[i], _height_maps[j], perfect);
      }
    }
  }

  for (size_t i=0; i<_normal_maps.size(); i++) {
    _normal_maps[i]->_blend_type = MayaShaderColorDef::BT_normal;
  }
  for (size_t i=0; i<_glow_maps.size(); i++) {
    if (_glow_maps[i]->_opposite) {
      _glow_maps[i]->_blend_type = MayaShaderColorDef::BT_unspecified;
      _glow_maps[i]->_opposite->_blend_type = MayaShaderColorDef::BT_modulate_glow;
    } else {
      _glow_maps[i]->_blend_type = MayaShaderColorDef::BT_glow;
    }
  }
  for (size_t i=0; i<_gloss_maps.size(); i++) {
    if (_gloss_maps[i]->_opposite) {
      _gloss_maps[i]->_blend_type = MayaShaderColorDef::BT_unspecified;
      _gloss_maps[i]->_opposite->_blend_type = MayaShaderColorDef::BT_modulate_gloss;
    } else {
      _gloss_maps[i]->_blend_type = MayaShaderColorDef::BT_gloss;
    }
  }
  for (size_t i=0; i<_height_maps.size(); i++) {
    if (_height_maps[i]->_opposite) {
      _height_maps[i]->_blend_type = MayaShaderColorDef::BT_unspecified;
      _height_maps[i]->_opposite->_blend_type = MayaShaderColorDef::BT_normal_height;
    } else {
      _height_maps[i]->_blend_type = MayaShaderColorDef::BT_height;
    }
  }
  for (size_t i=0; i<_trans_maps.size(); i++) {
    if (_trans_maps[i]->_opposite) {
      _trans_maps[i]->_blend_type = MayaShaderColorDef::BT_unspecified;
      _trans_maps[i]->_opposite->_blend_type = MayaShaderColorDef::BT_modulate;
    } else {
      _trans_maps[i]->_blend_type = MayaShaderColorDef::BT_modulate;
    }
  }
}

/**
 * Try to associate an RGB tex with an Alpha tex.
 */
bool MayaShader::try_pair(MayaShaderColorDef *map1,
                          MayaShaderColorDef *map2,
                          bool perfect) {
  if ((map1->_opposite)||(map2->_opposite)) {
    // one of the maps is already paired
    return false;
  }
  if (perfect) {
    if (map1->_texture_filename != map2->_texture_filename) {
      // perfect mode requires a filename match.
      return false;
    }
  } else {
    string pre1 = get_file_prefix(map1->_texture_filename);
    string pre2 = get_file_prefix(map2->_texture_filename);
    if (pre1 != pre2) {
      // imperfect mode requires a filename prefix match.
      return false;
    }
  }

  if ((map1->_projection_type   != map2->_projection_type) ||
      (map1->_projection_matrix != map2->_projection_matrix) ||
      (map1->_u_angle           != map2->_u_angle) ||
      (map1->_v_angle           != map2->_v_angle) ||
      (map1->_uvset_name        != map2->_uvset_name) ||
      (map1->_mirror            != map2->_mirror) ||
      (map1->_stagger           != map2->_stagger) ||
      (map1->_wrap_u            != map2->_wrap_u) ||
      (map1->_wrap_v            != map2->_wrap_v) ||
      (map1->_repeat_uv         != map2->_repeat_uv) ||
      (map1->_offset            != map2->_offset) ||
      (map1->_rotate_uv         != map2->_rotate_uv)) {
    return false;
  }
  // Pairing successful.
  map1->_opposite = map2;
  map2->_opposite = map1;
  return true;
}

/**
 * Try to associate an RGB tex with an Alpha tex.
 */
string MayaShader::
get_file_prefix(const string &fn) {
  Filename pfn = Filename::from_os_specific(fn);
  string base = pfn.get_basename_wo_extension();
  size_t offs = base.find("_");
  if (offs != string::npos) {
    base = base.substr(0, offs);
  }
  offs = base.find("-");
  if (offs != string::npos) {
    base = base.substr(0, offs);
  }
  return base;
}

/**
 * This is part of the legacy codepath.  Extracts out the shading information
 * from the Maya surface shader.
 */
bool MayaShader::
find_textures_legacy(MObject shader) {
  MStatus status;
  MFnDependencyNode shader_fn(shader);

  if (maya_cat.is_spam()) {
    maya_cat.spam()
      << "  Reading legacy surface shader " << shader_fn.name().asChar() << "\n";
  }

  // First, check for a connection to the color attribute.  This could be a
  // texture map or something, and will override whatever the shader says for
  // color.

  MPlug color_plug = shader_fn.findPlug("color");
  if (color_plug.isNull()) {
    // Or maybe a connection to outColor.  Not sure how this differs from just
    // color, but empirically it seems that either might be used.
    color_plug = shader_fn.findPlug("outColor");
  }

  if (!color_plug.isNull()) {
    MPlugArray color_pa;
    color_plug.connectedTo(color_pa, true, false);

    MayaShaderColorDef *color_p = new MayaShaderColorDef;
    for (size_t i = 0; i < color_pa.length(); i++) {
      maya_cat.spam() << "color_pa[" << i << "]:" << color_pa[i].name().asChar() << endl;
      color_p->find_textures_legacy(this, color_pa[0].node());
    }

    if (color_pa.length() < 1) {
      // assign a blank default color to this shader
      maya_cat.spam() << shader_fn.name().asChar() << " was not connected to texture" << endl;
      this->_color.push_back(color_p);
    }
  }

  // Transparency is stored separately.
  MPlug trans_plug = shader_fn.findPlug("transparency");
  if (trans_plug.isNull()) {
    trans_plug = shader_fn.findPlug("outTransparency");
  }

  if (!trans_plug.isNull()) {
    MPlugArray trans_pa;
    trans_plug.connectedTo(trans_pa, true, false);

    for (size_t i = 0; i < trans_pa.length(); i++) {
      maya_cat.spam() << "read a transparency texture" << endl;
      _transparency.find_textures_legacy(this, trans_pa[0].node(), true);
    }
  }

  // Also try to get the ordinary color directly from the surface shader.
  bool b_color_def = true;
  if (shader.hasFn(MFn::kLambert)) {
    MFnLambertShader lambert_fn(shader);
    MColor color = lambert_fn.color(&status);
    if (status) {
      // Warning!  The alpha component of color doesn't mean transparency in
      // Maya.
      for (size_t i=0; i<_color.size(); ++i) {
        _color[i]->_has_flat_color = true;
        _color[i]->_flat_color.set(color.r, color.g, color.b, color.a);
        maya_cat.spam() << shader_fn.name().asChar() << " set shader color" << endl;
        // needed to print the final check
        if (!_color[i]->_has_flat_color && !_color[i]->_has_texture)
          b_color_def = false;

        _transparency._flat_color.set(0.0, 0.0, 0.0, 0.0);

        // Get the transparency separately.
        color = lambert_fn.transparency(&status);
        if (status) {
          _transparency._has_flat_color = true;
          _transparency._flat_color.set(color.r, color.g, color.b, color.a);
        }
      }
    }
  }
  // if (!_color._has_flat_color && !_color._has_texture) {
  if (!b_color_def) {
    maya_cat.info() << shader_fn.name().asChar() << "Color def not found" << endl;
    if (maya_cat.is_spam()) {
      maya_cat.spam()
        << "  Color definition not found.\n";
    }
  }

  collect_maps();
  return true;
}
