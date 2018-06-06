/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStagePool.h
 * @author drose
 * @date 2010-05-03
 */

#ifndef TEXTURESTAGEPOOL_H
#define TEXTURESTAGEPOOL_H

#include "pandabase.h"
#include "textureStage.h"
#include "pointerTo.h"
#include "pmutex.h"
#include "pset.h"

/**
 * The TextureStagePool (there is only one in the universe) serves to unify
 * different pointers to the same TextureStage, mainly to help developers use
 * a common pointer to access things that are loaded from different model
 * files.
 *
 * It runs in one of three different modes, according to set_mode().  See that
 * method for more information.
 */
class EXPCL_PANDA_GOBJ TextureStagePool {
PUBLISHED:
  enum Mode {
    M_none,
    M_name,
    M_unique,
  };

  INLINE static TextureStage *get_stage(TextureStage *temp);
  INLINE static void release_stage(TextureStage *temp);
  INLINE static void release_all_stages();

  INLINE static void set_mode(Mode mode);
  INLINE static Mode get_mode();
  MAKE_PROPERTY(mode, get_mode, set_mode);

  INLINE static int garbage_collect();
  INLINE static void list_contents(std::ostream &out);
  static void write(std::ostream &out);

private:
  TextureStagePool();

  TextureStage *ns_get_stage(TextureStage *temp);
  void ns_release_stage(TextureStage *temp);
  void ns_release_all_stages();

  void ns_set_mode(Mode mode);
  Mode ns_get_mode();

  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;

  static TextureStagePool *get_global_ptr();

  static TextureStagePool *_global_ptr;

  Mutex _lock;

  // We store a map of CPT(TextureStage) to PT(TextureStage).  These are two
  // equivalent structures, but different pointers.  The first pointer never
  // leaves this class.  If the second pointer changes value, we'll notice it
  // and return a new one.
  typedef pmap<CPT(TextureStage), PT(TextureStage), indirect_compare_to<const TextureStage *> > StagesByProperties;
  StagesByProperties _stages_by_properties;

  typedef pmap<std::string, PT(TextureStage) > StagesByName;
  StagesByName _stages_by_name;

  Mode _mode;
};

EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, TextureStagePool::Mode mode);
EXPCL_PANDA_GOBJ std::istream &operator >> (std::istream &in, TextureStagePool::Mode &mode);

#include "textureStagePool.I"

#endif
