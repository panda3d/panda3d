// Filename: mayaShaderColorDef.h
// Created by:  drose (12Apr03)
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

#ifndef MAYASHADERCOLORDEF_H
#define MAYASHADERCOLORDEF_H

#include "pandatoolbase.h"

#include "luse.h"
#include "lmatrix.h"

class MObject;

////////////////////////////////////////////////////////////////////
//       Class : MayaShaderColorDef
// Description : This defines the various attributes that Maya may
//               associate with the "color" channel for a particular
//               shader (as well as on the "transparency" channel).
////////////////////////////////////////////////////////////////////
class MayaShaderColorDef {
public:
  MayaShaderColorDef();
  ~MayaShaderColorDef();
  
  LMatrix3d compute_texture_matrix() const;
  bool has_projection() const;
  TexCoordd project_uv(const LPoint3d &pos, const LPoint3d &ref_point) const;
  bool reset_maya_texture(const Filename &texture);
  
  void write(ostream &out) const;

  enum ProjectionType {
    PT_off,
    PT_planar,
    PT_spherical,
    PT_cylindrical,
    PT_ball,
    PT_cubic,
    PT_triplanar,
    PT_concentric,
    PT_perspective,
  };

  bool _has_texture;
  Filename _texture;
  
  bool _has_flat_color;
  Colord _flat_color;
  
  ProjectionType _projection_type;
  LMatrix4d _projection_matrix;
  double _u_angle;
  double _v_angle;
    
  LVector2f _coverage;
  LVector2f _translate_frame;
  double _rotate_frame;
  
  bool _mirror;
  bool _stagger;
  bool _wrap_u;
  bool _wrap_v;
  
  LVector2f _repeat_uv;
  LVector2f _offset;
  double _rotate_uv;
  
private:
  void read_surface_color(MObject color);
  void set_projection_type(const string &type);

  LPoint2d map_planar(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const;

  // Define a pointer to one of the above member functions.
  LPoint2d (MayaShaderColorDef::*_map_uvs)(const LPoint3d &pos, const LPoint3d &centroid) const;
  
  MObject *_color_object;
  
  friend class MayaShader;
};

#endif

