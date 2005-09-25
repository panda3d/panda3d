// Filename: shaderInput.h
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

#ifndef SHADERINPUT_H
#define SHADERINPUT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderInput
// Description : This is a small container class that can hold any
//               one of the value types that can be passed as input
//               to a shader.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderInput: public TypedWritableReferenceCount {
public:
  INLINE ~ShaderInput();

PUBLISHED:
  static const ShaderInput *get_blank();
  INLINE ShaderInput(InternalName *id, int priority=0);
  INLINE ShaderInput(InternalName *id, const NodePath &np, int priority=0);
  INLINE ShaderInput(InternalName *id, const LVector4f &v, int priority=0);
  INLINE ShaderInput(InternalName *id, Texture  *tex,      int priority=0);
  
  enum ShaderInputType {
    M_invalid = 0,
    M_texture,
    M_nodepath,
    M_vector
  };
  
  INLINE InternalName *get_name() const;
  
  INLINE int               get_value_type() const;
  INLINE int               get_priority() const;
  INLINE Texture          *get_texture() const;
  INLINE const NodePath   &get_nodepath() const;
  INLINE const LVector4f  &get_vector() const;
  
public:
  static void register_with_read_factory();

private:
  PT(InternalName) _name;
  int _type;
  int _priority;
  PT(Texture) _stored_texture;
  NodePath    _stored_nodepath;
  LVector4f   _stored_vector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "ShaderInput",
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "shaderInput.I"

#endif  // SHADERINPUT_H

