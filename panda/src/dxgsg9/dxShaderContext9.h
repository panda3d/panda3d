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
  void issue_transform(GSG *gsg);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(DXShaderContext9 *prev, GSG *gsg);

  LPDIRECT3DVERTEXDECLARATION9 get_vertex_declaration(GSG *gsg, const GeomVertexFormat *format);

private:
  bool r_query_constants(Shader::Stage stage, BYTE *offset,
                         D3DXSHADER_TYPEINFO &typeinfo, int &loc,
                         int reg_set, int &reg_idx, int reg_end);

  IDirect3DVertexShader9 *_vertex_shader = nullptr;
  IDirect3DPixelShader9 *_pixel_shader = nullptr;

  struct ConstantRegister {
    int vreg = -1;
    int freg = -1;
    D3DXREGISTER_SET set;
    UINT count = 0;
  };

  bool _uses_vertex_color = false;
  int _half_pixel_register = -1;
  pvector<ConstantRegister> _register_map;

  pmap<CPT(GeomVertexFormat), LPDIRECT3DVERTEXDECLARATION9> _vertex_declarations;

  int _frame_number = -1;
  LMatrix4 *_mat_part_cache = nullptr;

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
