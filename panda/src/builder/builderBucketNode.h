// Filename: builderBucketNode.h
// Created by:  drose (10Sep97)
//
////////////////////////////////////////////////////////////////////

#ifndef BUILDERBUCKETNODE_H
#define BUILDERBUCKETNODE_H

#include <pandabase.h>

#include "builderPrim.h"
#include "builderBucket.h"

#include <set>

class GeomNode;


///////////////////////////////////////////////////////////////////
// 	 Class : BuilderBucketNode
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

  bool add_prim(const BuilderPrim &prim);
  bool add_prim(const BuilderPrimI &prim);
  INLINE bool add_prim_nonindexed(const BuilderPrimI &prim);

  INLINE BuilderBucket *get_bucket() const;
 
  INLINE bool operator < (const BuilderBucketNode &other) const;
  INLINE bool operator == (const BuilderBucketNode &other) const;
  INLINE bool operator != (const BuilderBucketNode &other) const;

  int build(GeomNode *geom_node) const;
 
protected:
  typedef multiset<BuilderPrim, less<BuilderPrim> > Prims;
  typedef multiset<BuilderPrimI, less<BuilderPrimI> > IPrims;

  BuilderBucket *_bucket;
  Prims _prims;
  IPrims _iprims;
};

#include "builderBucketNode.I"

#endif
