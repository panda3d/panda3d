// Filename: mayaShader.cxx
// Created by:  drose (01Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_maya.h"

#include "pre_maya_include.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MColor.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::Constructor
//       Access: Public
//  Description: Reads the Maya "shading engine" to determine the
//               relevant shader properties.
////////////////////////////////////////////////////////////////////
MayaShader::
MayaShader(MObject engine) {
  _has_color = false;
  _transparency = 0.0;

  _has_texture = false;

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

  MFnDependencyNode engine_fn(engine);

  _name = engine_fn.name().asChar();

  if (maya_cat.is_debug()) {
    maya_cat.debug()
      << "Reading shading engine " << _name << "\n";
  }

  bool found_shader = false;
  MPlug shader_plug = engine_fn.findPlug("surfaceShader");
  if (!shader_plug.isNull()) {
    MPlugArray shader_pa;
    shader_plug.connectedTo(shader_pa, true, false);

    for (size_t i = 0; i < shader_pa.length() && !found_shader; i++) {
      MObject shader = shader_pa[0].node();
      found_shader = read_surface_shader(shader);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::compute_texture_matrix
//       Access: Public
//  Description: Returns a texture matrix corresponding to the texture
//               transforms indicated by the shader.
////////////////////////////////////////////////////////////////////
LMatrix3d MayaShader::
compute_texture_matrix() {
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

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MayaShader::
output(ostream &out) const {
  out << "Shader " << _name << ":\n";
  if (_has_texture) {
    out << "  texture is " << _texture << "\n"
        << "  coverage is " << _coverage << "\n"
        << "  translate_frame is " << _translate_frame << "\n"
        << "  rotate_frame is " << _rotate_frame << "\n"
        << "  mirror is " << _mirror << "\n"
        << "  stagger is " << _stagger << "\n"
        << "  wrap_u is " << _wrap_u << "\n"
        << "  wrap_v is " << _wrap_v << "\n"
        << "  repeat_uv is " << _repeat_uv << "\n"
        << "  offset is " << _offset << "\n"
        << "  rotate_uv is " << _rotate_uv << "\n";

  } else if (_has_color) {
    out << "  color is " << _color << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::read_surface_shader
//       Access: Public
//  Description: Extracts out the shading information from the Maya
//               surface shader.
////////////////////////////////////////////////////////////////////
bool MayaShader::
read_surface_shader(MObject shader) {
  MStatus status;
  MFnDependencyNode shader_fn(shader);

  if (maya_cat.is_spam()) {
    maya_cat.spam()
      << "  Reading surface shader " << shader_fn.name() << "\n";
  }

  // First, check for a connection to the color attribute.  This could
  // be a texture map or something, and will override whatever the
  // shader says for color.

  MPlug color_plug = shader_fn.findPlug("color");
  if (!color_plug.isNull()) {
    MPlugArray color_pa;
    color_plug.connectedTo(color_pa, true, false);

    for (size_t i = 0; i < color_pa.length(); i++) {
      read_surface_color(color_pa[0].node());
    }
  }

  // Also try to get the ordinary color directly from the surface
  // shader.
  if (shader.hasFn(MFn::kLambert)) {
    MFnLambertShader lambert_fn(shader);
    MColor color = lambert_fn.color(&status);
    if (status) {
      _color.set(color.r, color.g, color.b, color.a);
      _has_color = true;
    }
  }

  if (!_has_color && !_has_texture) {
    if (maya_cat.is_spam()) {
      maya_cat.spam()
        << "  Color definition not found.\n";
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::read_surface_color
//       Access: Public
//  Description: Determines the surface color specified by the shader.
//               This includes texturing and other advanced shader
//               properties.
////////////////////////////////////////////////////////////////////
void MayaShader::
read_surface_color(MObject color) {
  if (color.hasFn(MFn::kFileTexture)) {
    string filename;
    _has_texture = get_string_attribute(color, "fileTextureName", filename);
    if (_has_texture) {
      _texture = Filename::from_os_specific(filename);
    }

    get_vec2f_attribute(color, "coverage", _coverage);
    get_vec2f_attribute(color, "translateFrame", _translate_frame);
    get_angle_attribute(color, "rotateFrame", _rotate_frame);

    get_bool_attribute(color, "mirror", _mirror);
    get_bool_attribute(color, "stagger", _stagger);
    get_bool_attribute(color, "wrapU", _wrap_u);
    get_bool_attribute(color, "wrapV", _wrap_v);

    get_vec2f_attribute(color, "repeatUV", _repeat_uv);
    get_vec2f_attribute(color, "offset", _offset);
    get_angle_attribute(color, "rotateUV", _rotate_uv);

  } else {
    // This shader wasn't understood.
    if (maya_cat.is_debug()) {
      maya_cat.info()
        << "**Don't know how to interpret color attribute type "
        << color.apiTypeStr() << "\n";

    } else {
      // If we don't have a heavy verbose count, only report each type
      // of unsupportted shader once.
      static pset<MFn::Type> bad_types;
      if (bad_types.insert(color.apiType()).second) {
        maya_cat.info()
          << "**Don't know how to interpret color attribute type "
          << color.apiTypeStr() << "\n";
      }
    }
  }
}
