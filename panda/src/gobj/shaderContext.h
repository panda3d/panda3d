// Filename: shaderContext.h
// Created by: jyelon (01Sep05)
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

#ifndef SHADERCONTEXT_H
#define SHADERCONTEXT_H

#include "pandabase.h"
#include "internalName.h"
#include "savedContext.h"
#include "shader.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderContext
// Description : The ShaderContext is meant to contain the compiled
//               version of a shader string.  ShaderContext is an
//               abstract base class, there will be a subclass of it
//               for each shader language and graphics API.
//               Since the languages are so different and the
//               graphics APIs have so little in common, the base
//               class contains almost nothing.  All the implementation
//               details are in the subclasses.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderContext: public SavedContext {
public:
  INLINE ShaderContext(Shader *shader);
  
  Shader *_shader;
  
public:
  // The following declarations represent useful routines
  // and constants that can be used by shader implementations.
  
  enum {
    SHADER_type_vert=0,
    SHADER_type_frag=1,
    SHADER_type_both=2,
  };
  enum {
    SHADER_arg_world = -1,
    SHADER_arg_camera = -2,
    SHADER_arg_model = -3,
  };
  enum {
    SHADER_data_matrix,
    SHADER_data_transpose,
    SHADER_data_row0,
    SHADER_data_row1,
    SHADER_data_row2,
    SHADER_data_row3,
    SHADER_data_col0,
    SHADER_data_col1,
    SHADER_data_col2,
    SHADER_data_col3,
  };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderContext",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderContext.I"

#endif
