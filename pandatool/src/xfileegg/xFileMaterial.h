// Filename: xFileMaterial.h
// Created by:  drose (19Jun01)
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

#ifndef XFILEMATERIAL_H
#define XFILEMATERIAL_H

#include "pandatoolbase.h"
#include "luse.h"
#include "filename.h"

class EggPrimitive;
class Datagram;
class XFileToEggConverter;
class XFileNode;
class XFileDataNode;

////////////////////////////////////////////////////////////////////
//       Class : XFileMaterial
// Description : This represents an X file "material", which consists
//               of a color, lighting, and/or texture specification.
////////////////////////////////////////////////////////////////////
class XFileMaterial {
public:
  XFileMaterial();
  ~XFileMaterial();

  void set_from_egg(EggPrimitive *egg_prim);
  void apply_to_egg(EggPrimitive *egg_prim, XFileToEggConverter *converter);

  int compare_to(const XFileMaterial &other) const;

  bool has_material() const;
  bool has_texture() const;

  XFileDataNode *make_x_material(XFileNode *x_meshMaterials, const string &suffix);
  bool read_material_data(const Datagram &raw_data);
  bool read_texture_data(const Datagram &raw_data);

private:
  Colorf _face_color;
  float _power;
  RGBColorf _specular_color;
  RGBColorf _emissive_color;
  Filename _texture;

  bool _has_material;
  bool _has_texture;
};

#endif

