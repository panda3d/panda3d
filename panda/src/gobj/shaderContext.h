/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderContext.h
 * @author jyelon
 * @date 2005-09-01
 */

#ifndef SHADERCONTEXT_H
#define SHADERCONTEXT_H

#include "pandabase.h"
#include "internalName.h"
#include "savedContext.h"
#include "shader.h"

/**
 * The ShaderContext is meant to contain the compiled version of a shader
 * string.  ShaderContext is an abstract base class, there will be a subclass
 * of it for each shader language and graphics API. Since the languages are so
 * different and the graphics APIs have so little in common, the base class
 * contains almost nothing.  All the implementation details are in the
 * subclasses.
 */

class EXPCL_PANDA_GOBJ ShaderContext: public SavedContext {
public:
  INLINE ShaderContext(Shader *se);

  virtual void set_state_and_transform(const RenderState *,
                                       const TransformState *,
                                       const TransformState *,
                                       const TransformState *) {};

  INLINE virtual bool valid() { return false; }
  INLINE virtual void bind() {};
  INLINE virtual void unbind() {};
  INLINE virtual void issue_parameters(int altered) {};
  INLINE virtual void disable_shader_vertex_arrays() {};
  INLINE virtual bool update_shader_vertex_arrays(ShaderContext *prev, bool force) { return false; };
  INLINE virtual void disable_shader_texture_bindings() {};
  INLINE virtual void update_shader_texture_bindings(ShaderContext *prev) {};
  INLINE virtual void update_shader_buffer_bindings(ShaderContext *prev) {};

  INLINE virtual bool uses_standard_vertex_arrays(void) { return true; };
  INLINE virtual bool uses_custom_vertex_arrays(void) { return false; };

PUBLISHED:
  INLINE Shader *get_shader() const;
  MAKE_PROPERTY(shader, get_shader);

public:
  Shader *_shader;

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
