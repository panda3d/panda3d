// Filename: materialPool.h
// Created by:  drose (30Apr01)
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

#ifndef MATERIALPOOL_H
#define MATERIALPOOL_H

#include "pandabase.h"

#include "material.h"

#include "pointerTo.h"

#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : MaterialPool
// Description : The MaterialPool (there is only one in the universe)
//               serves to unify different pointers to the same
//               Material, so we do not (a) waste memory with many
//               different Material objects that are all equivalent,
//               and (b) waste time switching the graphics engine
//               between different Material states that are really the
//               same thing.
//
//               The idea is to create a temporary Material
//               representing the lighting state you want to apply,
//               then call get_material(), passing in your temporary
//               Material.  The return value will be a constant
//               Material object that should be modified (because it
//               is now shared among many different geometries), that
//               is the same as the temporary Material pointer you
//               supplied but may be a different pointer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MaterialPool {
PUBLISHED:
  INLINE static const Material *get_material(const CPT(Material) &temp);
  INLINE static int garbage_collect();
  INLINE static void list_contents(ostream &out);

private:
  INLINE MaterialPool();

  const Material *ns_get_material(const CPT(Material) &temp);
  int ns_garbage_collect();
  void ns_list_contents(ostream &out);

  static MaterialPool *get_ptr();

  static MaterialPool *_global_ptr;
  typedef pset< CPT(Material), indirect_compare_to<const Material *> > Materials;
  Materials _materials;
};

#include "materialPool.I"

#endif


