// Filename: dxShaderContext9.h
// Created by: aignacio (Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
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

#ifndef DXSHADERCONTEXT9_H
#define DXSHADERCONTEXT9_H

#include "dtool_config.h"
#include "pandabase.h"
#ifdef HAVE_CGDX9
#include "Cg/cg.h"
#include "Cg/cgGL.h"
#include "Cg/cgD3D9.h"
#endif
#include "string_utils.h"
#include "internalName.h"
#include "shaderExpansion.h"
#include "shaderContext.h"


#define CLP(name) DX##name##9
#define CLASSPREFIX_QUOTED "DX"


class VertexElementArray;
class CLP(GraphicsStateGuardian);

typedef struct
{
  int vertex_shader;
  int total_constant_descriptions;
  D3DXCONSTANT_DESC *constant_description_array;
}
DX_PARAMETER;

typedef struct
{
  int state;
  union
  {
    DIRECT_3D_VERTEX_SHADER direct_3d_vertex_shader;
    DIRECT_3D_PIXEL_SHADER direct_3d_pixel_shader;
  };
  LPD3DXCONSTANTTABLE constant_table;
  D3DXCONSTANTTABLE_DESC constant_table_description;

  int total_semantics;
  D3DXSEMANTIC *semantic_array;
}
DIRECT_3D_SHADER;

////////////////////////////////////////////////////////////////////
//       Class : DXShaderContext9
// Description : xyz
////////////////////////////////////////////////////////////////////

typedef CLP(GraphicsStateGuardian) GSG;

class EXPCL_PANDADX CLP(ShaderContext): public ShaderContext {
public:

  CLP(ShaderContext)(ShaderExpansion *s, GSG *gsg);
  ~CLP(ShaderContext)();

  INLINE bool valid(GSG *gsg);
  bool bind(GSG *gsg);
  void unbind(GSG *gsg);

  void issue_parameters(GSG *gsg, bool altered);
  void issue_transform(GSG *gsg);
  void disable_shader_vertex_arrays(GSG *gsg);
  void update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg);

  int _vertex_size;
  class VertexElementArray *_vertex_element_array;

  // FOR DEBUGGING
  string _name;

  bool _state;
  bool _cg_shader;
  bool _transpose_matrix;

private:

#ifdef HAVE_CGDX9
  struct ShaderAutoBind {
    CGparameter parameter;
    int value;
    DX_PARAMETER *dx_parameter;
  };
  struct ShaderArgBind {
    CGparameter parameter;
    PT(InternalName) name;
    DX_PARAMETER *dx_parameter;
  };
  struct ShaderTexBind {
    CGparameter parameter;
    PT(InternalName) name;
    int stage;
    int desiredtype;
    PT(InternalName) suffix;
    DX_PARAMETER *dx_parameter;
  };
  struct ShaderTransBind {
    CGparameter parameter;
    PT(InternalName) src_name;
    PT(InternalName) rel_name;
    int trans_piece;
    DX_PARAMETER *dx_parameter;
  };
  struct ShaderVarying {
    CGparameter parameter;
    PT(InternalName) name;
    int append_uv;
    DX_PARAMETER *dx_parameter;
  };

//  CGcontext _cg_context;
  CGprofile _cg_profile[2];
  CGprogram _cg_program[2];

  void report_cg_compile_errors(const string &file, CGcontext ctx);

  bool try_cg_compile(ShaderExpansion *s, GSG *gsg);
  void suggest_cg_profile(const string &vpro, const string &fpro);
  CGprofile parse_cg_profile(const string &id, bool vertex);
#endif

public:

  DIRECT_3D_SHADER _direct_3d_vertex_shader;
  DIRECT_3D_SHADER _direct_3d_pixel_shader;
  int _total_dx_parameters;
  DX_PARAMETER *_dx_parameter_array;

private:

  void release_resources(void);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "ShaderContext",
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
