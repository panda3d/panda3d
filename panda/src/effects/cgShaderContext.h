// Filename: cgShaderContext.h
// Created by:  sshodhan (20Jul04)
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

#ifndef CGSHADERCONTEXT_H
#define CGSHADERCONTEXT_H

#include "pandabase.h"

#ifdef HAVE_CG

#include "cgShader.h"
#include "luse.h"
#include "pmap.h"
#include "texture.h"
#include "dcast.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"
#include <Cg/cgGL.h>

////////////////////////////////////////////////////////////////////
//       Class : CgShaderContext
// Description : This is a base class to be derived by GLCgShaderContext
//               DX9CgShaderContext and DX8CgShaderContext
//               The CgShaderContexts will handle all the
//               DX/GL API specific shader calls
//               The CgShader object will do all the API generic stuff
//               The CgShaderAttrib will be made with a CgShader object
//               The gsgs will map CgShader objects to CgShaderContext
//               objects. The CgShader objects will be passed through
//               the CgShaderAttrib objects and will contain parameter
//               info.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX CgShaderContext: public TypedReferenceCount {

PUBLISHED:


  INLINE CgShaderContext(PT(CgShader) cg_shader);

public:

  // Overloaded functions to do cgGLSetParameter* on various kinds of parameters
  INLINE virtual void set_param(const string &pname, const float value,
    bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const double value, 
    bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const float value1, 
    const float value2, bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const double value1, 
    const double value2, bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const float value1,
    const float value2, const float value3, bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const double value1,
    const double value2, const double value3, bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const float value1, 
    const float value2, const float value3, const float value4,
      bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, const double value1, 
      const double value2, const double value3, const double value4,
        bool vert_or_frag);

  INLINE virtual void set_param(const string &pname, Texture *t, bool vert_or_frag, GraphicsStateGuardianBase *gsg);

  virtual void set_param(const string &pname, CgShader::Matrix_Type m,
    CgShader::Transform_Type t, bool vert_or_frag);


  INLINE virtual void enable_texture_param(const string &pname, bool vert_or_frag);
  
  INLINE virtual void disable_texture_param(const string &pname, bool vert_or_frag);
    
  // Bind and unbind shaders and call the set_param functions 
  // based on the CgShader object 
  INLINE virtual void bind(GraphicsStateGuardianBase *gsg);
  INLINE virtual void un_bind();

  virtual bool init_cg_shader_context();

  // Store a pointer to the CgShader object which contains all the parameter info
  PT(CgShader) _cg_shader;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CgShaderContext",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cgShaderContext.I"

#endif  // HAVE_CG

#endif


