// Filename: shader.cxx
// Created by: jyelon (01Sep05)
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

#include "shader.h"
#include "virtualFileSystem.h"

TypeHandle Shader::_type_handle;
Shader::LoadTable Shader::_load_table;
Shader::MakeTable Shader::_make_table;

////////////////////////////////////////////////////////////////////
//     Function: Shader::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
Shader::
Shader() {
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::Destructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
Shader::
~Shader() {
  if (_loaded) {
    LoadTableKey key(_file, _preprocessor);
    _load_table.erase(key);
  } else {
    MakeTableKey key(_body, _preprocessor);
    _make_table.erase(key);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::load
//       Access: Published, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(Shader) Shader::
load(const string &file, int preprocessor) {
  return load(Filename(file), preprocessor);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::load
//       Access: Published, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(Shader) Shader::
load(const Filename &file, int preprocessor) {
  LoadTableKey key(file, preprocessor);
  LoadTable::const_iterator i = _load_table.find(key);
  if (i != _load_table.end()) {
    return i->second;
  }
  Shader *result = new Shader;
  result->_name = file;
  result->_file = file;
  result->_loaded = true;
  result->_preprocessor = preprocessor;
  result->_fixed_expansion = 0;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->read_file(file, result->_body)) {
    cerr << "Could not read shader file: " << file << "\n";
    result->_body = "";
  }
  _load_table[key] = result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::make
//       Access: Published, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(Shader) Shader::
make(const string &body, int preprocessor) {
  MakeTableKey key(body, preprocessor);
  MakeTable::const_iterator i = _make_table.find(key);
  if (i != _make_table.end()) {
    return i->second;
  }
  Shader *result = new Shader;
  result->_name = "generated shader";
  result->_file = "";
  result->_loaded = false;
  result->_preprocessor = preprocessor;
  result->_fixed_expansion = 0;
  result->_body = body;
  _make_table[key] = result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::macroexpand
//       Access: Public
//  Description: The eventual plan is that shaders will be run through
//               a macro preprocessor in order to generate the actual
//               shader text in Cg, GLSL, HLSL, or whatever.  The
//               macro preprocessor will be able to query the RenderState
//               and generate different shader code for different states.
//               
//               The macroexpansion of the shader is stored in an object
//               of class ShaderExpansion.  This is somewhat expensive
//               to generate, so the ShaderExpansion is cached inside the
//               the RenderState itself.  The ShaderExpansion contains
//               a map of ShaderContexts, each containing a compiled
//               copy of the shader's macroexpansion.
//
//               Any given shader might not contain any ifdefs that
//               depend on the RenderState.  If macroexpand determines
//               that a given shader is not state-sensitive, it can cache
//               the macroexpansion in the field "_fixed_expansion."
//
//               The preprocessing is usually done by a built-in macro
//               preprocessing function.  However, the user can write
//               his own preprocessor if he wishes to do so.  A user-
//               supplied preprocessor can generate arbitrary code ---
//               in fact, it doesn't need to look at the input string
//               if it does not wish to do so.
//
//               Currently, macroexpand is just a stub that returns an
//               expansion which is exactly equal to the input string.
////////////////////////////////////////////////////////////////////
PT(ShaderExpansion) Shader::
macroexpand(const RenderState *context) const {
  if (_fixed_expansion) {
    return _fixed_expansion;
  }
  // I am casting away the 'const' so as to be able
  // to write to this field.  This field is just a cache.
  ((Shader*)this)->_fixed_expansion = ShaderExpansion::make(_name,_body);
  return _fixed_expansion;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::register_with_read_factory
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void Shader::
register_with_read_factory() {
  // IMPLEMENT ME
}


