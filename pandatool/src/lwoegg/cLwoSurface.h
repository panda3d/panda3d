// Filename: cLwoSurface.h
// Created by:  drose (25Apr01)
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

#ifndef CLWOSURFACE_H
#define CLWOSURFACE_H

#include "pandatoolbase.h"

#include "cLwoSurfaceBlock.h"

#include "lwoSurface.h"
#include "luse.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "pt_EggTexture.h"
#include "pt_EggMaterial.h"
#include "vector_PT_EggVertex.h"

#include "pmap.h"

class LwoToEggConverter;
class LwoSurfaceBlock;
class EggPrimitive;

////////////////////////////////////////////////////////////////////
//       Class : CLwoSurface
// Description : This class is a wrapper around LwoSurface and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoSurface {
public:
  CLwoSurface(LwoToEggConverter *converter, const LwoSurface *surface);
  ~CLwoSurface();

  INLINE const string &get_name() const;

  void apply_properties(EggPrimitive *egg_prim,
                        vector_PT_EggVertex &egg_vertices,
                        float &smooth_angle);
  bool check_texture();
  bool check_material();

  INLINE bool has_named_uvs() const;
  INLINE const string &get_uv_name() const;


  enum Flags {
    F_rgb          = 0x0001,
    F_diffuse      = 0x0002,
    F_luminosity   = 0x0004,
    F_specular     = 0x0008,
    F_reflection   = 0x0010,
    F_transparency = 0x0020,
    F_gloss        = 0x0040,
    F_translucency = 0x0080,
    F_smooth_angle = 0x0100,
    F_backface     = 0x0200,
  };

  int _flags;
  RGBColorf _rgb;
  float _diffuse;
  float _luminosity;
  float _specular;
  float _reflection;
  float _transparency;
  float _gloss;
  float _translucency;
  float _smooth_angle;
  bool _backface;

  Colorf _color;
  Colorf _diffuse_color;

  LwoToEggConverter *_converter;
  CPT(LwoSurface) _surface;

  bool _checked_material;
  PT_EggMaterial _egg_material;

  bool _checked_texture;
  PT_EggTexture _egg_texture;

  CLwoSurfaceBlock *_block;

private:
  void generate_uvs(vector_PT_EggVertex &egg_vertices);

  LPoint2d map_planar(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_cubic(const LPoint3d &pos, const LPoint3d &centroid) const;

  // Define a pointer to one of the above member functions.
  LPoint2d (CLwoSurface::*_map_uvs)(const LPoint3d &pos, const LPoint3d &centroid) const;
};

#include "cLwoSurface.I"

#endif


