// Filename: mayaShader.h
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

#ifndef MAYASHADER_H
#define MAYASHADER_H

#include "pandatoolbase.h"
#include "mayaShaderColorDef.h"

#include "luse.h"
#include "lmatrix.h"
#include "namable.h"

class MObject;

////////////////////////////////////////////////////////////////////
//       Class : MayaShader
// Description : Corresponds to a single "shader" in Maya.  This
//               extracts out all the parameters of a Maya shader that
//               we might care about.  There are many more parameters
//               that we don't care about or don't know enough to
//               extract.
////////////////////////////////////////////////////////////////////
class MayaShader : public Namable {
public:
  MayaShader(MObject engine);
  ~MayaShader();

  void output(ostream &out) const;
  void write(ostream &out) const;

  Colorf get_rgba() const;

  MayaShaderColorDef _color;
  MayaShaderColorDef _transparency;

private:
  bool read_surface_shader(MObject shader);
};

INLINE ostream &operator << (ostream &out, const MayaShader &shader) {
  shader.output(out);
  return out;
}

#endif

