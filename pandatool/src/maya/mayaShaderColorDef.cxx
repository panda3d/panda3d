// Filename: mayaShaderColorDef.cxx
// Created by:  drose (12Apr03)
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
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaShaderColorDef::
MayaShaderColorDef() {
  _color_gain.set(1.0f, 1.0f, 1.0f, 1.0f);

  _has_flat_color = false;
  _flat_color.set(0.0, 0.0, 0.0, 0.0);

  _has_texture = false;
  _texture_name = "";
  _uvset_name = "default";
  _projection_type = PT_off;
  _map_uvs = NULL;

  _coverage.set(1.0, 1.0);
  _translate_frame.set(0.0, 0.0);
  _rotate_frame = 0.0;

  _mirror = false;
  _stagger = false;
  _wrap_u = true;
  _wrap_v = true;

  _alpha_is_luminance = false;

  _repeat_uv.set(1.0, 1.0);
  _offset.set(0.0, 0.0);
  _rotate_uv = 0.0;

  _color_object = (MObject *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
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

  _alpha_is_luminance = copy._alpha_is_luminance;

  _repeat_uv = copy._repeat_uv;
  _offset = copy._offset;
  _rotate_uv = copy._rotate_uv;

  _map_uvs = copy._map_uvs;
  _color_object = copy._color_object;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaShaderColorDef::
~MayaShaderColorDef() {
  if (_color_object != (MObject *)NULL) {
    delete _color_object;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::compute_texture_matrix
//       Access: Public
//  Description: Returns a texture matrix corresponding to the texture
//               transforms indicated by the shader.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::has_projection
//       Access: Public
//  Description: Returns true if the shader has a projection in effect.
////////////////////////////////////////////////////////////////////
bool MayaShaderColorDef::
has_projection() const {
  return (_projection_type != PT_off);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::project_uv
//       Access: Public
//  Description: If the shader has a projection (has_projection()
//               returns true), this computes the appropriate UV
//               corresponding to the indicated 3-d point.  Seams that
//               might be introduced on polygons that cross quadrants
//               are closed up by ensuring the point is in the same
//               quadrant as the indicated reference point.
////////////////////////////////////////////////////////////////////
TexCoordd MayaShaderColorDef::
project_uv(const LPoint3d &pos, const LPoint3d &centroid) const {
  nassertr(_map_uvs != NULL, TexCoordd::zero());
  return (this->*_map_uvs)(pos * _projection_matrix, centroid * _projection_matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MayaShaderColorDef::
write(ostream &out) const {
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

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::reset_maya_texture
//       Access: Public
//  Description: Changes the texture filename stored in the Maya file
//               for this particular shader.
////////////////////////////////////////////////////////////////////
bool MayaShaderColorDef::
reset_maya_texture(const Filename &texture) {
  if (_color_object != (MObject *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::read_surface_color
//       Access: Private
//  Description: Determines the surface color specified by the shader.
//               This includes texturing and other advanced shader
//               properties.
////////////////////////////////////////////////////////////////////
void MayaShaderColorDef::
read_surface_color(MayaShader *shader, MObject color, bool trans) {
  RGBColorf color_gain;
  if (get_vec3f_attribute(color, "colorGain", color_gain)) {
    _color_gain[0] *= color_gain[0];
    _color_gain[1] *= color_gain[1];
    _color_gain[2] *= color_gain[2];
  }
  float alpha_gain;
  if (get_maya_attribute(color, "alphaGain", alpha_gain)) {
    _color_gain[3] *= alpha_gain;
  }
  if (color.hasFn(MFn::kFileTexture)) {
    _color_object = new MObject(color);
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

    get_vec2f_attribute(color, "coverage", _coverage);
    get_vec2f_attribute(color, "translateFrame", _translate_frame);
    get_angle_attribute(color, "rotateFrame", _rotate_frame);

    get_bool_attribute(color, "alphaIsLuminance", _alpha_is_luminance);

    get_bool_attribute(color, "mirror", _mirror);
    get_bool_attribute(color, "stagger", _stagger);
    get_bool_attribute(color, "wrapU", _wrap_u);
    get_bool_attribute(color, "wrapV", _wrap_v);

    get_vec2f_attribute(color, "repeatUV", _repeat_uv);
    get_vec2f_attribute(color, "offset", _offset);
    get_angle_attribute(color, "rotateUV", _rotate_uv);

    if (!trans) {
      maya_cat.debug() << "pushed a file texture" << endl;
      shader->_color.push_back(this);
    }

  } else if (color.hasFn(MFn::kProjection)) {
    maya_cat.debug() << "reading a projection texture" << endl;
    // This is a projected texture.  We will have to step one level
    // deeper to find the actual texture.
    MFnDependencyNode projection_fn(color);
    MPlug image_plug = projection_fn.findPlug("image");
    if (!image_plug.isNull()) {
      MPlugArray image_pa;
      image_plug.connectedTo(image_pa, true, false);
      
      for (size_t i = 0; i < image_pa.length(); i++) {
        read_surface_color(shader, image_pa[0].node());
      }
    }

    if (!get_mat4d_attribute(color, "placementMatrix", _projection_matrix)) {
      _projection_matrix = LMatrix4d::ident_mat();
    }

    // The uAngle and vAngle might be used for certain kinds of
    // projections.
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
    maya_cat.debug() << "Found layered texture" << endl;
    //list_maya_attributes(color);
    //shader->_multi_texture = true;
    //get_enum_attribute(color,"blendMode",shader->_blend_mode);
    //maya_cat.debug() << "blend mode :" << shader->_blend_mode << endl;

    get_bool_attribute(color, "alphaIsLuminance", shader->_alpha_is_luminance);
    //    get_bool_attribute(color, "isVisible", test_b);

    MFnDependencyNode layered_fn(color);
    MPlugArray color_pa;
    layered_fn.getConnections(color_pa);
    maya_cat.debug() << "number of connections: " << color_pa.length() << endl;
    bool first = true;
    for (size_t i=0; i<color_pa.length(); ++i) {
      MPlug pl = color_pa[i];
      MPlugArray pla;
      pl.connectedTo(pla, true, false);
      for (size_t j=0; j<pla.length(); ++j) {
        //maya_cat.debug() << pl.name() << " is(pl) " << pl.node().apiTypeStr() << endl;
        //maya_cat.debug() << pla[j].name() << " is(pla) " << pla[j].node().apiTypeStr() << endl;
        string pla_name = pla[j].name().asChar();
        // sometimes, by default, maya gives a outAlpha on subsequent plugs, ignore that
        if (pla_name.find("outAlpha") != string::npos) {
          maya_cat.debug() << pl.name() << " ignoring: " << pla_name << endl;
          continue;
        }
        if (!first) {
          maya_cat.debug() << pl.name() << " next:connectedTo: " << pla_name << endl;
          MayaShaderColorDef *color_p = new MayaShaderColorDef;
          color_p->read_surface_color(shader, pla[j].node());
          color_p->_texture_name.assign(pla[j].name().asChar());
          size_t loc = color_p->_texture_name.find('.',0);
          if (loc != string::npos) {
            color_p->_texture_name.resize(loc);
          }
          maya_cat.debug() << "uv_name : " << color_p->_texture_name << endl;
        }
        else {
          maya_cat.debug() << pl.name() << " first:connectedTo: " << pla_name << endl;
          read_surface_color(shader, pla[j].node());
          _texture_name.assign(pla[j].name().asChar());
          size_t loc = _texture_name.find('.',0);
          if (loc != string::npos) {
            _texture_name.resize(loc);
          }
          maya_cat.debug() << "uv_name : " << _texture_name << endl;
          first = false;
        }

        // lets see what this is connected to!?
        MPlug pl_temp = pla[j];
        MPlugArray pla_temp;
        pl_temp.connectedTo(pla_temp, true, false);
        maya_cat.debug() << pl_temp.name() << " connectedTo:" << pla_temp.length() << " plugs\n";
      }
      /*
      string blah;
      get_enum_attribute(pl.node(),"blendMode",blah);
      maya_cat.info() << "rsc layer: blend mode :" << blah << endl;
      float alpha;
      get_maya_attribute(pl.node(),"alpha",alpha);
      maya_cat.info() << "rsc layer: alpha :" << alpha << endl;
      */
    }
  } else {
    // This shader wasn't understood.
    if (maya_cat.is_debug()) {
      maya_cat.info()
        << "**Don't know how to interpret color attribute type "
        << color.apiTypeStr() << "\n";

    } else {
      // If we don't have a heavy verbose count, only report each type
      // of unsupported shader once.
      static pset<MFn::Type> bad_types;
      if (bad_types.insert(color.apiType()).second) {
        maya_cat.info()
          << "**Don't know how to interpret color attribute type "
          << color.apiTypeStr() << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::set_projection_type
//       Access: Private
//  Description: Sets up the shader to apply UV's according to the
//               indicated projection type.
////////////////////////////////////////////////////////////////////
void MayaShaderColorDef::
set_projection_type(const string &type) {
  if (cmp_nocase(type, "planar") == 0) {
    _projection_type = PT_planar;
    _map_uvs = &MayaShaderColorDef::map_planar;

    // The Planar projection normally projects to a range (-1, 1) in
    // both axes.  Scale this into our UV range of (0, 1).
    _projection_matrix = _projection_matrix * LMatrix4d(0.5, 0.0, 0.0, 0.0,
                                                        0.0, 0.5, 0.0, 0.0,
                                                        0.0, 0.0, 1.0, 0.0,
                                                        0.5, 0.5, 0.0, 1.0);

  } else if (cmp_nocase(type, "cylindrical") == 0) {
    _projection_type = PT_cylindrical;
    _map_uvs = &MayaShaderColorDef::map_cylindrical;

    // The cylindrical projection is orthographic in the Y axis; scale
    // the range (-1, 1) in this axis into our UV range (0, 1).
    _projection_matrix = _projection_matrix * LMatrix4d(1.0, 0.0, 0.0, 0.0,
                                                        0.0, 0.5, 0.0, 0.0,
                                                        0.0, 0.0, 1.0, 0.0,
                                                        0.0, 0.5, 0.0, 1.0);

  } else if (cmp_nocase(type, "spherical") == 0) {
    _projection_type = PT_spherical;
    _map_uvs = &MayaShaderColorDef::map_spherical;

  } else {
    // Other projection types are currently unimplemented by the
    // converter.
    maya_cat.error()
      << "Don't know how to handle type " << type << " projections.\n";
    _projection_type = PT_off;
    _map_uvs = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::map_planar
//       Access: Private
//  Description: Computes a UV based on the given point in space,
//               using a planar projection.
////////////////////////////////////////////////////////////////////
LPoint2d MayaShaderColorDef::
map_planar(const LPoint3d &pos, const LPoint3d &) const {
  // A planar projection is about as easy as can be.  We ignore the Z
  // axis, and project the point into the XY plane.  Done.
  return LPoint2d(pos[0], pos[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::map_spherical
//       Access: Private
//  Description: Computes a UV based on the given point in space,
//               using a spherical projection.
////////////////////////////////////////////////////////////////////
LPoint2d MayaShaderColorDef::
map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const {
  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Y axis.  Project the vector
  // into the XZ plane to do this.

  LVector2d xz(pos[0], pos[2]);
  double xz_length = xz.length();

  if (xz_length < 0.01) {
    // If we have a point on or near either pole, we've got problems.
    // This point maps to the entire bottom edge of the image, so
    // which U value should we choose?  It does make a difference,
    // especially if we have a number of polygons around the south
    // pole that all share the common vertex.

    // We choose the U value based on the polygon's centroid.
    xz.set(centroid[0], centroid[2]);
  }

  // Now, if the polygon crosses the seam, we also have problems.
  // Make sure that the u value is in the same half of the texture as
  // the centroid's u value.
  double u = rad_2_deg(atan2(xz[0], xz[1])) / (2.0 * _u_angle);
  double c = rad_2_deg(atan2(centroid[0], centroid[2])) / (2.0 * _u_angle);

  if (u - c > 0.5) {
    u -= floor(u - c + 0.5);
  } else if (u - c < -0.5) {
    u += floor(c - u + 0.5);
  }

  // Now rotate the vector into the YZ plane, and the V value is based
  // on the latitude: the angle about the X axis.
  LVector2d yz(pos[1], xz_length);
  double v = rad_2_deg(atan2(yz[0], yz[1])) / (2.0 * _v_angle);

  LPoint2d uv(u - 0.5, v - 0.5);

  nassertr(fabs(u - c) <= 0.5, uv);
  return uv;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShaderColorDef::map_cylindrical
//       Access: Private
//  Description: Computes a UV based on the given point in space,
//               using a cylindrical projection.
////////////////////////////////////////////////////////////////////
LPoint2d MayaShaderColorDef::
map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const {
  // This is almost identical to the spherical projection, except for
  // the computation of V.

  LVector2d xz(pos[0], pos[2]);
  double xz_length = xz.length();

  if (xz_length < 0.01) {
    // A cylindrical mapping has the same singularity problem at the
    // pole as a spherical mapping does: points at the pole do not map
    // to a single point on the texture.  (It's technically a slightly
    // different problem: in a cylindrical mapping, points at the pole
    // do not map to any point on the texture, while in a spherical
    // mapping, points at the pole map to the top or bottom edge of
    // the texture.  But this is a technicality that doesn't really
    // apply to us.)  We still solve it the same way: if our point is
    // at or near the pole, compute the angle based on the centroid of
    // the polygon (which we assume is further from the pole).
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

  // For a cylindrical mapping, the V value comes directly from Y.
  // Easy.
  LPoint2d uv(u - 0.5, pos[1]);

  nassertr(fabs(u - c) <= 0.5, uv);
  return uv;
}
