/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileMaterial.h
 * @author drose
 * @date 2001-06-19
 */

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

/**
 * This represents an X file "material", which consists of a color, lighting,
 * and/or texture specification.
 */
class XFileMaterial {
public:
  XFileMaterial();
  ~XFileMaterial();

  void set_from_egg(EggPrimitive *egg_prim);
  void apply_to_egg(EggPrimitive *egg_prim, XFileToEggConverter *converter);

  int compare_to(const XFileMaterial &other) const;

  bool has_material() const;
  bool has_texture() const;

  XFileDataNode *make_x_material(XFileNode *x_meshMaterials, const std::string &suffix);
  bool fill_material(XFileDataNode *obj);

private:
  LColor _face_color;
  double _power;
  LRGBColor _specular_color;
  LRGBColor _emissive_color;
  Filename _texture;

  bool _has_material;
  bool _has_texture;
};

#endif
