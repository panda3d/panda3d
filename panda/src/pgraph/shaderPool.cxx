// Filename: shaderPool.cxx
// Created by:  aignacio (Mar06)
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

#include "shaderPool.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "loader.h"
#include "shader.h"

ShaderPool *ShaderPool::_global_ptr = (ShaderPool *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::write
//       Access: Published, Static
//  Description: Lists the contents of the shader pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
void ShaderPool::
write(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_has_shader
//       Access: Private
//  Description: The nonstatic implementation of has_shader().
////////////////////////////////////////////////////////////////////
bool ShaderPool::
ns_has_shader(const string &str) {
  MutexHolder holder(_lock);

  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  Shaders::const_iterator ti;
  ti = _shaders.find(index_str);
  if (ti != _shaders.end()) {
    // This shader was previously loaded.
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_load_shader
//       Access: Private
//  Description: The nonstatic implementation of load_shader().
////////////////////////////////////////////////////////////////////
CPT(Shader) ShaderPool::
ns_load_shader(const string &str) {
  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  {
    MutexHolder holder(_lock);

    Shaders::const_iterator ti;
    ti = _shaders.find(index_str);
    if (ti != _shaders.end()) {
      // This shader was previously loaded.
      return (*ti).second;
    }
  }

/*
  shader_cat.info()
    << "Loading shader " << filename << "\n";
*/

  CPT(Shader) shader;

  shader = (CPT(Shader)) NULL;
  string extension = filename.get_extension();

  if (extension.empty() || extension == "cg" || extension == "hlsl") {
    // this does nothing for now
  }

// ***** face_index ???

  if (shader == (CPT(Shader)) NULL) {
    shader = Shader::load (filename);
  }

  if (shader == (CPT(Shader)) NULL) {
    // This shader was not found or could not be read.
    return NULL;
  }

  {
    MutexHolder holder(_lock);

    // Now try again.  Someone may have loaded the shader in another
    // thread.
    Shaders::const_iterator ti;
    ti = _shaders.find(index_str);
    if (ti != _shaders.end()) {
      // This shader was previously loaded.
      return (*ti).second;
    }

    _shaders[index_str] = shader;
  }

  return shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_add_shader
//       Access: Private
//  Description: The nonstatic implementation of add_shader().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_add_shader(const string &str, Shader *shader) {
  MutexHolder holder(_lock);

  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  // We blow away whatever shader was there previously, if any.
  _shaders[index_str] = shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_release_shader
//       Access: Private
//  Description: The nonstatic implementation of release_shader().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_release_shader(const string &filename) {
  MutexHolder holder(_lock);

  Shaders::iterator ti;
  ti = _shaders.find(filename);
  if (ti != _shaders.end()) {
    _shaders.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_release_all_shaders
//       Access: Private
//  Description: The nonstatic implementation of release_all_shaders().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_release_all_shaders() {
  MutexHolder holder(_lock);

  _shaders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int ShaderPool::
ns_garbage_collect() {
  MutexHolder holder(_lock);

  int num_released = 0;
  Shaders new_set;

  Shaders::iterator ti;
  for (ti = _shaders.begin(); ti != _shaders.end(); ++ti) {
    CPT(Shader) shader = (*ti).second;
    if (shader->get_ref_count() == 1) {
/*
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Releasing " << (*ti).first << "\n";
      }
*/
      num_released++;
    } else {
      new_set.insert(new_set.end(), *ti);
    }
  }

  _shaders.swap(new_set);
  return num_released;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_list_contents(ostream &out) const {
  MutexHolder holder(_lock);

  out << _shaders.size() << " shaders:\n";
  Shaders::const_iterator ti;
  for (ti = _shaders.begin(); ti != _shaders.end(); ++ti) {
    CPT(Shader) shader = (*ti).second;
    out << "  " << (*ti).first
        << " (count = " << shader->get_ref_count() << ")\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::lookup_filename
//       Access: Private, Static
//  Description: Accepts a shader "filename", which might consist of a
//               filename followed by an optional colon and a face
//               index, and splits it out into its two components.
//               Then it looks up the filename on the model path.
//               Sets the filename and face index accordingly.  Also
//               sets index_str to be the concatenation of the
//               found filename with the face index, thus restoring
//               the original input (but normalized to contain the
//               full path.)
////////////////////////////////////////////////////////////////////
void ShaderPool::
lookup_filename(const string &str, string &index_str,
                Filename &filename, int &face_index) {
  int colon = (int)str.length() - 1;
  // Scan backwards over digits for a colon.
  while (colon >= 0 && isdigit(str[colon])) {
    --colon;
  }
  if (colon >= 0 && str[colon] == ':') {
    string digits = str.substr(colon + 1);
    filename = str.substr(0, colon);
    face_index = atoi(digits.c_str());
  } else {
    filename = str;
    face_index = 0;
  }

  // Now look up the filename on the model path.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(filename, get_model_path());

  ostringstream strm;
  strm << filename << ":" << face_index;
  index_str = strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one ShaderPool object in the system.
////////////////////////////////////////////////////////////////////
ShaderPool *ShaderPool::
get_ptr() {
  if (_global_ptr == (ShaderPool *)NULL) {
    _global_ptr = new ShaderPool;
  }
  return _global_ptr;
}
