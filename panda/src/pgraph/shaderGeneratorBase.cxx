// Filename: shaderGeneratorBase.cxx
// Created by:  drose (05Nov08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "shaderGeneratorBase.h"

TypeHandle ShaderGeneratorBase::_type_handle;
ShaderGeneratorBase *ShaderGeneratorBase::_default_generator = NULL;

////////////////////////////////////////////////////////////////////
//     Function: ShaderGeneratorBase::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
ShaderGeneratorBase::
ShaderGeneratorBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGeneratorBase::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ShaderGeneratorBase::
~ShaderGeneratorBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGeneratorBase::get_default
//       Access: Published, Static
//  Description: Get a pointer to the default shader generator.
////////////////////////////////////////////////////////////////////
ShaderGeneratorBase *ShaderGeneratorBase::
get_default() {
  return _default_generator;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderGeneratorBase::set_default
//       Access: Published, Static
//  Description: Set the default shader generator.
////////////////////////////////////////////////////////////////////
void ShaderGeneratorBase::
set_default(ShaderGeneratorBase *generator) {
  if (generator != _default_generator) {
    if (_default_generator != (ShaderGeneratorBase *)NULL) {
      unref_delete(_default_generator);
    }
    _default_generator = generator;
    if (_default_generator != (ShaderGeneratorBase *)NULL) {
      _default_generator->ref();
    }
  }
}
