// Filename: textureContext.h
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

#ifndef TEXTURECONTEXT_H
#define TEXTURECONTEXT_H

#include "pandabase.h"

#include "savedContext.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureContext
// Description : This is a special class object that holds all the
//               information returned by a particular GSG to indicate
//               the texture's internal context identifier.
//
//               Textures typically have an immediate-mode and a
//               retained-mode operation.  When using textures in
//               retained-mode (in response to Texture::prepare()),
//               the GSG will create some internal handle for the
//               texture and store it here.  The texture stores all of
//               these handles internally.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureContext : public SavedContext {
public:
  INLINE TextureContext(Texture *tex);

  virtual size_t estimate_texture_memory();

  // This cannot be a PT(Texture), because the texture and the GSG
  // both own their TextureContexts!  That would create a circular
  // reference count.
  Texture *_texture;

  INLINE void mark_dirty(int flags_to_set);
  INLINE void clear_dirty_flags(int flags_to_clear = ~0);
  INLINE int get_dirty_flags() const;

private:
  int _dirty_flags;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "TextureContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "textureContext.I"

#endif

