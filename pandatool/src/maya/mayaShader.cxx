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
#include "mayaFile.h"
#include "global_parameters.h"

#include "eggPrimitive.h"
#include "eggTexture.h"
#include "eggTextureCollection.h"

#include "pre_maya_include.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MColor.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include "post_maya_include.h"

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

  if (verbose >= 2) {
    nout << "Reading shading engine " << _name << "\n";
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

void MayaShader::
set_attributes(EggPrimitive &primitive, MayaFile &file) {
  if (_has_texture) {
    EggTextureCollection &textures = file._textures;
    EggTexture tex(_name, _texture);
    tex.set_wrap_u(_wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp);
    tex.set_wrap_v(_wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp);

    // Let's mipmap all textures by default.
    tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
    tex.set_magfilter(EggTexture::FT_linear);

    LMatrix3d mat = compute_texture_matrix();
    if (!mat.almost_equal(LMatrix3d::ident_mat())) {
      tex.set_transform(mat);
    }

    EggTexture *new_tex =
      textures.create_unique_texture(tex, ~EggTexture::E_tref_name);

    primitive.set_texture(new_tex);

  } else if (_has_color) {
    primitive.set_color(Colorf(_color[0], _color[1], _color[2], 1.0));
  }
}

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

bool MayaShader::
read_surface_shader(MObject shader) {
  MStatus status;
  MFnDependencyNode shader_fn(shader);

  if (verbose >= 3) {
    nout << "  Reading surface shader " << shader_fn.name() << "\n";
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
    if (verbose >= 2) {
      nout << "  Color definition not found.\n";
    }
  }
  return true;
}

void MayaShader::
read_surface_color(MObject color) {
  if (color.hasFn(MFn::kFileTexture)) {
    _has_texture = get_string_attribute(color, "fileTextureName", _texture);

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
    if (verbose >= 2) {
      nout << "**Don't know how to interpret color attribute type "
           << color.apiTypeStr() << "\n";
    } else {
      // If we don't have a heavy verbose count, only report each type
      // of unsupportted shader once.
      static pset<MFn::Type> bad_types;
      if (bad_types.insert(color.apiType()).second) {
        if (verbose == 1) {
          nout << "\n";
        }
        nout << "Don't know how to interpret color attribute type "
             << color.apiTypeStr() << "\n";
      }
    }
  }
}
