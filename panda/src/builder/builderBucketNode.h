// Filename: builderBucketNode.h
// Created by:  drose (10Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BUILDERBUCKETNODE_H
#define BUILDERBUCKETNODE_H

#include "pandabase.h"

#include "builderPrim.h"
#include "builderBucket.h"

#include "pset.h"

class GeomNode;
class GeomNode;


///////////////////////////////////////////////////////////////////
//       Class : BuilderBucketNode
// Description : This is a wrapper class around BuilderBucket, used by
//               the Builder class.  It stores a pointer to a
//               BuilderBucket object, as well as lists of the
//               primitives that have been added to it.
//
//               There are no functions in this class that are
//               intended to be called directly by user code; instead,
//               use the interface provided by Builder.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderBucketNode {
public:
  INLINE BuilderBucketNode(BuilderBucket *bucket);
  INLINE BuilderBucketNode(const BuilderBucketNode &copy);
  INLINE void operator = (const BuilderBucketNode &copy);

  bool add_prim(BuilderPrim prim);
  bool add_prim(const BuilderPrimI &prim);
  INLINE bool add_prim_nonindexed(const BuilderPrimI &prim);

  INLINE BuilderBucket *get_bucket() const;

  INLINE bool operator < (const BuilderBucketNode &other) const;
  INLINE bool operator == (const BuilderBucketNode &other) const;
  INLINE bool operator != (const BuilderBucketNode &other) const;

  int build(GeomNode *geom_node) const;

protected:
  typedef pmultiset<BuilderPrim, less<BuilderPrim> > Prims;
  typedef pmultiset<BuilderPrimI, less<BuilderPrimI> > IPrims;

  BuilderBucket *_bucket;
  Prims _prims;
  IPrims _iprims;
};

#include "builderBucketNode.I"

#endif
