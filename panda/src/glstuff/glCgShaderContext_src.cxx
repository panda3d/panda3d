// Filename: glCgShaderContext_src.cxx
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
#include "pandabase.h"

#ifdef HAVE_CGGL

#include <Cg/cgGL.h>

TypeHandle CLP(CgShaderContext)::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::init_cg
//       Access: Published
//  Description: Create the profiles : just go for the best available
//               
////////////////////////////////////////////////////////////////////
bool CLP(CgShaderContext)::
init_cg_shader_context() {

  // Get The Latest GL Vertex Profile
  _cg_shader->cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  // Get The Latest GL Fragment Profile
  _cg_shader->cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

  // Validate Our Profile Determination 
  if (_cg_shader->cgVertexProfile == CG_PROFILE_UNKNOWN) {
    cerr << "VERTEX PROFILE UNKNOWN" << endl;
    return false;
  }
  if (_cg_shader->cgFragmentProfile == CG_PROFILE_UNKNOWN) {
    cerr << "FRAGMENT PROFILE UNKNOWN" << endl;
    return false;
  }

  cgGLSetOptimalOptions(_cg_shader->cgVertexProfile);// Set The Current Profile
  cgGLSetOptimalOptions(_cg_shader->cgFragmentProfile);// Set The Current Profile


  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::load_shaders
//       Access: Published
//  Description: Download the programs to the GPU
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
load_shaders() {
  // Load The Programs
  cgGLLoadProgram(_cg_shader->cgVertexProgram);
  cgGLLoadProgram(_cg_shader->cgFragmentProgram);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::bind
//       Access: Published
//  Description: We support Textures, Matrices and Numbers as 
//               parameters to the shaders. We iterate through
//               maps which have <name, CgParameter> tuples
//               There are two maps..one for vertex shader params
//               and one for fragment shader params.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
bind(GraphicsStateGuardianBase *gsg) {
  CgShader::CGPARAMETER::const_iterator param_iter; // Use this to go through all params
  
  // Matrix params
  // Vertex
  for (param_iter = _cg_shader->_vertex_matrix_params.begin();
    param_iter != _cg_shader->_vertex_matrix_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_matrices[param_iter->first].matrix, 
       _cg_shader->_cg_matrices[param_iter->first].transform, 1);
  }
  // Fragment
   for (param_iter = _cg_shader->_fragment_matrix_params.begin();
    param_iter != _cg_shader->_fragment_matrix_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_matrices[param_iter->first].matrix, 
       _cg_shader->_cg_matrices[param_iter->first].transform, 0);
  }

  // BIND THE FRAGMENT AND SHADER PROGRAMS (after Matrices are loaded)  
  cgGLEnableProfile(_cg_shader->cgVertexProfile);
  cgGLBindProgram(_cg_shader->cgVertexProgram);
  cgGLEnableProfile(_cg_shader->cgFragmentProfile);
  cgGLBindProgram(_cg_shader->cgFragmentProgram);
   
  // Texture params
  // Vertex
  for (param_iter = _cg_shader->_vertex_texture_params.begin();
    param_iter != _cg_shader->_vertex_texture_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_textures[param_iter->first] , 1, gsg);
    enable_texture_param(param_iter->first, 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_texture_params.begin();
    param_iter != _cg_shader->_fragment_texture_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_textures[param_iter->first] , 0, gsg);
    enable_texture_param(param_iter->first, 0);
  }

  // 1F params
  // Vertex
  for (param_iter = _cg_shader->_vertex_1f_params.begin();
    param_iter != _cg_shader->_vertex_1f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params1f[param_iter->first] , 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_1f_params.begin();
    param_iter != _cg_shader->_fragment_1f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params1f[param_iter->first] , 0);
  }


  // 2F params
  // Vertex
  for (param_iter = _cg_shader->_vertex_2f_params.begin();
    param_iter != _cg_shader->_vertex_2f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params2f[param_iter->first][0],
       _cg_shader->_cg_params2f[param_iter->first][1], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_2f_params.begin();
    param_iter != _cg_shader->_fragment_2f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params2f[param_iter->first][0],
       _cg_shader->_cg_params2f[param_iter->first][1], 0);
  }

  // 3F params
  // Vertex
  for (param_iter = _cg_shader->_vertex_3f_params.begin();
    param_iter != _cg_shader->_vertex_3f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params3f[param_iter->first][0],
       _cg_shader->_cg_params3f[param_iter->first][1], 
         _cg_shader->_cg_params3f[param_iter->first][2], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_3f_params.begin();
    param_iter != _cg_shader->_fragment_3f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params3f[param_iter->first][0],
       _cg_shader->_cg_params3f[param_iter->first][1],
         _cg_shader->_cg_params3f[param_iter->first][2], 0);
  }

  // 4F params
  // Vertex
  for (param_iter = _cg_shader->_vertex_4f_params.begin();
    param_iter != _cg_shader->_vertex_4f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params4f[param_iter->first][0],
       _cg_shader->_cg_params4f[param_iter->first][1], 
         _cg_shader->_cg_params4f[param_iter->first][2],
           _cg_shader->_cg_params4f[param_iter->first][3], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_4f_params.begin();
    param_iter != _cg_shader->_fragment_4f_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params4f[param_iter->first][0],
       _cg_shader->_cg_params4f[param_iter->first][1],
         _cg_shader->_cg_params4f[param_iter->first][2],
           _cg_shader->_cg_params4f[param_iter->first][3],0);
  }

  // 1D params
  // Vertex
  for (param_iter = _cg_shader->_vertex_1d_params.begin();
    param_iter != _cg_shader->_vertex_1d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params1d[param_iter->first] , 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_1d_params.begin();
    param_iter != _cg_shader->_fragment_1d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params1d[param_iter->first] , 0);
  }


  // 2D params
  // Vertex
  for (param_iter = _cg_shader->_vertex_2d_params.begin();
    param_iter != _cg_shader->_vertex_2d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params2d[param_iter->first][0],
       _cg_shader->_cg_params2d[param_iter->first][1], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_2d_params.begin();
    param_iter != _cg_shader->_fragment_2d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params2d[param_iter->first][0],
       _cg_shader->_cg_params2d[param_iter->first][1], 0);
  }

  // 3D params
  // Vertex
  for (param_iter = _cg_shader->_vertex_3d_params.begin();
    param_iter != _cg_shader->_vertex_3d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params3d[param_iter->first][0],
       _cg_shader->_cg_params3d[param_iter->first][1], 
         _cg_shader->_cg_params3d[param_iter->first][2], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_3d_params.begin();
    param_iter != _cg_shader->_fragment_3d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params3d[param_iter->first][0],
       _cg_shader->_cg_params3d[param_iter->first][1],
         _cg_shader->_cg_params3d[param_iter->first][2], 0);
  }

  // 4D params
  // Vertex
  for (param_iter = _cg_shader->_vertex_4d_params.begin();
    param_iter != _cg_shader->_vertex_4d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params4d[param_iter->first][0],
       _cg_shader->_cg_params4d[param_iter->first][1], 
         _cg_shader->_cg_params4d[param_iter->first][2],
           _cg_shader->_cg_params4d[param_iter->first][3], 1);
  }
  //Fragment
  for (param_iter = _cg_shader->_fragment_4d_params.begin();
    param_iter != _cg_shader->_fragment_4d_params.end(); param_iter++) {
    set_param(param_iter->first, _cg_shader->_cg_params4d[param_iter->first][0],
       _cg_shader->_cg_params4d[param_iter->first][1],
         _cg_shader->_cg_params4d[param_iter->first][2],
           _cg_shader->_cg_params4d[param_iter->first][3],0);
  }


}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::unbind
//       Access: Published
//  Description: Disable textures and shaders
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
un_bind(){

  CgShader::CGPARAMETER::const_iterator param_iter; 
  for (param_iter = _cg_shader->_vertex_texture_params.begin(); 
    param_iter != _cg_shader->_vertex_texture_params.end(); param_iter++) {
    disable_texture_param(param_iter->first, 1);
  }

  for (param_iter = _cg_shader->_fragment_texture_params.begin(); 
    param_iter != _cg_shader->_fragment_texture_params.end(); param_iter++) {
    disable_texture_param(param_iter->first, 0);
  }


  cgGLDisableProfile(_cg_shader->cgVertexProfile);// Disable Our Vertex Profile
  cgGLDisableProfile(_cg_shader->cgFragmentProfile);// Disable Our Fragment Profile	
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(CgShaderContext)::set_param Matrix
//       Access: Published
//  Description: Select a matrix type and a transform type
//               Matrices you can send to your shaders are:
//                M_MODELVIEW,M_PROJECTION,M_TEXTURE,M_MODELVIEW_PROJECTION,
//               and they can have th transforms:
//                T_IDENTITY,T_TRANSPOSE,T_INVERSE,T_INVERSE_TRANSPOSE,
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
set_param(const string &pname, CgShader::Matrix_Type m, CgShader::Transform_Type t,
  bool vert_or_frag) {
  // MODELVIEW BEGINS
  if (m == M_MODELVIEW) {
    if (t == T_IDENTITY) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname],
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname],
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
      }
    } else if (t == T_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      }
    } else if (t == T_INVERSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE);
      }
    } else if (t == T_INVERSE_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      }
    }
  // MODELVIEW ENDS
  // PROJECTION BEGINS
  } else if (m == M_PROJECTION) {
    if (t == T_IDENTITY) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname],
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
      }
    } else if (t == T_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      }
    } else if (t == T_INVERSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);
      }
    } else if (t == T_INVERSE_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      }
    }
  // PROJECTION ENDS
  // TEXTURE BEGINS
  } else if (m == M_TEXTURE) {
    if (t == T_IDENTITY) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_IDENTITY);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_IDENTITY);
      }
    } else if (t == T_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      }
    } else if (t == T_INVERSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_INVERSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_INVERSE);
      }
    } else if (t == T_INVERSE_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_TEXTURE_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      }
    }
  // TEXTURE ENDS
  // MODELVIEWPROJECTION BEGINS
  } else if (m == M_MODELVIEW_PROJECTION) {
    if (t == T_IDENTITY) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
      }
    } else if (t == T_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
      }
    } else if (t == T_INVERSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);
      }
    } else if (t == T_INVERSE_TRANSPOSE) {
      if (vert_or_frag) {
        cgGLSetStateMatrixParameter(_cg_shader->_vertex_matrix_params[pname], 
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      } else {
        cgGLSetStateMatrixParameter(_cg_shader->_fragment_matrix_params[pname],
          CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);
      }
    }
  }
  // MODELVIEWPROJECTION ENDS
}

#endif
