// Filename: glCgShaderContext_src.h
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
#include "cgShader.h"
#include "cgShaderContext.h"
#include "luse.h"
#include "pmap.h"
#include "texture.h"
#include "dcast.h"
#include <Cg/cgGL.h>


////////////////////////////////////////////////////////////////////
//       Class : GLCgShaderContext
// Description : The GL version of CgShaderContext.
//               This class binds and unbinds shaders, does the
//               actual API specific parameter passing based on 
//               the CgShader object pointed to in CgShaderContext
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(CgShaderContext) : public CgShaderContext {

public:
  CLP(CgShaderContext)(PT(CgShader) cg_shader);
  
  INLINE void set_param(const string &pname, const float value,
    bool vert_or_frag);

  INLINE void set_param(const string &pname, const double value, 
    bool vert_or_frag);

  INLINE void set_param(const string &pname, const float value1, 
    const float value2, bool vert_or_frag);

  INLINE void set_param(const string &pname, const double value1, 
    const double value2, bool vert_or_frag);

  INLINE void set_param(const string &pname, const float value1,
    const float value2, const float value3, bool vert_or_frag);

  INLINE void set_param(const string &pname, const double value1,
    const double value2, const double value3, bool vert_or_frag);

  INLINE void set_param(const string &pname, const float value1, 
    const float value2, const float value3, const float value4,
      bool vert_or_frag);

  INLINE void set_param(const string &pname, const double value1, 
      const double value2, const double value3, const double value4,
        bool vert_or_frag);

  INLINE void set_param(const string &pname, Texture *t, bool vert_or_frag, GraphicsStateGuardianBase *gsg);

  void set_param(const string &pname, CgShader::Matrix_Type m,
    CgShader::Transform_Type t, bool vert_or_frag);

  INLINE void enable_texture_param(const string &pname, bool vert_or_frag);
  
  INLINE void disable_texture_param(const string &pname, bool vert_or_frag);

  void load_shaders();
    
public:

  
  INLINE void bind(GraphicsStateGuardianBase *gsg);
  INLINE void un_bind();
  bool init_cg_shader_context();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CgShaderContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "CgShaderContext",
                  CgShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glCgShaderContext_src.I"

#endif

