/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.h
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 *   for set_shader_input)
 * @author weifengh, PandaSE
 * @date 2010-04-15
 */

#ifndef SHADERATTRIB_H
#define SHADERATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"
#include "pointerTo.h"
#include "shaderInput.h"
#include "shader.h"
#include "pta_float.h"
#include "pta_double.h"
#include "pta_LMatrix4.h"
#include "pta_LMatrix3.h"
#include "pta_LVecBase4.h"
#include "pta_LVecBase3.h"
#include "pta_LVecBase2.h"
#include "extension.h"

/**
 *
 */
class EXPCL_PANDA_PGRAPH ShaderAttrib: public RenderAttrib {
private:
  INLINE ShaderAttrib();
  INLINE ShaderAttrib(const ShaderAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make(const Shader *shader = nullptr, int priority = 0);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  enum {
    F_disable_alpha_write = 0,  // Suppress writes to color buffer alpha channel.
    F_subsume_alpha_test  = 1,  // Shader promises to subsume the alpha test using TEXKILL
    F_hardware_skinning   = 2,  // Shader needs pre-animated vertices
    F_shader_point_size   = 3,  // Shader provides point size, not RenderModeAttrib
  };

  INLINE bool               has_shader() const;
  INLINE bool               auto_shader() const;
  INLINE int                get_shader_priority() const;
  INLINE int                get_instance_count() const;
  INLINE bool               auto_normal_on() const;
  INLINE bool               auto_glow_on() const;
  INLINE bool               auto_gloss_on() const;
  INLINE bool               auto_ramp_on() const;
  INLINE bool               auto_shadow_on() const;

  CPT(RenderAttrib) set_shader(const Shader *s, int priority=0) const;
  CPT(RenderAttrib) set_shader_off(int priority=0) const;
  CPT(RenderAttrib) set_shader_auto(int priority=0) const;

  CPT(RenderAttrib) set_shader_auto(BitMask32 shader_switch, int priority=0) const;

  CPT(RenderAttrib) clear_shader() const;
  // Shader Inputs
  CPT(RenderAttrib) set_shader_input(const ShaderInput &input) const;
  CPT(RenderAttrib) set_shader_input(ShaderInput &&input) const;

public:
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, Texture *tex,       int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const NodePath &np, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_float &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_double &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LMatrix4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LMatrix3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase2 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase2 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LMatrix4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LMatrix3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, double n1=0, double n2=0, double n3=0, double n4=1,
                                            int priority=0) const;
                                            
  CPT(RenderAttrib) set_shader_inputs(const pvector<ShaderInput> &inputs) const;

PUBLISHED:
  EXTENSION(CPT(RenderAttrib) set_shader_input(CPT_InternalName, PyObject *, int priority=0) const);
  EXTENSION(CPT(RenderAttrib) set_shader_inputs(PyObject *args, PyObject *kwargs) const);

  CPT(RenderAttrib) set_instance_count(int instance_count) const;

  CPT(RenderAttrib) set_flag(int flag, bool value) const;
  CPT(RenderAttrib) clear_flag(int flag) const;

  CPT(RenderAttrib) clear_shader_input(const InternalName *id) const;
  CPT(RenderAttrib) clear_shader_input(const std::string &id) const;

  CPT(RenderAttrib) clear_all_shader_inputs() const;

  INLINE bool get_flag(int flag) const;
  INLINE bool has_shader_input(CPT_InternalName id) const;

  const Shader *get_shader() const;
  const ShaderInput &get_shader_input(const InternalName *id) const;
  const ShaderInput &get_shader_input(const std::string &id) const;

  NodePath get_shader_input_nodepath(const InternalName *id) const;
  LVecBase4 get_shader_input_vector(InternalName *id) const;
  Texture *get_shader_input_texture(const InternalName *id, SamplerState *sampler=nullptr) const;
  const Shader::ShaderPtrData *get_shader_input_ptr(const InternalName *id) const;
  bool get_shader_input_ptr(const InternalName *id, Shader::ShaderPtrData &data) const;
  const LMatrix4 &get_shader_input_matrix(const InternalName *id, LMatrix4 &matrix) const;
  ShaderBuffer *get_shader_input_buffer(const InternalName *id) const;

  static void register_with_read_factory();

PUBLISHED:
  MAKE_PROPERTY(shader, get_shader);
  MAKE_PROPERTY(instance_count, get_instance_count);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:

  CPT(Shader) _shader;
  int         _shader_priority;
  bool        _auto_shader;
  bool        _has_shader;
  int         _flags;
  int         _has_flags;
  int         _instance_count;

  bool        _auto_normal_on;
  bool        _auto_glow_on;
  bool        _auto_gloss_on;
  bool        _auto_ramp_on;
  bool        _auto_shadow_on;

  // We don't keep a reference to the InternalName, since this is also already
  // stored on the ShaderInput object.
  typedef pmap<const InternalName *, ShaderInput> Inputs;
  Inputs _inputs;

  friend class Extension<NodePath>;
  friend class Extension<ShaderAttrib>;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ShaderAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 10, new ShaderAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};


#include "shaderAttrib.I"

#endif  // SHADERATTRIB_H
