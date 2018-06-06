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

// Caution: adding HLSL support is going to be tricky, as the parsing needs to
// be done in the cull thread, which cannot use the DX API.  - Josh
//
//
// typedef struct
// {
//   int vertex_shader;
//   int total_constant_descriptions;
//   D3DXCONSTANT_DESC *constant_description_array;
// }
// DX_PARAMETER;
//
// typedef struct
// {
//   int state;
//   union
//   {
//     DIRECT_3D_VERTEX_SHADER direct_3d_vertex_shader;
//     DIRECT_3D_PIXEL_SHADER direct_3d_pixel_shader;
//   };
//   LPD3DXCONSTANTTABLE constant_table;
//   D3DXCONSTANTTABLE_DESC constant_table_description;
//
//   int total_semantics;
//   D3DXSEMANTIC *semantic_array;
// }
// DIRECT_3D_SHADER;

/**
 * xyz
 */
class EXPCL_PANDADX DXShaderContext9 : public ShaderContext {
public:
  typedef DXGraphicsStateGuardian9 GSG;

  DXShaderContext9(Shader *s, GSG *gsg);
  ~DXShaderContext9();

  INLINE bool valid(GSG *gsg);
  bool bind(GSG *gsg);
  void unbind(GSG *gsg);
  void issue_parameters(GSG *gsg, int altered);
  void issue_transform(GSG *gsg);
  void disable_shader_vertex_arrays(GSG *gsg);
  bool update_shader_vertex_arrays(DXShaderContext9 *prev, GSG *gsg,
                                   bool force);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(DXShaderContext9 *prev, GSG *gsg);

  class VertexElementArray* _vertex_element_array;
  LPDIRECT3DVERTEXDECLARATION9 _vertex_declaration;

  int _num_bound_streams;

  // FOR DEBUGGING
  std::string _name;

private:
#ifdef HAVE_CG
  CGprogram _cg_program;
  pvector <CGparameter> _cg_parameter_map;
#endif

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
