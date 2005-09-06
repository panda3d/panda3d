// Filename: shaderMode.cxx
// Created by:  jyelon (01Sep05)
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
#include "shader.h"
#include "shaderMode.h"
#include "virtualFileSystem.h"

TypeHandle ShaderMode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::load
//       Access: Published
//  Description: Obtain the shader associated with the specified 
//               file and construct a ShaderMode for that shader.
////////////////////////////////////////////////////////////////////
PT(ShaderMode) ShaderMode::
load(const Filename &file)
{
  string text;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->read_file(file, text)) {
    cerr << "Could not read " << file << "\n";
    return new ShaderMode(new Shader("", file));
  }
  return new ShaderMode(new Shader(text, file));
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::load
//       Access: Published
//  Description: Obtain the shader associated with the specified 
//               file and construct a ShaderMode for that shader.
////////////////////////////////////////////////////////////////////
PT(ShaderMode) ShaderMode::
load(const string &file)
{
  Filename fn(file);
  return load(fn);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::make
//       Access: Published
//  Description: Obtain the shader associated with the specified
//               text and construct a ShaderMode for that shader.
////////////////////////////////////////////////////////////////////
PT(ShaderMode) ShaderMode::
make(const string &text)
{
  return new ShaderMode(new Shader(text, ""));
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::Constructor
//       Access: Published
//  Description: Use to construct a new shader instance.
////////////////////////////////////////////////////////////////////
ShaderMode::
ShaderMode(Shader *shader) {
  _shader = shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::Destructor
//       Access: Public
//  Description: delete a shader
////////////////////////////////////////////////////////////////////
ShaderMode::
~ShaderMode() {
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::mod_param
//  Access: Private
//  Description: To set a parameter, you must first access it and
//               then clear it.  This function accesses then clears
//               the storage for a parameter, prior to setting it.
////////////////////////////////////////////////////////////////////
INLINE ShaderModeArg *ShaderMode::
mod_param(const string &pname, int t) {
  int index = _shader->arg_index(pname);
  if (index >= (int)(_args.size()))
    _args.resize(index+1);
  ShaderModeArg *arg = &(_args[index]);
  arg->_type = t;
  arg->_nvalue = NodePath();
  arg->_tvalue = (Texture*)NULL;
  arg->_fvalue = LVector4d(0,0,0,0);
  return arg;
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param texture
//  Access: Public
//  Description: Store texture in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, Texture *x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_TEXTURE);
  arg->_tvalue = x;
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param nodepath
//  Access: Public
//  Description: Store nodepath in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, const NodePath &x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_NODEPATH);
  arg->_nvalue = x;
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 1f
//  Access: Public
//  Description: Store 1f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, float x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x,0,0,0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 2f
//  Access: Public
//  Description: Store 2f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector2f x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x[0],x[1],0,0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 3f
//  Access: Public
//  Description: Store 3f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector3f x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x[0],x[1],x[2],0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 4f
//  Access: Public
//  Description: Store 4f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector4f x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x[0],x[1],x[2],x[3]);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 1d
//  Access: Public
//  Description: Store 1d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, double x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x,0,0,0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 2d
//  Access: Public
//  Description: Store 2d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector2d x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x[0],x[1],0,0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 3d
//  Access: Public
//  Description: Store 3d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector3d x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = LVector4d(x[0],x[1],x[2],0);
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::set_param 4d
//  Access: Public
//  Description: Store 4d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
void ShaderMode::
set_param(const string &pname, LVector4d x) {
  ShaderModeArg *arg = mod_param(pname, ShaderModeArg::SAT_FLOAT);
  arg->_fvalue = x;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderMode::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a ShaderMode object
////////////////////////////////////////////////////////////////////
void ShaderMode::
register_with_read_factory() {
  // IMPLEMENT ME
}

