/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTrans.h
 * @author drose
 * @date 2000-02-14
 */

#ifndef EGGTRANS_H
#define EGGTRANS_H

#include "pandatoolbase.h"

#include "eggFilter.h"

/**
 * A program to read an egg file and write an equivalent egg file, possibly
 * performing some minor operations along the way.
 */
class EggTrans : public EggFilter {
public:
  EggTrans();

  void run();

  bool _flatten_transforms;
  bool _apply_texmats;
  bool _collapse_equivalent_textures;
  bool _remove_invalid_primitives;
  bool _triangulate_polygons;
  bool _mesh_triangles;
  bool _standardize_names;
};

#endif
