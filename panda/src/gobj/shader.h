// Filename: shader.h
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

////////////////////////////////////////////////////////////////////
//
// Current Loose Ends:
//   - BAM reading/writing not implemented on most classes.
//   - ShaderPool not implemented.
//   - compilation of shaders for OpenGL not implemented.
//   - compilation of shaders for DirectX8 not implemented.
//   - compilation of shaders for DirectX9 not implemented.
//
////////////////////////////////////////////////////////////////////

#ifndef SHADER_H
#define SHADER_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"

////////////////////////////////////////////////////////////////////
//       Class : Shader
//      Summary: The Shader object contains the string which 
//               is the shader's text, a filename that indicates
//               where the shader came from (optional), and an
//               argument-name to argument-index allocator.  The
//               allocator is there so that all the Shader and
//               ShaderContext objects associated with this Shader
//               can refer to arguments by index instead of by name,
//               which could make the bind process significantly
//               faster.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA Shader: public TypedWritableReferenceCount {

PUBLISHED:
  Shader(const string &text, const string &file);
  ~Shader();

  INLINE const string   &get_text();
  INLINE const Filename &get_file();
  
  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

PUBLISHED:
  // These routines help split the shader into sections,
  // for those shader implementations that need to do so.
  void parse_init();
  void parse_line(string &result, bool rt, bool lt);
  void parse_upto(string &result, string pattern, bool include);
  void parse_rest(string &result);
  int  parse_lineno();
  bool parse_eof();
  
public:

  INLINE int arg_count();
  int        arg_index(const string &id);

  string         _text;
  Filename       _file;
  int            _parse;
  vector<string> _args;
  
  typedef pmap<PreparedGraphicsObjects *, ShaderContext *> Contexts;
  Contexts _contexts;

  static void register_with_read_factory();

  friend class ShaderContext;
  friend class PreparedGraphicsObjects;

  ShaderContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                             GraphicsStateGuardianBase *gsg);

private:  
  void parse();
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Shader",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shader.I"

#endif
