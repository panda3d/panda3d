// Filename: chromium.TextureContext.h
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CRTEXTURECONTEXT_H
#define CRTEXTURECONTEXT_H

#include "pandabase.h"

#ifdef WIN32_VC
// Must include windows.h before gl.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif


#include <GL/gl.h>
// Chromium specific
#ifdef WIN32_VC // [
#define WINDOWS 1
#endif //]
#include "cr_glwrapper.h"
#include "cr_applications.h"
#include "cr_spu.h"
extern SPUDispatchTable chromium;

#include <textureContext.h>

////////////////////////////////////////////////////////////////////
//       Class : CRTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDACR CRTextureContext : public TextureContext {
public:
  INLINE CRTextureContext(Texture *tex);

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
    register_type(_type_handle, "CRTextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "crTextureContext.I"

#endif

