// Filename: mayaShader.cxx
// Created by:  drose (01Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_maya.h"
#include "string_utils.h"
#include "pnmImageHeader.h"  // for lumin_red, etc.
#include "pset.h"

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
  MFnDependencyNode engine_fn(engine);

  set_name(engine_fn.name().asChar());

  if (maya_cat.is_debug()) {
    maya_cat.debug()
      << "Reading shading engine " << get_name() << "\n";
  }

  bool found_shader = false;
  MPlug shader_plug = engine_fn.findPlug("surfaceShader");
  if (!shader_plug.isNull()) {
    MPlugArray shader_pa;
    shader_plug.connectedTo(shader_pa, true, false);
    maya_cat.spam() << "shader plug connected to: " << shader_pa.length() << endl;
    for (size_t i = 0; i < shader_pa.length() && !found_shader; i++) {
      MObject shader = shader_pa[0].node();
      found_shader = read_surface_shader(shader);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaShader::
~MayaShader() {
  for (size_t i=0; i<_color.size(); ++i) {
    _color.pop_back();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MayaShader::
output(ostream &out) const {
  out << "Shader " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MayaShader::
write(ostream &out) const {
  out << "Shader " << get_name() << "\n"
      << "  color:\n";

  for (size_t i=0; i<_color.size(); ++i) {
    _color[i]->write(out);
  }

  out << "  transparency:\n";
  _transparency.write(out);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::get_color_def
//       Access: Public
//  Description: Now that the shaders can have multiple textures
//               return the color def i.e. texture at idx
////////////////////////////////////////////////////////////////////
MayaShaderColorDef *MayaShader::
get_color_def(size_t idx) const {
  if (_color.size() > 0)
    return _color[idx];
  else
    return (MayaShaderColorDef *)NULL;
}
////////////////////////////////////////////////////////////////////
//     Function: MayaShader::get_rgba
//       Access: Public
//  Description: Returns the overall color of the shader as a
//               single-precision rgba value, where the alpha
//               component represents transparency according to the
//               Panda convention.  If no overall color is specified
//               (_has_flat_color is not true), this returns white.
//
//               Normally, Maya makes texture color override the flat
//               color, so if a texture is also applied (_has_texture
//               is true), this value is not used by Maya.
////////////////////////////////////////////////////////////////////
Colorf MayaShader::
get_rgba(size_t idx) const {
  Colorf rgba(1.0f, 1.0f, 1.0f, 1.0f);

  if (_color.size() && _color[idx]->_has_flat_color) {
    rgba[0] = (float)_color[idx]->_flat_color[0];
    rgba[1] = (float)_color[idx]->_flat_color[1];
    rgba[2] = (float)_color[idx]->_flat_color[2];
  }

  if (_transparency._has_flat_color) {
    // Maya supports colored transparency, but we only support
    // grayscale transparency.  Use the pnmimage constants to
    // convert color to grayscale.
    double trans =
      _transparency._flat_color[0] * lumin_red + 
      _transparency._flat_color[1] * lumin_grn + 
      _transparency._flat_color[2] * lumin_blu;
    rgba[3] = 1.0f - (float)trans;
  }

  return rgba;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::read_surface_shader
//       Access: Private
//  Description: Extracts out the shading information from the Maya
//               surface shader.
////////////////////////////////////////////////////////////////////
bool MayaShader::
read_surface_shader(MObject shader) {
  MStatus status;
  MFnDependencyNode shader_fn(shader);
  
  if (maya_cat.is_spam()) {
    maya_cat.spam()
      << "  Reading surface shader " << shader_fn.name().asChar() << "\n";
  }

  // First, check for a connection to the color attribute.  This could
  // be a texture map or something, and will override whatever the
  // shader says for color.

  MPlug color_plug = shader_fn.findPlug("color");
  if (color_plug.isNull()) {
    // Or maybe a connection to outColor.  Not sure how this differs
    // from just color, but empirically it seems that either might be
    // used.
    color_plug = shader_fn.findPlug("outColor");
  }

  if (!color_plug.isNull()) {
    MPlugArray color_pa;
    color_plug.connectedTo(color_pa, true, false);

    MayaShaderColorDef *color_p = new MayaShaderColorDef;
    for (size_t i = 0; i < color_pa.length(); i++) {
      maya_cat.spam() << "color_pa[" << i << "]:" << color_pa[i].name() << endl;
      color_p->read_surface_color(this, color_pa[0].node());
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
      _transparency.read_surface_color(this, trans_pa[0].node(), true);
    }
  }

  // Also try to get the ordinary color directly from the surface
  // shader.
  bool b_color_def = true;
  if (shader.hasFn(MFn::kLambert)) {
    MFnLambertShader lambert_fn(shader);
    MColor color = lambert_fn.color(&status);
    if (status) {
      // Warning! The alpha component of color doesn't mean
      // transparency in Maya.
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
  //  if (!_color._has_flat_color && !_color._has_texture) {
  if (!b_color_def) {
    maya_cat.info() << shader_fn.name().asChar() << "Color def not found" << endl;
    if (maya_cat.is_spam()) {
      maya_cat.spam()
        << "  Color definition not found.\n";
    }
  }
  return true;
}
