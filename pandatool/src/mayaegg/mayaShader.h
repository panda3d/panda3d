// Filename: mayaShader.h
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

#ifndef MAYASHADER_H
#define MAYASHADER_H

#include "pandatoolbase.h"

#include "luse.h"
#include "lmatrix.h"

class MObject;
class MayaToEggConverter;
class EggPrimitive;

////////////////////////////////////////////////////////////////////
//       Class : MayaShader
// Description : Corresponds to a single "shader" in Maya.  This
//               extracts out all the parameters of a Maya shader that
//               are meaningful to egg.
////////////////////////////////////////////////////////////////////
class MayaShader {
public:
  MayaShader(MObject engine, MayaToEggConverter *converter);

  void set_attributes(EggPrimitive &primitive, MayaToEggConverter &conv);
  LMatrix3d compute_texture_matrix();

  void output(ostream &out) const;

  string _name;

  bool _has_color;
  Colord _color;
  double _transparency;

  bool _has_texture;
  string _texture;

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
  bool read_surface_shader(MObject shader);
  void read_surface_color(MObject color);

  MayaToEggConverter *_converter;
};

inline ostream &operator << (ostream &out, const MayaShader &shader) {
  shader.output(out);
  return out;
}

#endif

