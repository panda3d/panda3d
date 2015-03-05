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
#include "string_utils.h"

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
ns_has_shader(const Filename &orig_filename) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  LightMutexHolder holder(_lock);
  Shaders::const_iterator ti;
  ti = _shaders.find(filename);
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
ns_load_shader(const Filename &orig_filename) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  {
    LightMutexHolder holder(_lock);

    Shaders::const_iterator ti;
    ti = _shaders.find(filename);
    if (ti != _shaders.end()) {
      // This shader was previously loaded.
      return (*ti).second;
    }
  }

  // The shader was not found in the pool.
  gobj_cat.info()
    << "Loading shader " << filename << "\n";

  Shader::ShaderLanguage lang = Shader::SL_none;

  // Do some guesswork to see if we can figure out the shader language
  // from the file extension.  This is really just guesswork - there are
  // no standardized extensions for shaders, especially for GLSL.
  // These are the ones that appear to be closest to "standard".
  string ext = downcase(filename.get_extension());
  if (ext == "cg" || ext == "sha") {
    // "sha" is for historical reasons.
    lang = Shader::SL_Cg;

  } else if (ext == "glsl" || ext == "vert" || ext == "frag" ||
             ext == "geom" || ext == "tesc" || ext == "tese" ||
             ext == "comp") {
    lang = Shader::SL_GLSL;
  }

  PT(Shader) shader = Shader::load(filename, lang);
  if (shader == (Shader *)NULL) {
    // This shader was not found or could not be read.
    return NULL;
  }

  {
    LightMutexHolder holder(_lock);

    // Now try again.  Someone may have loaded the shader in another
    // thread.
    Shaders::const_iterator ti;
    ti = _shaders.find(filename);
    if (ti != _shaders.end()) {
      // This shader was previously loaded.
      return (*ti).second;
    }

    _shaders[filename] = shader;
  }

  return shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_add_shader
//       Access: Private
//  Description: The nonstatic implementation of add_shader().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_add_shader(const Filename &orig_filename, Shader *shader) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  LightMutexHolder holder(_lock);
  // We blow away whatever shader was there previously, if any.
  _shaders[filename] = shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_release_shader
//       Access: Private
//  Description: The nonstatic implementation of release_shader().
////////////////////////////////////////////////////////////////////
void ShaderPool::
ns_release_shader(const Filename &filename) {
  LightMutexHolder holder(_lock);

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
  LightMutexHolder holder(_lock);

  _shaders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int ShaderPool::
ns_garbage_collect() {
  LightMutexHolder holder(_lock);

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
  LightMutexHolder holder(_lock);

  out << _shaders.size() << " shaders:\n";
  Shaders::const_iterator ti;
  for (ti = _shaders.begin(); ti != _shaders.end(); ++ti) {
    CPT(Shader) shader = (*ti).second;
    out << "  " << (*ti).first
        << " (count = " << shader->get_ref_count() << ")\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ShaderPool::resolve_filename
//       Access: Private
//  Description: Searches for the indicated filename along the
//               model path.
////////////////////////////////////////////////////////////////////
void ShaderPool::
resolve_filename(Filename &new_filename, const Filename &orig_filename) {
  new_filename = orig_filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(new_filename, get_model_path());
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
