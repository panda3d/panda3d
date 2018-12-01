/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.h
 * @author drose
 * @date 2001-04-30
 */

#ifndef MATERIALPOOL_H
#define MATERIALPOOL_H

#include "pandabase.h"
#include "material.h"
#include "pointerTo.h"
#include "lightMutex.h"
#include "pset.h"

/**
 * The MaterialPool (there is only one in the universe) serves to unify
 * different pointers to the same Material, so we do not (a) waste memory with
 * many different Material objects that are all equivalent, and (b) waste time
 * switching the graphics engine between different Material states that are
 * really the same thing.
 *
 * The idea is to create a temporary Material representing the lighting state
 * you want to apply, then call get_material(), passing in your temporary
 * Material.  The return value will either be a new Material object, or it may
 * be the the same object you supplied; in either case, it will have the same
 * value.
 */
class EXPCL_PANDA_GOBJ MaterialPool {
PUBLISHED:
  INLINE static Material *get_material(Material *temp);
  INLINE static void release_material(Material *temp);
  INLINE static void release_all_materials();

  INLINE static int garbage_collect();
  INLINE static void list_contents(std::ostream &out);

  static void write(std::ostream &out);

private:
  INLINE MaterialPool();

  Material *ns_get_material(Material *temp);
  void ns_release_material(Material *temp);
  void ns_release_all_materials();

  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;

  static MaterialPool *get_global_ptr();

  static MaterialPool *_global_ptr;

  LightMutex _lock;

  // We store a map of CPT(Material) to PT(Material).  These are two
  // equivalent structures, but different pointers.  The first pointer never
  // leaves this class.  If the second pointer changes value, we'll notice it
  // and return a new one.
  typedef pmap< CPT(Material), PT(Material), indirect_compare_to<const Material *> > Materials;
  Materials _materials;
};

#include "materialPool.I"

#endif
