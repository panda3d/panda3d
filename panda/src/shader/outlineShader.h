// Filename: outlineShader.h
// Created by:  mike (09Jan97)
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
#ifndef OUTILNESHADER_H
#define OUTLINESHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "shader.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : OutlineShader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER OutlineShader : public Shader {
public:
  OutlineShader(void);
  OutlineShader(const Colorf &color);
  ~OutlineShader() { }

  virtual void config(void);
  virtual void apply(Node *node, const AllAttributesWrapper &init_state,
                     const AllTransitionsWrapper &net_trans,
                     GraphicsStateGuardian *gsg);

  INLINE void set_color(const Colorf &color) { _color = color; }

protected:
  Colorf                _color;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Shader::init_type();
    register_type(_type_handle, "OutlineShader",
                        Shader::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
