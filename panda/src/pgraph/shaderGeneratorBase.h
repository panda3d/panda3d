// Filename: shaderGeneratorBase.h
// Created by:  drose (05Nov08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SHADERGENERATORBASE_H
#define SHADERGENERATORBASE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderGeneratorBase
// Description : This is the abstract base class for ShaderGenerator.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ShaderGeneratorBase : public TypedWritableReferenceCount {
protected:
  ShaderGeneratorBase();

PUBLISHED:
  virtual ~ShaderGeneratorBase();
  virtual CPT(RenderAttrib) synthesize_shader(const RenderState *rs)=0;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderGeneratorBase",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "shaderGeneratorBase.I"

#endif  // SHADERGENERATORBASE_H

