// Filename: glTextureContext_src.h
// Created by:  drose (07Oct99)
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
#include "textureContext.h"

////////////////////////////////////////////////////////////////////
//       Class : GLTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(TextureContext) : public TextureContext {
public:
  INLINE CLP(TextureContext)(Texture *tex);

  // This is the GL "name" of the texture object.
  GLuint _index;

  // This is a GL texture priority.
  GLfloat _priority;

  // These are the parameters that we specified with the last
  // glTexImage2D() call.  If none of these have changed, we can
  // reload the texture image with a glTexSubImage2D().
  bool _already_applied;
  GLint _internal_format;
  GLsizei _width;
  GLsizei _height;
  GLsizei _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "TextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glTextureContext_src.I"

