// Filename: spheretexShader.h
// Created by:  mike (09Jan97)
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
#ifndef SPHERETEXSHADER_H
#define SPHERETEXSHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "shader.h"
#include <texture.h>
#include <colorBlendTransition.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : SpheretexShader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER SpheretexShader : public Shader
{
  public:

    SpheretexShader(Texture* texture = NULL);
    ~SpheretexShader() { }

    virtual void config(void);
    virtual void apply(Node *node, const AllAttributesWrapper &init_state,
                       const AllTransitionsWrapper &net_trans,
                       GraphicsStateGuardian *gsg);

    INLINE void set_texture(Texture* texture) {
      _texture = texture;
      make_dirty();
    }
    INLINE Texture* get_texture(void) { return _texture; }
    INLINE void set_blend_mode(ColorBlendProperty::Mode mode) {
      _blend_mode = mode;
    }

  protected:

    PT(Texture)                 _texture;
    ColorBlendProperty::Mode    _blend_mode;

  public:

    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      Shader::init_type();
      register_type(_type_handle, "SpheretexShader",
                          Shader::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:

    static TypeHandle _type_handle;
};

#endif
