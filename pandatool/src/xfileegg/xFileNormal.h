// Filename: xFileNormal.h
// Created by:  drose (19Jun01)
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

#ifndef XFILENORMAL_H
#define XFILENORMAL_H

#include "pandatoolbase.h"
#include "luse.h"

class EggVertex;
class EggPrimitive;

////////////////////////////////////////////////////////////////////
//       Class : XFileNormal
// Description : This represents a single normal associated with an
//               XFileFace.  It is separate from XFileVertex, because
//               the X syntax supports a different table of normals
//               than that of vertices.
////////////////////////////////////////////////////////////////////
class XFileNormal {
public:
  XFileNormal();
  void set_from_egg(EggVertex *egg_vertex, EggPrimitive *egg_prim);
  int compare_to(const XFileNormal &other) const;

  Normalf _normal;
  bool _has_normal;
};

#endif

