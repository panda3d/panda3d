// Filename: builderNormalVisualizer.h
// Created by:  drose (08Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef BUILDERNORMALVISUALIZER_H
#define BUILDERNORMALVISUALIZER_H

#include <pandabase.h>

#include "mesherConfig.h"

#ifdef SUPPORT_SHOW_NORMALS

#include "builderBucket.h"
#include "builderAttrib.h"
#include "builderVertex.h"
#include "builderPrim.h"

#include <vector>

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
  vector<BuilderPrim> _lines;
};

#include "builderNormalVisualizer.I"

#endif  // SUPPORT_SHOW_NORMALS

#endif
