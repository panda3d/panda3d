// Filename: eggTrans.h
// Created by:  drose (14Feb00)
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

#ifndef EGGTRANS_H
#define EGGTRANS_H

#include "pandatoolbase.h"

#include "eggFilter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggTrans
// Description : A program to read an egg file and write an equivalent
//               egg file, possibly performing some minor operations
//               along the way.
////////////////////////////////////////////////////////////////////
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

