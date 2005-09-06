// Filename: shaderMode.h
// Created by:  jyelon (01Sep05)
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

#ifndef SHADERMODE_H
#define SHADERMODE_H

#include "pandabase.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderModeArg
// Description : The ShaderModeArg is a small structure designed to hold
//               any of the various kinds of values that can be
//               passed to a shader.  Basically, it is a temporary
//               storage repository for shader input parameters.
////////////////////////////////////////////////////////////////////

class ShaderModeArg
{
public:
  enum ShaderModeArgType
  {
    SAT_INVALID,
    SAT_FLOAT,
    SAT_TEXTURE,
    SAT_NODEPATH,
  };

  INLINE ShaderModeArg(void);
  INLINE ~ShaderModeArg(void);
  
  int         _type;
  NodePath    _nvalue;
  PT(Texture) _tvalue;
  LVector4d   _fvalue;
};

////////////////////////////////////////////////////////////////////
//       Class : ShaderMode
//      Summary: The ShaderMode object contains a Shader and a
//               list of input data to pass into the shader when
//               it is executing.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderMode: public TypedWritableReferenceCount {

public:
  PT(Shader)            _shader;
  vector<ShaderModeArg> _args;

PUBLISHED:
  ShaderMode(Shader *body);

  static PT(ShaderMode) load(const string   &file);
  static PT(ShaderMode) load(const Filename &file);
  static PT(ShaderMode) make(const string   &text);
  
  ~ShaderMode();
  
  INLINE Shader *get_shader(void);
  
  // Overloaded set_param to be used based on your param type
  void set_param(const string &pname, Texture *t);
  void set_param(const string &pname, const NodePath  &np);
  void set_param(const string &pname, float     p1f);
  void set_param(const string &pname, LVector2f p2f);
  void set_param(const string &pname, LVector3f p3f);
  void set_param(const string &pname, LVector4f p4f);
  void set_param(const string &pname, double    p1d);
  void set_param(const string &pname, LVector2d p2d);
  void set_param(const string &pname, LVector3d p3d);
  void set_param(const string &pname, LVector4d p4d);

  static void register_with_read_factory(void);

private:
  ShaderModeArg *mod_param(const string &pname, int kind);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderMode",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "shaderMode.I"

#endif


