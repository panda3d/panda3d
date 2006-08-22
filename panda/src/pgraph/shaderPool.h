// Filename: shaderPool.h
// Created by:  aignacio (Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
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

#ifndef SHADERPOOL_H
#define SHADERPOOL_H

#include "pandabase.h"
#include "shader.h"
#include "filename.h"
#include "pmutex.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderPool
// Description : This is the preferred interface for loading shaders for
//               the TextNode system.  It is similar to ModelPool and
//               TexturePool in that it unifies references to the same
//               filename.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ShaderPool {
PUBLISHED:
  // These functions take string parameters instead of Filenames
  // because that's somewhat more convenient to the scripting
  // language.
  INLINE static bool has_shader(const string &filename);
  INLINE static bool verify_shader(const string &filename);
  INLINE static CPT(Shader) load_shader(const string &filename);
  INLINE static void add_shader(const string &filename, Shader *shader);
  INLINE static void release_shader(const string &filename);
  INLINE static void release_all_shaders();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);
  static void write(ostream &out);

private:
  INLINE ShaderPool();

  bool ns_has_shader(const string &str);
  CPT(Shader) ns_load_shader(const string &str);
  void ns_add_shader(const string &str, Shader *shader);
  void ns_release_shader(const string &filename);
  void ns_release_all_shaders();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out) const;

  static void lookup_filename(const string &str, string &index_str,
                              Filename &filename, int &face_index);

  static ShaderPool *get_ptr();
  static ShaderPool *_global_ptr;

  Mutex _lock;
  typedef pmap<string,  CPT(Shader) > Shaders;
  Shaders _shaders;
};

#include "shaderPool.I"

#endif
