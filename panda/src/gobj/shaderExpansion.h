// Filename: shaderExpansion.h
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

#ifndef SHADEREXPANSION_H
#define SHADEREXPANSION_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderExpansion
//      Summary: A shader can contain context-sensitive macros.
//               A ShaderExpansion is the output you get when you
//               run the macro preprocessor on a shader.
//               The ShaderExpansion contains the shader's 
//               macroexpanded text, and a map of ShaderContext
//               objects.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderExpansion: public TypedReferenceCount {

PUBLISHED:
  static PT(ShaderExpansion) make(const string &name, const string &body);
  
  INLINE const string &get_name() const;
  INLINE const string &get_text() const;
  
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
  ~ShaderExpansion();

  string         _name;
  string         _text;
  int            _parse;
  
  typedef pair < string, string > ExpansionKey;
  typedef pmap < ExpansionKey, ShaderExpansion * > ExpansionCache;
  static ExpansionCache _expansion_cache;

  friend class ShaderContext;
  friend class PreparedGraphicsObjects;

  typedef pmap <PreparedGraphicsObjects *, ShaderContext *> Contexts;
  Contexts _contexts;
  
  ShaderContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                             GraphicsStateGuardianBase *gsg);
  
private:  
  ShaderExpansion();
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ShaderExpansion",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderExpansion.I"

#endif
