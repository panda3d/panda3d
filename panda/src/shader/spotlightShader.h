// Filename: spotlightShader.h
// Created by:  mike (09Jan97)
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
#ifndef SPOTLIGHTSHADER_H
#define SPOTLIGHTSHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "projtexShader.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : SpotlightShader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER SpotlightShader : public ProjtexShader
{
public:
  SpotlightShader(int size = 256, float radius = 0.7);

  virtual void config(void);
  // SpotlightShader uses ProjtexShader::apply()

protected:
  float                 _radius;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ProjtexShader::init_type();
    register_type(_type_handle, "SpotlightShader",
                    ProjtexShader::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

#endif
