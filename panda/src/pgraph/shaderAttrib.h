// Filename: shaderAttrib.h
// Created by:  sshodhan (10Jul04)
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

#ifndef SHADERATTRIB_H
#define SHADERATTRIB_H

#include "pandabase.h"
#include "luse.h"
#include "pmap.h"
#include "shader.h"
#include "renderAttrib.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "factoryParam.h"
#include "dcast.h"
#include "shaderMode.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderAttrib
// Description : fill me in
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderAttrib: public RenderAttrib {

private:
  INLINE ShaderAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make(ShaderMode *sm);
  static CPT(RenderAttrib) make_off();

  INLINE bool is_off() const;
  INLINE ShaderMode *get_shader_mode() const;

  static void register_with_read_factory();

public:
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual RenderAttrib *make_default_impl() const;
  virtual int compare_to_impl(const RenderAttrib *other) const;

private:
  PT(ShaderMode) _shader_mode;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ShaderAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "shaderAttrib.I"

#endif  // SHADERATTRIB_H



