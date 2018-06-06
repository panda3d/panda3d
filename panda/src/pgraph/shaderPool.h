/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderPool.h
 * @author aignacio
 * @date 2006-03
 */

#ifndef SHADERPOOL_H
#define SHADERPOOL_H

#include "pandabase.h"
#include "shader.h"
#include "filename.h"
#include "lightMutex.h"
#include "pmap.h"

/**
 * This is the preferred interface for loading shaders for the TextNode
 * system.  It is similar to ModelPool and TexturePool in that it unifies
 * references to the same filename.
 */
class EXPCL_PANDA_PGRAPH ShaderPool {
PUBLISHED:
  INLINE static bool has_shader(const Filename &filename);
  INLINE static bool verify_shader(const Filename &filename);
  BLOCKING INLINE static CPT(Shader) load_shader(const Filename &filename);
  INLINE static void add_shader(const Filename &filename, Shader *shader);
  INLINE static void release_shader(const Filename &filename);
  INLINE static void release_all_shaders();

  INLINE static int garbage_collect();

  INLINE static void list_contents(std::ostream &out);
  static void write(std::ostream &out);

private:
  INLINE ShaderPool();

  bool ns_has_shader(const Filename &orig_filename);
  CPT(Shader) ns_load_shader(const Filename &orig_filename);
  void ns_add_shader(const Filename &orig_filename, Shader *shader);
  void ns_release_shader(const Filename &orig_filename);
  void ns_release_all_shaders();
  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;

  void resolve_filename(Filename &new_filename, const Filename &orig_filename);

  static ShaderPool *get_ptr();
  static ShaderPool *_global_ptr;

  LightMutex _lock;
  typedef pmap<Filename,  CPT(Shader) > Shaders;
  Shaders _shaders;
};

#include "shaderPool.I"

#endif
