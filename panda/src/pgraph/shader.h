// Filename: shader.h
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

#ifndef SHADER_H
#define SHADER_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "dcast.h"

class ShaderExpansion;

////////////////////////////////////////////////////////////////////
//       Class : Shader
// Description : 
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA Shader: public TypedWritableReferenceCount {
PUBLISHED:
  static CPT(Shader) load(const Filename &file, int preprocessor=0);
  static CPT(Shader) load(const string &file,   int preprocessor=0);
  static CPT(Shader) make(const string &body,   int preprocessor=0);
  
  INLINE const string   &get_name() const;
  INLINE const Filename &get_file() const;
  INLINE const string   &get_body() const;
  INLINE int             get_preprocessor() const;
  INLINE bool            get_loaded() const;
  
  PT(ShaderExpansion) macroexpand(const RenderState *context) const;
  
public:
  Shader();
  ~Shader();
  
private:
  string   _name;
  Filename _file;
  string   _body;
  bool     _loaded;
  int      _preprocessor;
  
  PT(ShaderExpansion) _fixed_expansion;
  
  typedef pair < Filename, int > LoadTableKey;
  typedef pair < string,   int > MakeTableKey;

  typedef pmap < LoadTableKey , Shader * > LoadTable;
  typedef pmap < MakeTableKey , Shader * > MakeTable;
  
  static LoadTable _load_table;
  static MakeTable _make_table;
  
public:
  static void register_with_read_factory();

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

#endif  // SHADER_H
