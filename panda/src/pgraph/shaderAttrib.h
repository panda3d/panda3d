// Filename: shaderAttrib.h
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

#ifndef SHADERATTRIB_H
#define SHADERATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"
#include "pointerTo.h"
#include "shaderInput.h"
#include "shader.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderAttrib
// Description : 
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderAttrib: public RenderAttrib {

private:
  INLINE ShaderAttrib();
  INLINE ShaderAttrib(const ShaderAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_off();

  INLINE bool               has_shader() const;
  INLINE int                get_shader_priority() const;
  
  CPT(RenderAttrib) set_shader(const Shader *s, int priority=0) const;
  CPT(RenderAttrib) set_shader_off(int priority=0) const;
  CPT(RenderAttrib) clear_shader() const;
  CPT(RenderAttrib) set_shader_input(const ShaderInput *inp) const;
  CPT(RenderAttrib) set_shader_input(InternalName *id, Texture *tex,       int priority=0) const;
  CPT(RenderAttrib) set_shader_input(InternalName *id, const NodePath &np, int priority=0) const;
  CPT(RenderAttrib) set_shader_input(InternalName *id, const LVector4f &v, int priority=0) const;
  CPT(RenderAttrib) set_shader_input(InternalName *id, double n1=0, double n2=0, double n3=0, double n4=1,
                                     int priority=0) const;
  CPT(RenderAttrib) set_shader_input(const string &id, Texture *tex,       int priority=0) const;
  CPT(RenderAttrib) set_shader_input(const string &id, const NodePath &np, int priority=0) const;
  CPT(RenderAttrib) set_shader_input(const string &id, const LVector4f &v, int priority=0) const;
  CPT(RenderAttrib) set_shader_input(const string &id, double n1=0, double n2=0, double n3=0, double n4=1,
                                     int priority=0) const;
  CPT(RenderAttrib) clear_shader_input(InternalName *id) const;
  CPT(RenderAttrib) clear_shader_input(const string &id) const;

  const Shader      *get_shader() const;
  const ShaderInput *get_shader_input(InternalName *id) const;
  const ShaderInput *get_shader_input(const string &id) const;
  
  static void register_with_read_factory();
  
public:
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual RenderAttrib *make_default_impl() const;
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  
private:
  CPT(Shader) _shader;
  int         _shader_priority;
  bool        _has_shader;
  typedef pmap < CPT(InternalName), CPT(ShaderInput) > Inputs;
  Inputs _inputs;

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



