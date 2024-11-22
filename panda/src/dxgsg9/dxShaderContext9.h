/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxShaderContext9.h
 * @author aignacio
 * @date 2006-01
 */

#ifndef DXSHADERCONTEXT9_H
#define DXSHADERCONTEXT9_H

#include "dtool_config.h"
#include "pandabase.h"
#include "string_utils.h"
#include "internalName.h"
#include "shader.h"
#include "shaderContext.h"

class VertexElementArray;
class DXGraphicsStateGuardian9;

/**
 * Shader back-end for DX9.  We only support SPIR-V shaders, which are
 * transpiled to HLSL by spirv-cross, then compiled using D3DXCompileShader.
 */
class EXPCL_PANDADX DXShaderContext9 : public ShaderContext {
public:
  typedef DXGraphicsStateGuardian9 GSG;

  DXShaderContext9(Shader *s, GSG *gsg);
  ~DXShaderContext9();

  bool compile_module(const ShaderModule *module, DWORD *&data);
  bool query_constants(const ShaderModule *module, DWORD *data);

  INLINE bool uses_vertex_color();

  INLINE bool valid(GSG *gsg);
  bool bind(GSG *gsg);
  void unbind(GSG *gsg);
  void issue_parameters(GSG *gsg, int altered);
  void update_tables(GSG *gsg, const GeomVertexDataPipelineReader *data_reader);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(DXShaderContext9 *prev, GSG *gsg);

  LPDIRECT3DVERTEXDECLARATION9 get_vertex_declaration(GSG *gsg, const GeomVertexFormat *format, BitMask32 &used_streams);

private:
  bool r_query_constants(Shader::Stage stage, const Shader::Parameter &param,
                         const ShaderType *type, size_t offset,
                         BYTE *table_data, D3DXSHADER_TYPEINFO &typeinfo,
                         int reg_set, int &reg_idx, int reg_end);
  bool r_query_resources(Shader::Stage stage, const Shader::Parameter &param,
                         const ShaderType *type, const char *path, int index,
                         int reg_set, int &reg_idx, int reg_end);

  IDirect3DVertexShader9 *_vertex_shader = nullptr;
  IDirect3DPixelShader9 *_pixel_shader = nullptr;

  struct Binding {
    PT(ShaderInputBinding) _binding;
    size_t _offset;
    int _dep;
  };
  pvector<Binding> _data_bindings;
  size_t _scratch_space_size = 0;

  struct ConstantRegister {
    D3DXREGISTER_SET set;
    bool convert = false;
    int reg = -1;
    UINT count = 0;
    int dep = 0;
    size_t offset = 0;
  };

  int _half_pixel_register = -1;
  pvector<ConstantRegister> _vertex_constants;
  pvector<ConstantRegister> _pixel_constants;
  int _constant_deps = 0;

  struct TextureRegister {
    UINT unit;
    PT(ShaderInputBinding) binding;
    uint64_t resource_id;
    int size_vreg = -1;
    int size_freg = -1;
  };
  pvector<TextureRegister> _textures;

  pmap<CPT(GeomVertexFormat), std::pair<LPDIRECT3DVERTEXDECLARATION9, BitMask32> > _vertex_declarations;

private:
  void release_resources(void);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "DXShaderContext9",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxShaderContext9.I"

#endif
