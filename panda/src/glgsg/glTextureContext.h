// Filename: glTextureContext.h
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef GLTEXTURECONTEXT_H
#define GLTEXTURECONTEXT_H

#include <pandabase.h>

#ifdef WIN32_VC
// Must include windows.h before gl.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <GL/gl.h>
#include <textureContext.h>

////////////////////////////////////////////////////////////////////
// 	 Class : GLTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL GLTextureContext : public TextureContext {
public:
  INLINE GLTextureContext(Texture *tex);

  // This is the GL "name" of the texture object.
  GLuint _index;

  // This is a GL texture priority.
  GLfloat _priority;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "GLTextureContext",
 		  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "glTextureContext.I"

#endif

