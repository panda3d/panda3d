// Filename: cgShader.cxx
// Created by:  sshodhan (10Jul04)
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

#ifdef HAVE_CG

#include "cgShader.h"
#include "config_effects.h"
TypeHandle CgShader::_type_handle;
#include <Cg/cg.h>


  
////////////////////////////////////////////////////////////////////
//     Function: CgShader::init_cg
//       Access: Published
//  Description: Create a cgContext. Note that this is completely
//               different from Panda's CgShaderContext and its
//               derived GLCgShaderContext (or Dx9CgShaderContext)
//               This is a Cg API specific context.
////////////////////////////////////////////////////////////////////
bool CgShader::
init_cg() {
  // Create a new context for our Cg Program(s)
  cgContext = cgCreateContext();
  
  if (cgContext == NULL) {
    cerr << "COULD NOT CREATE CG CONTEXT" << "\n" ;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CgShader::load_shaders
//       Access: Published
//  Description: this is called after the instantiating a 
//           CgShaderContext because we need the profiles
//           which are initiated by a GL or DX specific
//           CgShaderContext.
////////////////////////////////////////////////////////////////////
bool CgShader::
load_shaders(){
  
  // Load And Compile the shaders from file
  cgVertexProgram = cgCreateProgramFromFile(cgContext, 
    CG_SOURCE, _vertex_shader.c_str(), cgVertexProfile, "main", 0);
  cgFragmentProgram = cgCreateProgramFromFile(cgContext, 
    CG_SOURCE, _fragment_shader.c_str(), cgFragmentProfile, "main", 0);
  
  // Validate Success
  if (cgVertexProgram == NULL) {
    // We Need To Determine What Went Wrong
    CGerror Error = cgGetError();
    printf("VERTEX SHADER ERROR %s",cgGetErrorString(Error));
    return false;
  }

  if (cgFragmentProgram == NULL) {
    // We Need To Determine What Went Wrong
    CGerror Error = cgGetError();
    printf("PIXEL SHADER ERROR %s",cgGetErrorString(Error));
    return false;
  }


  // Now that our programs are loaded we can actually
  // access their handles with cgGetNamedParameter
  // The python programmer specifies parameter names and 
  // handle names when he calles add_param 
  // So lets go through all the handles and associate them with
  // CGparameter objects

  HANDLES::const_iterator param_iter; // Use this to go through all params
    
  // Matrix handles
  // Vertex
  for (param_iter = _vertex_matrix_handles.begin();
    param_iter !=_vertex_matrix_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_matrix_params.insert(vert_param);
  }
  // Fragment
   for (param_iter = _fragment_matrix_handles.begin();
    param_iter != _fragment_matrix_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_matrix_params.insert(frag_param);

  }


  // Texture handles
  // Vertex
  for (param_iter = _vertex_texture_handles.begin();
    param_iter != _vertex_texture_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_texture_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_texture_handles.begin();
    param_iter != _fragment_texture_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_texture_params.insert(frag_param);
  }

  // 1F handles
  // Vertex
  for (param_iter = _vertex_1f_handles.begin();
    param_iter != _vertex_1f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_1f_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_1f_handles.begin();
    param_iter != _fragment_1f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_1f_params.insert(frag_param);
  }


  // 2F handles
  // Vertex
  for (param_iter = _vertex_2f_handles.begin();
    param_iter != _vertex_2f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_2f_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_2f_handles.begin();
    param_iter != _fragment_2f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_2f_params.insert(frag_param);
  }

  // 3F handles
  // Vertex
  for (param_iter = _vertex_3f_handles.begin();
    param_iter != _vertex_3f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_3f_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_3f_handles.begin();
    param_iter != _fragment_3f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_3f_params.insert(frag_param);
  }

  // 4F handles
  // Vertex
  for (param_iter = _vertex_4f_handles.begin();
    param_iter != _vertex_4f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_4f_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_4f_handles.begin();
    param_iter != _fragment_4f_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_4f_params.insert(frag_param);
  }

  // 1D handles
  // Vertex
  for (param_iter = _vertex_1d_handles.begin();
    param_iter != _vertex_1d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_1d_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_1d_handles.begin();
    param_iter != _fragment_1d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_1d_params.insert(frag_param);
  }


  // 2D handles
  // Vertex
  for (param_iter = _vertex_2d_handles.begin();
    param_iter != _vertex_2d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_2d_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_2d_handles.begin();
    param_iter != _fragment_2d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_2d_params.insert(frag_param);
  }

  // 3D handles
  // Vertex
  for (param_iter = _vertex_3d_handles.begin();
    param_iter !=  _vertex_3d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_3d_params.insert(vert_param);
  }
  //Fragment
  for (param_iter =  _fragment_3d_handles.begin();
    param_iter !=  _fragment_3d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_3d_params.insert(frag_param);
  }

  // 4D handles
  // Vertex
  for (param_iter = _vertex_4d_handles.begin();
    param_iter != _vertex_4d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgVertexProgram, param_iter->second.c_str());
    CGPARAMETER::value_type vert_param(param_iter->first, p);
    _vertex_4d_params.insert(vert_param);
  }
  //Fragment
  for (param_iter = _fragment_4d_handles.begin();
    param_iter != _fragment_4d_handles.end(); param_iter++) {
    p = cgGetNamedParameter(cgFragmentProgram, param_iter->second.c_str());
    CGPARAMETER::value_type frag_param(param_iter->first, p);
    _fragment_4d_params.insert(frag_param);
  }
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: CgShader::Constructor
//       Access: Published
//  Description: Use to construct a new CgShader object.
//           Pass filenames for the vertex and fragment programs
////////////////////////////////////////////////////////////////////
CgShader::
CgShader(const string &name, const string &vertex_shader,
  const string &fragment_shader) {
  _name = name;
  _vertex_shader = vertex_shader;
  _fragment_shader = fragment_shader;
  bool res = init_cg();
}

  
////////////////////////////////////////////////////////////////////
//     Function: CgShader::Destructor
//       Access: Public
//  Description: We need to clean up the CGcontext
////////////////////////////////////////////////////////////////////
CgShader::
~CgShader() {
  cgDestroyContext(cgContext); 
}





////////////////////////////////////////////////////////////////////
//     Function: CgShader::add_param
//       Access: Published
//  Description: Add a new parameter
//           Specify its name, name in the shader program (handle name)
//           bind type, parameter type and whether its a vertex
//           or fragment parameter
////////////////////////////////////////////////////////////////////
void CgShader:: 
add_param(const string &pname, const string &handle_name,
  Param_Type t, Bind_Type b, bool vert_or_frag) {
  
/* We have only per-frame binding
  if (b == BONCE) {
    _bind_once.push_back(pname);
  } else if (b == BFRAME) {
    _bind_frame.push_back(pname);
  } else if (b == BVERTEX) {
    _bind_vertex.push_back(pname);
  }
*/

  // Add to the <parameter name, handle name> map 
  // Later, once we have loaded the cg programs
  // this handles map is traversed and associations
  // are made with the CGparameter objects
  // using the cgGetNamedParameter function
  if (t == P1F) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_1f_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_1f_handles.insert(param_handle);
    }
  }else if (t == P2F) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_2f_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_2f_handles.insert(param_handle);
    }
  }else if (t == P3F) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_3f_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_3f_handles.insert(param_handle);
    }
  }else if (t == P4F) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_4f_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_4f_handles.insert(param_handle);
    }
  }else if (t == P1D) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_1d_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_1d_handles.insert(param_handle);
    }
  }else if (t == P2D) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_2d_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_2d_handles.insert(param_handle);
    }
  }else if (t == P3D) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_3d_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_3d_handles.insert(param_handle);
    }
  }else if (t == P4D) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_4d_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_4d_handles.insert(param_handle);
    }
  } else if (t == PTEXTURE) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_texture_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_texture_handles.insert(param_handle);
    }    
  } else if (t == PMATRIX) {
    if (vert_or_frag) {
      HANDLES::value_type param_handle(pname, handle_name);
      _vertex_matrix_handles.insert(param_handle);
    } else {
      HANDLES::value_type param_handle(pname, handle_name);
      _fragment_matrix_handles.insert(param_handle);
    }    
  }
  
}

#endif  // HAVE_CG

