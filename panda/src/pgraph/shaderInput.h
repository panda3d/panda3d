/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput.h
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 */

#ifndef SHADERINPUT_H
#define SHADERINPUT_H

#include "pandabase.h"
#include "pointerTo.h"
#include "internalName.h"
#include "paramValue.h"
#include "pta_float.h"
#include "pta_double.h"
#include "pta_LMatrix4.h"
#include "pta_LMatrix3.h"
#include "pta_LVecBase4.h"
#include "pta_LVecBase3.h"
#include "pta_LVecBase2.h"
#include "samplerState.h"
#include "shader.h"
#include "texture.h"
#include "shaderBuffer.h"
#include "extension.h"

/**
 * This is a small container class that can hold any one of the value types
 * that can be passed as input to a shader.
 */
class EXPCL_PANDA_PGRAPH ShaderInput {
PUBLISHED:
  // Used when binding texture images.
  enum AccessFlags {
    A_read    = 0x01,
    A_write   = 0x02,
    A_layered = 0x04,
  };

  static const ShaderInput &get_blank();
  INLINE explicit ShaderInput(CPT_InternalName name, int priority=0);

  EXTENSION(explicit ShaderInput(CPT_InternalName name, PyObject *value, int priority=0));

public:
  INLINE ShaderInput(CPT_InternalName name, Texture *tex, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, ParamValueBase *param, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, ShaderBuffer *buf, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_float &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase4f &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase3f &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase2f &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LMatrix4f &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LMatrix3f &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase4f &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase3f &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase2f &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LMatrix4f &mat, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LMatrix3f &mat, int priority=0);

  INLINE ShaderInput(CPT_InternalName name, const PTA_double &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase4d &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase3d &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase2d &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LMatrix4d &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LMatrix3d &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase4d &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase3d &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase2d &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LMatrix4d &mat, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LMatrix3d &mat, int priority=0);

  INLINE ShaderInput(CPT_InternalName name, const PTA_int &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase4i &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase3i &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const PTA_LVecBase2i &ptr, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase4i &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase3i &vec, int priority=0);
  INLINE ShaderInput(CPT_InternalName name, const LVecBase2i &vec, int priority=0);

  ShaderInput(CPT_InternalName name, const NodePath &np, int priority=0);

PUBLISHED:
  explicit ShaderInput(CPT_InternalName name, Texture *tex, bool read, bool write, int z=-1, int n=0, int priority=0);
  explicit ShaderInput(CPT_InternalName name, Texture *tex, const SamplerState &sampler, int priority=0);

  enum ShaderInputType {
    M_invalid = 0,
    M_texture,
    M_nodepath,
    M_vector,
    M_numeric,
    M_texture_sampler,
    M_param,
    M_texture_image,
    M_buffer,
  };

  INLINE operator bool() const;
  INLINE bool operator == (const ShaderInput &other) const;
  INLINE bool operator != (const ShaderInput &other) const;
  INLINE bool operator < (const ShaderInput &other) const;

  size_t add_hash(size_t hash) const;

  INLINE const InternalName *get_name() const;

  INLINE int get_value_type() const;
  INLINE int get_priority() const;
  INLINE const LVecBase4 &get_vector() const;
  INLINE const Shader::ShaderPtrData &get_ptr() const;

  NodePath get_nodepath() const;
  Texture *get_texture() const;
  const SamplerState &get_sampler() const;

public:
  ShaderInput() = default;

  INLINE ParamValueBase *get_param() const;
  INLINE TypedWritableReferenceCount *get_value() const;

  static void register_with_read_factory();

private:
  LVecBase4 _stored_vector;
  Shader::ShaderPtrData _stored_ptr;
  CPT_InternalName _name;
  PT(TypedWritableReferenceCount) _value;
  int _priority;
  int _type;

  friend class ShaderAttrib;
  friend class Extension<ShaderInput>;
};

#include "shaderInput.I"

#endif  // SHADERINPUT_H
