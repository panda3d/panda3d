/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelPool.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef MODELPOOL_H
#define MODELPOOL_H

#include "pandabase.h"

#include "filename.h"
#include "modelRoot.h"
#include "pointerTo.h"
#include "lightMutex.h"
#include "pmap.h"
#include "loaderOptions.h"

/**
 * This class unifies all references to the same filename, so that multiple
 * attempts to load the same model will return the same pointer.  Note that
 * the default behavior is thus to make instances: use with caution.  Use the
 * copy_subgraph() method on Node (or use NodePath::copy_to) to make
 * modifiable copies of the node.
 *
 * Unlike TexturePool, this class does not automatically resolve the model
 * filenames before loading, so a relative path and an absolute path to the
 * same model will appear to be different filenames.
 *
 * However, see the Loader class, which is now the preferred interface for
 * loading models.  The Loader class can resolve filenames, supports threaded
 * loading, and can automatically consult the ModelPool, according to the
 * supplied LoaderOptions.
 */
class EXPCL_PANDA_PGRAPH ModelPool {
PUBLISHED:
  INLINE static bool has_model(const Filename &filename);
  INLINE static bool verify_model(const Filename &filename);
  INLINE static ModelRoot *get_model(const Filename &filename, bool verify);
  BLOCKING INLINE static ModelRoot *load_model(const Filename &filename,
                                               const LoaderOptions &options = LoaderOptions());

  INLINE static void add_model(const Filename &filename, ModelRoot *model);
  INLINE static void release_model(const Filename &filename);

  INLINE static void add_model(ModelRoot *model);
  INLINE static void release_model(ModelRoot *model);

  INLINE static void release_all_models();

  INLINE static int garbage_collect();

  INLINE static void list_contents(std::ostream &out);
  INLINE static void list_contents();
  static void write(std::ostream &out);

private:
  INLINE ModelPool();

  bool ns_has_model(const Filename &filename);
  ModelRoot *ns_get_model(const Filename &filename, bool verify);
  ModelRoot *ns_load_model(const Filename &filename,
                           const LoaderOptions &options);
  void ns_add_model(const Filename &filename, ModelRoot *model);
  void ns_release_model(const Filename &filename);

  void ns_add_model(ModelRoot *model);
  void ns_release_model(ModelRoot *model);

  void ns_release_all_models();
  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;

  static ModelPool *get_ptr();

  static ModelPool *_global_ptr;

  LightMutex _lock;
  typedef pmap<Filename,  PT(ModelRoot) > Models;
  Models _models;
};

#include "modelPool.I"

#endif
