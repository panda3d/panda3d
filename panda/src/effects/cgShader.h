// Filename: cgShader.h
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

#ifndef CGSHADER_H
#define CGSHADER_H

#include "pandabase.h"

#ifdef HAVE_CG

#include "luse.h"
#include "pmap.h"
#include "typedWritableReferenceCount.h"
#include "texture.h"
#include "dcast.h"
#include <Cg/cg.h>


////////////////////////////////////////////////////////////////////
//       Class : CgShader
//      Summary: CgShader object initializes shader programs and
//               sets up the parameters and their values
//               A GL or DX CgShaderContext uses this object at bind time
//               to send parameters to a shader program.
//                  
//     Detailed: A CgShader object is created with a name and 
//               file names for a vertex program and fragment program
//               It is then passed to a CgShaderAttrib::make function
//               
//               Passing parameters to your shader programs is done
//               in two steps:
//               1) Add the param, specifying:
//                  a) Its name and corresponding handle in the Cg program
//                  b) Its type - matrix/ texture/ number(s)
//                  c) Its bind type: Right now everything is bound per frame
//                  d) Whether its for a vertex shader or a fragment shader
//               2) Set its value(s)
//
//               We store maps containing associations between
//               - parameter names and handle names
//               - parameter names and parameter objects (Cgparameter)
//               - parameter names and their actual values
//               It has different named maps for different types of params
//               There's also a fragment map and a vertex map for each
//               These maps are populated and then this object gets
//               pointed to from a CgShaderContext (right now only GL)
//               The Shadercontext does the actual "sending" of the params.
//                  
//               To make a cg shader variable get its values from a Panda 
//               program we define it as "uniform" in our cg shader program
//
//               We refer to the names of the variables in the .cg shader 
//               program file as "handles" 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX CgShader: public TypedWritableReferenceCount {

PUBLISHED:

  enum Bind_Type {
    BONCE,
    BFRAME,
    BVERTEX,
  };

  enum Param_Type {
    PMATRIX,
    PTEXTURE,
    P1F,
    P2F,
    P3F,
    P4F,
    P1D,
    P2D,
    P3D,
    P4D,
  };

// Cg allows us to pass combinatios any of these four matrices 
// with any of the four transforms enumerated below
  enum Matrix_Type {
      MTXMODELVIEW,
      MTXPROJECTION,
      MTXTEXTURE,
      MTXMODELVIEWPROJECTION,
  };

  enum Transform_Type {
      TRFIDENTITY,
      TRFTRANSPOSE,
      TRFINVERSE,
      TRFINVERSETRANSPOSE,
  };

  CgShader(const string &name, const string &v_s , const string &f_s);

  // First step for parameters: Add them and specify name, cg hande name, 
  // type, bind and vertex/fragment( 1 for vertex program 0 for fragment)
  void add_param(const string &pname, const string &handle_name,
    Param_Type t, Bind_Type b, bool vert_or_frag);
  
  // Overloaded set_param to be used based on your param type
  INLINE void set_param(const string &pname, Texture *t);
  INLINE void set_param(const string &pname, Matrix_Type m, Transform_Type t);
  INLINE void set_param(const string &pname, float p1f);
  INLINE void set_param(const string &pname, LVector2f p2f);
  INLINE void set_param(const string &pname, LVector3f p3f);
  INLINE void set_param(const string &pname, LVector4f p4f);
  INLINE void set_param(const string &pname, double p1d);
  INLINE void set_param(const string &pname, LVector2d p2d);
  INLINE void set_param(const string &pname, LVector3d p3d);
  INLINE void set_param(const string &pname, LVector4d p4d);
  
public:
  CGcontext cgContext;// A context to hold our Cg program(s)
  CGprogram cgVertexProgram;// A handle to the vertex shader .cg file 
  CGprogram cgFragmentProgram;// A handle to the pixel shader .cg file
  CGprofile cgVertexProfile;// Vertex profile... different features on various cards
  CGprofile cgFragmentProfile;// Profile for Pixel Shader
  CGparameter p; // Used to insert Cgparameter objects into the maps

// Matrix parameters need to know type of Matrix and its transform  
  typedef struct {
    Matrix_Type matrix;
    Transform_Type transform;
  }CGMATRIXDEF;

  typedef pmap<string, string> HANDLES;
  typedef pmap<string, CGparameter> CGPARAMETER;
  typedef pmap<string, PT(Texture) > CGTEXTURE;
  typedef pmap<string, CGMATRIXDEF> CGMATRIX;
  typedef pmap<string, float> CGPARAM1F;
  typedef pmap<string, LVector2f> CGPARAM2F;
  typedef pmap<string, LVector3f> CGPARAM3F;
  typedef pmap<string, LVector4f> CGPARAM4F;
  typedef pmap<string, double> CGPARAM1D;
  typedef pmap<string, LVector2d> CGPARAM2D;
  typedef pmap<string, LVector3d> CGPARAM3D;
  typedef pmap<string, LVector4d> CGPARAM4D;


  // add_param adds to these maps
  CGPARAMETER _vertex_1f_params;
  CGPARAMETER _fragment_1f_params;

  CGPARAMETER _vertex_2f_params;
  CGPARAMETER _fragment_2f_params;
 
  CGPARAMETER _vertex_3f_params;
  CGPARAMETER _fragment_3f_params;

  CGPARAMETER _vertex_4f_params;
  CGPARAMETER _fragment_4f_params;
  
  CGPARAMETER _vertex_1d_params;
  CGPARAMETER _fragment_1d_params;

  CGPARAMETER _vertex_2d_params;
  CGPARAMETER _fragment_2d_params;

  CGPARAMETER _vertex_3d_params;
  CGPARAMETER _fragment_3d_params;

  CGPARAMETER _vertex_4d_params;
  CGPARAMETER _fragment_4d_params;

  CGPARAMETER _vertex_matrix_params;
  CGPARAMETER _fragment_matrix_params;
  
  CGPARAMETER _vertex_texture_params; 
  CGPARAMETER _fragment_texture_params;
  
  HANDLES _vertex_1f_handles;
  HANDLES _fragment_1f_handles;

  HANDLES _vertex_2f_handles;
  HANDLES _fragment_2f_handles;
  
  HANDLES _vertex_3f_handles;
  HANDLES _fragment_3f_handles;

  HANDLES _vertex_4f_handles;
  HANDLES _fragment_4f_handles;
  
  HANDLES _vertex_1d_handles;
  HANDLES _fragment_1d_handles;

  HANDLES _vertex_2d_handles;
  HANDLES _fragment_2d_handles;

  HANDLES _vertex_3d_handles;
  HANDLES _fragment_3d_handles;

  HANDLES _vertex_4d_handles;
  HANDLES _fragment_4d_handles;

  HANDLES _vertex_matrix_handles;
  HANDLES _fragment_matrix_handles;
  
  HANDLES _vertex_texture_handles; 
  HANDLES _fragment_texture_handles;
  
  
  // set_param adds to these maps
  CGTEXTURE _cg_textures;
  CGMATRIX _cg_matrices;
  CGPARAM1F _cg_params1f;
  CGPARAM2F _cg_params2f;
  CGPARAM3F _cg_params3f;
  CGPARAM4F _cg_params4f;
  CGPARAM1D _cg_params1d;
  CGPARAM2D _cg_params2d;
  CGPARAM3D _cg_params3d;
  CGPARAM4D _cg_params4d;  

  INLINE string get_name() const;
  
  
  bool init_cg();// This initializes a CGcontext
  
  // After a CgShaderContext (GL or DX) has been created
  // this function loads vertex and fragment programs
  // from the hard disk.
  // Only after this is done can we associate our
  // Cgparameter objects with the handles
  // The CgShaderContext then loads these programs
  // into the GPU
  bool load_shaders();
  
  ~CgShader();

  string _name;
  string _vertex_shader;
  string _fragment_shader;
  

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "CgShader",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cgShader.I"

#endif  // HAVE_CG

#endif


