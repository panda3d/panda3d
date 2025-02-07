/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding.h
 * @author rdb
 * @date 2024-09-22
 */

#ifndef SHADERINPUTBINDING_H
#define SHADERINPUTBINDING_H

#include "pandabase.h"
#include "referenceCount.h"
#include "internalName.h"
#include "shaderEnums.h"
#include "small_vector.h"
#include "texture.h"
#include "shaderBuffer.h"

class GraphicsStateGuardian;
class LMatrix4f;

/**
 * Controls how a shader parameter's value is filled in by Panda at runtime.
 */
class EXPCL_PANDA_GOBJ ShaderInputBinding : public ReferenceCount {
protected:
  ShaderInputBinding() = default;

public:
  static ShaderInputBinding *make(ShaderEnums::SourceLanguage language,
                                  const InternalName *name, const ShaderType *type);

  template<class Callable>
  INLINE static ShaderInputBinding *make_data(int dep, Callable callable);
  template<class Callable>
  INLINE static ShaderInputBinding *make_texture(int dep, Callable callable);
  template<class Callable>
  INLINE static ShaderInputBinding *make_texture_image(int dep, Callable callable);

  virtual int get_state_dep() const;
  virtual void setup(Shader *shader);

  // Encapsulates parameters to pass to fetch_data et al, so we can add
  // members without having to modify their signature.
  struct State {
    GraphicsStateGuardian *gsg;
    LMatrix4f *matrix_cache;
  };

  virtual void fetch_data(const State &state, void *into,
                          bool packed = false) const;

  typedef uintptr_t ResourceId;
  virtual ResourceId get_resource_id(int index) const;
  virtual PT(Texture) fetch_texture(const State &state,
                                    ResourceId resource_id,
                                    SamplerState &sampler, int &view) const;
  virtual PT(Texture) fetch_texture_image(const State &state,
                                          ResourceId resource_id,
                                          ShaderType::Access &access,
                                          int &z, int &n) const;
  virtual PT(ShaderBuffer) fetch_shader_buffer(const State &state,
                                               ResourceId resource_id) const;

  // All the binders are defined in display, we provide this mechanism so that
  // we don't get a dependency on display here.
  typedef ShaderInputBinding *(*Binder)(const InternalName *name, const ShaderType *type);
  static void register_binder(ShaderEnums::SourceLanguage language, int sort, Binder binder);

  virtual bool is_model_to_apiclip_matrix() const { return false; }
  virtual bool is_color_scale() const { return false; }

private:
  struct BinderDef {
    ShaderEnums::SourceLanguage _lang;
    int _sort;
    Binder _func;

    bool operator < (const BinderDef &other) const {
      return _sort < other._sort;
    }
  };
  static small_vector<BinderDef, 2> _binders;
};

#include "shaderInputBinding.I"

#endif
