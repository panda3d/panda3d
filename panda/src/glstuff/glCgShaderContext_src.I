// Filename: glCgShaderContext_src.I
// Created by:  sshodhan (19Jul04)
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
#ifdef HAVE_CGGL

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::Constructor
//       Access: Published
//  Description: Use CLP(CgShaderContext)() to construct a new
//               CLP(CgShaderContext object.
////////////////////////////////////////////////////////////////////
INLINE CLP(CgShaderContext)::
CLP(CgShaderContext)(PT(CgShader) cg_shader) :
  CgShaderContext(cg_shader) {
  bool res = init_cg_shader_context();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 1f
//       Access: Published
//  Description: Send 1f values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const float value, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter1f(_cg_shader->_vertex_1f_params[pname], value);
  } else {
    cgGLSetParameter1f(_cg_shader->_fragment_1f_params[pname], value);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 1d
//       Access: Published
//  Description: Send 1d values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const double value, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter1f(_cg_shader->_vertex_1d_params[pname], value);
  } else {
    cgGLSetParameter1f(_cg_shader->_fragment_1d_params[pname], value);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 2f
//       Access: Published
//  Description: Send 2f values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const float value1, const float value2,
  bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter2f(_cg_shader->_vertex_2f_params[pname], value1, value2);
  } else {
    cgGLSetParameter2f(_cg_shader->_fragment_2f_params[pname], value1, value2);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 2d
//       Access: Published
//  Description: Send 2d values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const double value1, const double value2,
  bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter2d(_cg_shader->_vertex_2d_params[pname], value1, value2);
  } else {
    cgGLSetParameter2d(_cg_shader->_fragment_2d_params[pname], value1, value2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 3f
//       Access: Published
//  Description: Send 3f values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const float value1, const float value2,
  const float value3, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter3f(_cg_shader->_vertex_3f_params[pname], value1, value2, value3);
  } else {
    cgGLSetParameter3f(_cg_shader->_fragment_3f_params[pname], value1, value2, value3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 3d
//       Access: Published
//  Description: Send 3d values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const double value1, const double value2, 
  const double value3, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter3d(_cg_shader->_vertex_3d_params[pname], value1, value2, value3);
  } else {
    cgGLSetParameter3d(_cg_shader->_fragment_3d_params[pname], value1, value2, value3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 4f
//       Access: Published
//  Description: Send 4f values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const float value1, const float value2, 
  const float value3, const float value4, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter4f(_cg_shader->_vertex_4f_params[pname], value1, value2, value3, value4);
  } else {
    cgGLSetParameter4f(_cg_shader->_fragment_4f_params[pname], value1, value2, value3, value4);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param 4d
//       Access: Published
//  Description: Send 4d values to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, const double value1, const double value2,
  const double value3, const double value4, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLSetParameter4d(_cg_shader->_vertex_4d_params[pname], value1, value2, value3, value4);
  } else {
    cgGLSetParameter4d(_cg_shader->_fragment_4d_params[pname], value1, value2, value3, value4);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param texture
//       Access: Published
//  Description: Send texture to vertex or fragment shaders
//               Your shaders must declare these as uniform params
//               and can make use of them
//               What we actually send is the OpenGL texture index 
//               for a texture
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
set_param(const string &pname, Texture *t , bool vert_or_frag, GraphicsStateGuardianBase *gsg) {
  TextureContext *tc = t->prepare_now(gsg->get_prepared_objects(),gsg);
  CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
  if (vert_or_frag) {
    cgGLSetTextureParameter(_cg_shader->_vertex_texture_params[pname], gtc->_index);
  } else {
    cgGLSetTextureParameter(_cg_shader->_fragment_texture_params[pname], gtc->_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::enable_texture_param
//       Access: Published
//  Description: Enable a texture that has already been set
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
enable_texture_param(const string &pname, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLEnableTextureParameter(_cg_shader->_vertex_texture_params[pname]);
  } else {
    cgGLEnableTextureParameter(_cg_shader->_fragment_texture_params[pname]);
    
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::enable_texture_param
//       Access: Published
//  Description: Disable a texture 
////////////////////////////////////////////////////////////////////
INLINE void CLP(CgShaderContext)::
disable_texture_param(const string &pname, bool vert_or_frag) {
  if (vert_or_frag) {
    cgGLDisableTextureParameter(_cg_shader->_vertex_texture_params[pname]);
  } else {
    cgGLDisableTextureParameter(_cg_shader->_fragment_texture_params[pname]);
  }
}



#endif
