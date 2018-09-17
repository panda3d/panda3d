/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaShaderColorDef.cxx
 * @author drose
 * @date 2003-04-12
 * Modified 19Mar10 by ETC PandaSE team (see
 *   header comment for mayaToEgg.cxx for details)
 */

#include "mayaShaderColorDef.h"
#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_maya.h"
#include "string_utils.h"
#include "pset.h"

#include "pre_maya_include.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnEnumAttribute.h>
#include "post_maya_include.h"

using std::endl;
using std::string;

/**
 *
 */
MayaShaderColorDef::
MayaShaderColorDef() {

  _blend_type = BT_unspecified;

  _projection_type = PT_off;
  _projection_matrix = LMatrix4d::ident_mat();
  _u_angle = 0.0;
  _v_angle = 0.0;

  _texture_filename = "";
  _texture_name = "";
  _color_gain.set(1.0f, 1.0f, 1.0f, 1.0f);

  _coverage.set(1.0, 1.0);
  _translate_frame.set(0.0, 0.0);
  _rotate_frame = 0.0;

  _mirror = false;
  _stagger = false;
  _wrap_u = true;
  _wrap_v = true;

  _repeat_uv.set(1.0, 1.0);
  _offset.set(0.0, 0.0);
  _rotate_uv = 0.0;

  _is_alpha = false;

  _opposite = 0;

  _color_object = nullptr;

  _has_texture = false;
  _has_flat_color = false;
  _flat_color.set(0.0, 0.0, 0.0, 0.0);
  _has_alpha_channel = false;
  _keep_color = false; // classic mode overwrites color: new mode retains color with a 3rd layer
  _keep_alpha = false;
  _interpolate = false;
  _uvset_name = "map1";

  _map_uvs = nullptr;
}

/**
 *
 */
MayaShaderColorDef::
MayaShaderColorDef(MayaShaderColorDef &copy) {
  _has_texture = copy._has_texture;
  _texture_filename = copy._texture_filename;
  _texture_name = copy._texture_name;
  _uvset_name = copy._uvset_name;
  _color_gain = copy._color_gain;

  _has_flat_color = copy._has_flat_color;
  _flat_color = copy._flat_color;

  _projection_type = copy._projection_type;
  _projection_matrix = copy._projection_matrix;
  _u_angle = copy._u_angle;
  _v_angle = copy._v_angle;

  _coverage = copy._coverage;
  _translate_frame = copy._translate_frame;
  _rotate_frame = copy._rotate_frame;

  _mirror = copy._mirror;
  _stagger = copy._stagger;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;

  _blend_type = copy._blend_type;
  _has_alpha_channel = copy._has_alpha_channel;
  _keep_color = copy._keep_color;
  _keep_alpha = copy._keep_alpha;
  _interpolate = copy._interpolate;

  _repeat_uv = copy._repeat_uv;
  _offset = copy._offset;
  _rotate_uv = copy._rotate_uv;

  _is_alpha = copy._is_alpha;

  _map_uvs = copy._map_uvs;
  _color_object = copy._color_object;

  _opposite = 0;
}

/**
 *
 */
MayaShaderColorDef::
~MayaShaderColorDef() {
  if (_color_object != nullptr) {
    delete _color_object;
  }
}

/**
 * Returns a texture matrix corresponding to the texture transforms indicated
 * by the shader.
 */
LMatrix3d MayaShaderColorDef::
compute_texture_matrix() const {
  LVector2d scale(_repeat_uv[0] / _coverage[0],
                  _repeat_uv[1] / _coverage[1]);
  LVector2d trans(_offset[0] - _translate_frame[0] / _coverage[0],
                  _offset[1] - _translate_frame[1] / _coverage[1]);

  return
    (LMatrix3d::translate_mat(LVector2d(-0.5, -0.5)) *
     LMatrix3d::rotate_mat(_rotate_frame) *
     LMatrix3d::translate_mat(LVector2d(0.5, 0.5))) *
    LMatrix3d::scale_mat(scale) *
    LMatrix3d::translate_mat(trans);
}

/**
 * Returns true if the shader has a projection in effect.
 */
bool MayaShaderColorDef::
has_projection() const {
  return (_projection_type != PT_off);
}

/**
 * If the shader has a projection (has_projection() returns true), this
 * computes the appropriate UV corresponding to the indicated 3-d point.
 * Seams that might be introduced on polygons that cross quadrants are closed
 * up by ensuring the point is in the same quadrant as the indicated reference
 * point.
 */
LTexCoordd MayaShaderColorDef::
project_uv(const LPoint3d &pos, const LPoint3d &centroid) const {
  nassertr(_map_uvs != nullptr, LTexCoordd::zero());
  return (this->*_map_uvs)(pos * _projection_matrix, centroid * _projection_matrix);
}

/**
 *
 */
void MayaShaderColorDef::
write(std::ostream &out) const {
  if (_has_texture) {
    out << "    texture filename is " << _texture_filename << "\n"
        << "    texture name is " << _texture_name << "\n"
        << "    uv_set name is " << _uvset_name << "\n"
        << "    coverage is " << _coverage << "\n"
        << "    translate_frame is " << _translate_frame << "\n"
        << "    rotate_frame is " << _rotate_frame << "\n"
        << "    mirror is " << _mirror << "\n"
        << "    stagger is " << _stagger << "\n"
        << "    wrap_u is " << _wrap_u << "\n"
        << "    wrap_v is " << _wrap_v << "\n"
        << "    repeat_uv is " << _repeat_uv << "\n"
        << "    offset is " << _offset << "\n"
        << "    rotate_uv is " << _rotate_uv << "\n"
        << "    color_gain is " << _color_gain << "\n";

  } else if (_has_flat_color) {
    out << "    flat color is " << _flat_color << "\n";
  }
}

/**
 * Changes the texture filename stored in the Maya file for this particular
 * shader.
 */
bool MayaShaderColorDef::
reset_maya_texture(const Filename &texture) {
  if (_color_object != nullptr) {
    _has_texture = set_string_attribute(*_color_object, "fileTextureName",
                                        texture.to_os_generic());
    _texture_filename = texture;

    if (!_has_texture) {
      maya_cat.error()
        << "Unable to reset texture filename.\n";
    }

    return _has_texture;
  }

  maya_cat.error()
    << "Attempt to reset texture on Maya object that has no color set.\n";
  return false;
}


/**
 * Maya's default uvset name is "map1".  Panda's default uvset name is
 * "default".  Otherwise, leaves uvset name untranslated.
 */
string MayaShaderColorDef::
get_panda_uvset_name() {
  if (_uvset_name == "map1") {
    return "default";
  }
  return _uvset_name;
}

/**
 * This is part of the deprecated codepath.  Determines the surface color
 * specified by the shader.  This includes texturing and other advanced shader
 * properties.
 */
void MayaShaderColorDef::
find_textures_legacy(MayaShader *shader, MObject color, bool trans) {
  LRGBColor color_gain;
  if (get_vec3_attribute(color, "colorGain", color_gain)) {
    color_gain[0] = color_gain[0] > 1.0 ? 1.0 : color_gain[0];
    color_gain[0] = color_gain[0] < 0.0 ? 0.0 : color_gain[0];
    _color_gain[0] *= color_gain[0];
    color_gain[1] = color_gain[1] > 1.0 ? 1.0 : color_gain[1];
    color_gain[1] = color_gain[1] < 0.0 ? 0.0 : color_gain[1];
    _color_gain[1] *= color_gain[1];
    color_gain[2] = color_gain[2] > 1.0 ? 1.0 : color_gain[2];
    color_gain[2] = color_gain[2] < 0.0 ? 0.0 : color_gain[2];
    _color_gain[2] *= color_gain[2];
  }
  PN_stdfloat alpha_gain;
  if (get_maya_attribute(color, "alphaGain", alpha_gain)) {
    alpha_gain = alpha_gain > 1.0 ? 1.0 : alpha_gain;
    alpha_gain = alpha_gain < 0.0 ? 0.0 : alpha_gain;
    _color_gain[3] *= alpha_gain;
  }
  if (color.hasFn(MFn::kFileTexture)) {
    MFnDependencyNode dfn(color);
    _color_object = new MObject(color);
    _texture_name = dfn.name().asChar();
    string filename;
    _has_texture = get_string_attribute(color, "fileTextureName", filename);
    _has_texture = _has_texture && !filename.empty();
    if (_has_texture) {
      _texture_filename = Filename::from_os_specific(filename);
      if (_texture_filename.is_directory()) {
        maya_cat.warning()
          << "Shader " << shader->get_name()
          << " references texture filename " << filename
          << " which is a directory; clearing.\n";
        _has_texture = false;
        set_string_attribute(color, "fileTextureName", "");
      }
    }

    get_vec2_attribute(color, "coverage", _coverage);
    get_vec2_attribute(color, "translateFrame", _translate_frame);
    get_angle_attribute(color, "rotateFrame", _rotate_frame);

    // get_bool_attribute(color, "alphaIsLuminance", _alpha_is_luminance);

    get_bool_attribute(color, "mirror", _mirror);
    get_bool_attribute(color, "stagger", _stagger);
    get_bool_attribute(color, "wrapU", _wrap_u);
    get_bool_attribute(color, "wrapV", _wrap_v);

    get_vec2_attribute(color, "repeatUV", _repeat_uv);
    get_vec2_attribute(color, "offset", _offset);
    get_angle_attribute(color, "rotateUV", _rotate_uv);

    if (!trans) {
      if (maya_cat.is_debug()) {
        maya_cat.debug() << "pushed a file texture" << endl;
      }
      shader->_color.push_back(this);
    }

  } else if (color.hasFn(MFn::kProjection)) {
    if (maya_cat.is_debug()) {
      maya_cat.debug() << "reading a projection texture" << endl;
    }
    // This is a projected texture.  We will have to step one level deeper to
    // find the actual texture.
    MFnDependencyNode projection_fn(color);
    MPlug image_plug = projection_fn.findPlug("image");
    if (!image_plug.isNull()) {
      MPlugArray image_pa;
      image_plug.connectedTo(image_pa, true, false);

      for (size_t i = 0; i < image_pa.length(); i++) {
        find_textures_legacy(shader, image_pa[0].node());
      }
    }

    if (!get_mat4d_attribute(color, "placementMatrix", _projection_matrix)) {
      _projection_matrix = LMatrix4d::ident_mat();
    }

    // The uAngle and vAngle might be used for certain kinds of projections.
    if (!get_angle_attribute(color, "uAngle", _u_angle)) {
      _u_angle = 360.0;
    }
    if (!get_angle_attribute(color, "vAngle", _v_angle)) {
      _v_angle = 180.0;
    }

    string type;
    if (get_enum_attribute(color, "projType", type)) {
      set_projection_type(type);
    }

  } else if (color.hasFn(MFn::kLayeredTexture)) {
    if (maya_cat.is_debug()) {
      maya_cat.debug() << "Found layered texture" << endl;
    }

    int blendValue;
    MStatus status;
    MPlugArray color_pa;
    MFnDependencyNode layered_fn(color);
    layered_fn.getConnections(color_pa);
    MPlug inputsPlug = layered_fn.findPlug("inputs", &status);
    MPlug blendModePlug = layered_fn.findPlug("blendMode", &status);

    if (maya_cat.is_debug()) {
      maya_cat.debug() << "number of connections: " << color_pa.length() << endl;
    }
    bool first = true;
    BlendType bt = BT_modulate;
    for (size_t i=0; i<color_pa.length(); ++i) {
      MPlug pl = color_pa[i];
      MPlugArray pla;
      pl.connectedTo(pla, true, false);

      // First figure out the blend mode intended for this shadercolordef
      int li = pl.logicalIndex();
      if (li > -1) {
        // found a blend mode
        if (maya_cat.is_spam()) {
          MString name = inputsPlug.name();
          maya_cat.spam() << "*** Start doIt... ***" << endl;
          maya_cat.spam() << "inputsPlug Name: " << name.asChar() << endl;
        }
        status = blendModePlug.selectAncestorLogicalIndex(li,inputsPlug);
        blendModePlug.getValue(blendValue);

        if (maya_cat.is_spam()) {
          MString name = blendModePlug.name();
          maya_cat.spam()
            << name.asChar() << ": has value " << blendValue << endl;
        }

        MFnEnumAttribute blendModeEnum(blendModePlug);
        MString blendName = blendModeEnum.fieldName(blendValue, &status);

        switch (blendValue) {
        case 1:
          bt = BT_decal;
          get_bool_attribute(color, "interpolate", _interpolate);
          maya_cat.info() << "interpolate: " << _interpolate << endl;
          _keep_color = true;
          break;
        case 6:
          bt = BT_modulate;
          get_bool_attribute(color, "keepAlpha", _keep_alpha);
          maya_cat.info() << "keepAlpha: " << _keep_alpha << endl;
          break;
        case 4:
          bt = BT_add;
          break;
        }

        if (maya_cat.is_info()) {
          MString name = layered_fn.name();
          maya_cat.info() << name.asChar() << ": blendMode used " << blendName.asChar() << endl;
          if (maya_cat.is_spam()) {
            maya_cat.spam() << "*** END doIt... ***" << endl;
          }
        }

        // advance to the next plug, because that is where the shader info are
        pl = color_pa[++i];
        pl.connectedTo(pla, true, false);
      }
      for (size_t j=0; j<pla.length(); ++j) {
        // maya_cat.debug() << pl.name() << " is(pl) " <<
        // pl.node().apiTypeStr() << endl; maya_cat.debug() << pla[j].name()
        // << " is(pla) " << pla[j].node().apiTypeStr() << endl;
        string pla_name = pla[j].name().asChar();
        // sometimes, by default, maya gives a outAlpha on subsequent plugs,
        // ignore that
        if (pla_name.find("outAlpha") != string::npos) {
          // top texture has an alpha channel, so make sure that this alpha is
          // retained by egg file
          if (maya_cat.is_debug()) {
            maya_cat.debug() << pl.name().asChar() << ":has alpha channel" << pla_name << endl;
          }
          _has_alpha_channel = true;
          continue;
        }
        if (!first) {
          if (maya_cat.is_debug()) {
            maya_cat.debug() << pl.name().asChar() << " next:connectedTo: " << pla_name << endl;
          }
          MayaShaderColorDef *color_p = new MayaShaderColorDef;
          color_p->find_textures_legacy(shader, pla[j].node());
          color_p->_blend_type = bt;
          size_t loc = color_p->_texture_name.find('.',0);
          if (loc != string::npos) {
            color_p->_texture_name.resize(loc);
          }
          if (maya_cat.is_debug()) {
            maya_cat.debug() << "uv_name : " << color_p->_texture_name << endl;
          }
        }
        else {
          if (maya_cat.is_debug()) {
            maya_cat.debug() << pl.name().asChar() << " first:connectedTo: " << pla_name << endl;
          }
          find_textures_legacy(shader, pla[j].node());
          _texture_name.assign(pla[j].name().asChar());
          _blend_type = bt;
          size_t loc = _texture_name.find('.',0);
          if (loc != string::npos) {
            _texture_name.resize(loc);
          }
          if (maya_cat.is_debug()) {
            maya_cat.debug() << "uv_name : " << _texture_name << endl;
          }
          first = false;
        }
      }
    }
  } else {
    // This shader wasn't understood.
    if (maya_cat.is_debug()) {
      maya_cat.info()
        << "**Don't know how to interpret color attribute type "
        << color.apiTypeStr() << "\n";

    } else {
      // If we don't have a heavy verbose count, only report each type of
      // unsupported shader once.
      static pset<MFn::Type> bad_types;
      if (bad_types.insert(color.apiType()).second) {
        maya_cat.info()
          << "**Don't know how to interpret color attribute type "
          << color.apiTypeStr() << "\n";
      }
    }
  }
}

/**
 * Search to find any file textures that lead into the given input plug.  Any
 * textures found will be added to the provided MayaShaderColorList.
 */
void MayaShaderColorDef::
find_textures_modern(const string &shadername, MayaShaderColorList &list, MPlug inplug, bool is_alpha) {

  MPlugArray outplugs;
  inplug.connectedTo(outplugs, true, false);
  if (outplugs.length() == 0) {
    return;
  }
  if (outplugs.length() > 1) {
    // Only one output plug should be connected to a given input plug.
    maya_cat.warning()
      << "Shader " << shadername << " has weird plug connections.\n";
    return;
  }
  MPlug outplug = outplugs[0];
  MObject source = outplug.node();
  MFnDependencyNode sourceFn(source);

  if (source.hasFn(MFn::kFileTexture)) {

    string filename;
    bool hasfn = get_string_attribute(source, "fileTextureName", filename);
    if ((!hasfn) || (filename.empty())) {
      maya_cat.warning()
        << "Shader " << shadername << " references file texture "
        << "with no file name, ignoring invalid file texture.\n";
      return;
    }
    Filename fn = filename;
    if (fn.is_directory()) {
      maya_cat.warning()
        << "Shader " << shadername << " references file name "
        << filename << " which is a directory, ignoring it.\n";
      return;
    }

    MayaShaderColorDef *def = new MayaShaderColorDef;

    def->_color_object = new MObject(source);
    def->_texture_filename = Filename::from_os_specific(filename);
    def->_texture_name = sourceFn.name().asChar();

    get_vec2_attribute(source, "coverage",       def->_coverage);
    get_vec2_attribute(source, "translateFrame", def->_translate_frame);
    get_angle_attribute(source, "rotateFrame",    def->_rotate_frame);

    get_bool_attribute(source, "mirror",          def->_mirror);
    get_bool_attribute(source, "stagger",         def->_stagger);
    get_bool_attribute(source, "wrapU",           def->_wrap_u);
    get_bool_attribute(source, "wrapV",           def->_wrap_v);

    get_vec2_attribute(source, "repeatUV",       def->_repeat_uv);
    get_vec2_attribute(source, "offset",         def->_offset);
    get_angle_attribute(source, "rotateUV",       def->_rotate_uv);

    LRGBColor color_gain;
    PN_stdfloat alpha_gain;
    get_vec3_attribute(source, "colorGain",      color_gain);
    get_maya_attribute(source, "alphaGain",       alpha_gain);
    def->_color_gain[0] = color_gain[0];
    def->_color_gain[1] = color_gain[1];
    def->_color_gain[2] = color_gain[2];
    def->_color_gain[3] = alpha_gain;

    def->_is_alpha = is_alpha;

    if (maya_cat.is_debug()) {
      maya_cat.debug() << "pushed a file texture" << endl;
    }
    list.push_back(def);

    return;
  }

  if (source.hasFn(MFn::kProjection)) {
    // This is a projected texture.  We will have to step one level deeper to
    // find the actual texture.
    size_t before = list.size();
    MPlug image_plug = sourceFn.findPlug("image");
    if (!image_plug.isNull()) {
      MPlugArray image_pa;
      image_plug.connectedTo(image_pa, true, false);

      for (size_t i = 0; i < image_pa.length(); i++) {
        find_textures_modern(shadername, list, image_pa[0], is_alpha);
      }
    }

    // Now apply any inherited attributes to all textures found.

    for (size_t i=before; i<list.size(); i++) {
      MayaShaderColorDef *def = list[i];

      if (!get_mat4d_attribute(source, "placementMatrix", def->_projection_matrix)) {
        def->_projection_matrix = LMatrix4d::ident_mat();
      }

      // The uAngle and vAngle might be used for certain kinds of projections.
      if (!get_angle_attribute(source, "uAngle", def->_u_angle)) {
        def->_u_angle = 360.0;
      }
      if (!get_angle_attribute(source, "vAngle", def->_v_angle)) {
        def->_v_angle = 180.0;
      }

      string type;
      if (get_enum_attribute(source, "projType", type)) {
        def->set_projection_type(type);
      }
    }
    return;
  }

  if (source.hasFn(MFn::kLayeredTexture)) {
    if (maya_cat.is_debug()) {
      maya_cat.debug() << "Found layered texture" << endl;
    }

    MPlug inputsPlug = sourceFn.findPlug("inputs");
    size_t nlayers = inputsPlug.numElements();
    for (size_t layer=0; layer<nlayers; layer++) {
      MPlug elt = inputsPlug.elementByPhysicalIndex(layer);
      MPlug color;
      MPlug blend;
      for (size_t j=0; j<elt.numChildren(); j++) {
        MPlug child = elt.child(j);
        MFnAttribute att(child.attribute());
        if (att.name() == "color") color = child;
        if (att.name() == "blendMode") blend = child;
      }
      if (color.isNull() || blend.isNull()) {
        maya_cat.warning() << "Invalid layered texture - bad inputs.\n";
        return;
      }
      size_t before = list.size();
      find_textures_modern(shadername, list, color, is_alpha);
      int blendValue;
      blend.getValue(blendValue);
      for (size_t sub=before; sub<list.size(); sub++) {
        MayaShaderColorDef *def = list[sub];
        switch (blendValue) {
        case 1:  def->_blend_type = BT_decal;     break;
        case 6:  def->_blend_type = BT_modulate;  break;
        case 4:  def->_blend_type = BT_add;       break;
        }
      }
    }
    return;
  }

  if (source.apiType() == MFn::kReverse) {
    MPlug input_plug = sourceFn.findPlug("input");
    find_textures_modern(shadername, list, input_plug, is_alpha);
    return;
  }

  // This shader wasn't understood.
  if (maya_cat.is_debug()) {
    maya_cat.info()
      << "**Don't know how to interpret color attribute type "
      << source.apiTypeStr() << "\n";
  } else {
    // If we don't have a heavy verbose count, only report each type of
    // unsupported shader once.
    static pset<MFn::Type> bad_types;
    if (bad_types.insert(source.apiType()).second) {
      maya_cat.warning()
        << "Don't know how to export a shader of type "
        << source.apiTypeStr() << " " << sourceFn.type() << "\n";
    }
  }
}

/**
 * Sets up the shader to apply UV's according to the indicated projection
 * type.
 */
void MayaShaderColorDef::
set_projection_type(const string &type) {
  if (cmp_nocase(type, "planar") == 0) {
    _projection_type = PT_planar;
    _map_uvs = &MayaShaderColorDef::map_planar;

    // The Planar projection normally projects to a range (-1, 1) in both
    // axes.  Scale this into our UV range of (0, 1).
    _projection_matrix = _projection_matrix * LMatrix4d(0.5, 0.0, 0.0, 0.0,
                                                        0.0, 0.5, 0.0, 0.0,
                                                        0.0, 0.0, 1.0, 0.0,
                                                        0.5, 0.5, 0.0, 1.0);

  } else if (cmp_nocase(type, "cylindrical") == 0) {
    _projection_type = PT_cylindrical;
    _map_uvs = &MayaShaderColorDef::map_cylindrical;

    // The cylindrical projection is orthographic in the Y axis; scale the
    // range (-1, 1) in this axis into our UV range (0, 1).
    _projection_matrix = _projection_matrix * LMatrix4d(1.0, 0.0, 0.0, 0.0,
                                                        0.0, 0.5, 0.0, 0.0,
                                                        0.0, 0.0, 1.0, 0.0,
                                                        0.0, 0.5, 0.0, 1.0);

  } else if (cmp_nocase(type, "spherical") == 0) {
    _projection_type = PT_spherical;
    _map_uvs = &MayaShaderColorDef::map_spherical;

  } else {
    // Other projection types are currently unimplemented by the converter.
    maya_cat.error()
      << "Don't know how to handle type " << type << " projections.\n";
    _projection_type = PT_off;
    _map_uvs = nullptr;
  }
}

/**
 * Computes a UV based on the given point in space, using a planar projection.
 */
LPoint2d MayaShaderColorDef::
map_planar(const LPoint3d &pos, const LPoint3d &) const {
  // A planar projection is about as easy as can be.  We ignore the Z axis,
  // and project the point into the XY plane.  Done.
  return LPoint2d(pos[0], pos[1]);
}

/**
 * Computes a UV based on the given point in space, using a spherical
 * projection.
 */
LPoint2d MayaShaderColorDef::
map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const {
  // To compute the x position on the frame, we only need to consider the
  // angle of the vector about the Y axis.  Project the vector into the XZ
  // plane to do this.

  LVector2d xz(pos[0], pos[2]);
  double xz_length = xz.length();

  if (xz_length < 0.01) {
    // If we have a point on or near either pole, we've got problems.  This
    // point maps to the entire bottom edge of the image, so which U value
    // should we choose?  It does make a difference, especially if we have a
    // number of polygons around the south pole that all share the common
    // vertex.

    // We choose the U value based on the polygon's centroid.
    xz.set(centroid[0], centroid[2]);
  }

  // Now, if the polygon crosses the seam, we also have problems.  Make sure
  // that the u value is in the same half of the texture as the centroid's u
  // value.
  double u = rad_2_deg(atan2(xz[0], xz[1])) / (2.0 * _u_angle);
  double c = rad_2_deg(atan2(centroid[0], centroid[2])) / (2.0 * _u_angle);

  if (u - c > 0.5) {
    u -= floor(u - c + 0.5);
  } else if (u - c < -0.5) {
    u += floor(c - u + 0.5);
  }

  // Now rotate the vector into the YZ plane, and the V value is based on the
  // latitude: the angle about the X axis.
  LVector2d yz(pos[1], xz_length);
  double v = rad_2_deg(atan2(yz[0], yz[1])) / (2.0 * _v_angle);

  LPoint2d uv(u - 0.5, v - 0.5);

  nassertr(fabs(u - c) <= 0.5, uv);
  return uv;
}

/**
 * Computes a UV based on the given point in space, using a cylindrical
 * projection.
 */
LPoint2d MayaShaderColorDef::
map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const {
  // This is almost identical to the spherical projection, except for the
  // computation of V.

  LVector2d xz(pos[0], pos[2]);
  double xz_length = xz.length();

  if (xz_length < 0.01) {
/*
 * A cylindrical mapping has the same singularity problem at the pole as a
 * spherical mapping does: points at the pole do not map to a single point on
 * the texture.  (It's technically a slightly different problem: in a
 * cylindrical mapping, points at the pole do not map to any point on the
 * texture, while in a spherical mapping, points at the pole map to the top or
 * bottom edge of the texture.  But this is a technicality that doesn't really
 * apply to us.)  We still solve it the same way: if our point is at or near
 * the pole, compute the angle based on the centroid of the polygon (which we
 * assume is further from the pole).
 */
    xz.set(centroid[0], centroid[2]);
  }

  // And cylinders do still have a seam at the back.
  double u = rad_2_deg(atan2(xz[0], xz[1])) / _u_angle;
  double c = rad_2_deg(atan2(centroid[0], centroid[2])) / _u_angle;

  if (u - c > 0.5) {
    u -= floor(u - c + 0.5);
  } else if (u - c < -0.5) {
    u += floor(c - u + 0.5);
  }

  // For a cylindrical mapping, the V value comes directly from Y. Easy.
  LPoint2d uv(u - 0.5, pos[1]);

  nassertr(fabs(u - c) <= 0.5, uv);
  return uv;
}
