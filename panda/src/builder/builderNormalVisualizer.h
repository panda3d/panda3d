// Filename: builderNormalVisualizer.h
// Created by:  drose (08Sep99)
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

#ifndef BUILDERNORMALVISUALIZER_H
#define BUILDERNORMALVISUALIZER_H

#include "pandabase.h"

#include "mesherConfig.h"

#ifdef SUPPORT_SHOW_NORMALS

#include "builderBucket.h"
#include "builderAttrib.h"
#include "builderVertex.h"
#include "builderPrim.h"

#include "pvector.h"

///////////////////////////////////////////////////////////////////
//       Class : BuilderNormalVisualizer
// Description : A useful class for collecting information about
//               vertices and their associated normals as geometry is
//               built, so that its normals may be visualized via
//               renderable geometry.  This is activated by the
//               _show_normals flag in the BuilderProperties.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderNormalVisualizer {
public:
  INLINE BuilderNormalVisualizer(BuilderBucket &bucket);

  void add_prim(const BuilderPrim &prim);
  void add_prim(const BuilderPrimI &prim);

  void show_normals(GeomNode *node);

private:
  void add_normal(const BuilderV &center, const BuilderN &normal);

  BuilderBucket &_bucket;

  BuilderV _net_vertex;
  int _num_vertices;
  pvector<BuilderPrim> _lines;
};

#include "builderNormalVisualizer.I"

#endif  // SUPPORT_SHOW_NORMALS

#endif
