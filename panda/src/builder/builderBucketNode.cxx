// Filename: builderBucketNode.cxx
// Created by:  drose (10Sep97)
// 
////////////////////////////////////////////////////////////////////

#include "builderFuncs.h"
#include "builderBucketNode.h"

#include <geomNode.h>

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::add_prim
//       Access: Public
//  Description: Adds the indicated indexed primitive to the
//               bucket, and returns true if the primitive was valid.
//               Intended to be called from Builder::add_prim().
////////////////////////////////////////////////////////////////////
bool BuilderBucketNode::
add_prim(const BuilderPrim &prim) {
  bool result = expand(prim, *_bucket, inserter(_prims, _prims.begin()));
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::add_prim
//       Access: Public
//  Description: Adds the indicated nonindexed primitive to the
//               bucket, and returns true if the primitive was valid.
//               Intended to be called from Builder::add_prim().
////////////////////////////////////////////////////////////////////
bool BuilderBucketNode::
add_prim(const BuilderPrimI &prim) {
  bool result = expand(prim, *_bucket, inserter(_iprims, _iprims.begin()));
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::build
//       Access: Public
//  Description: Builds all the geometry assigned to this particular
//               bucket, and assigns it to the indicated GeomNode.
//               Returns the number of Geoms created.
////////////////////////////////////////////////////////////////////
int BuilderBucketNode::
build(GeomNode *geom_node) const {
  int count = 0;

  {
    // First, the nonindexed.
    Prims::const_iterator pi, last_pi;
    last_pi = _prims.begin();
    
    for (pi = _prims.begin();
	 pi != _prims.end();
	 ++pi) {
      if ((*last_pi) < (*pi)) {
	count += mesh_and_build(last_pi, pi, *_bucket, geom_node, 
			        (BuilderPrim *)0);
	last_pi = pi;
      }
    }
    count += mesh_and_build(last_pi, pi, *_bucket, geom_node,
                            (BuilderPrim *)0);
  }

  {
    // Then, the indexed.
    IPrims::const_iterator pi, last_pi;
    last_pi = _iprims.begin();
    
    for (pi = _iprims.begin();
	 pi != _iprims.end();
	 ++pi) {
      if ((*last_pi) < (*pi)) {
	count += mesh_and_build(last_pi, pi, *_bucket, geom_node,
				(BuilderPrimI *)0);
	last_pi = pi;
      }
    }
    count += mesh_and_build(last_pi, pi, *_bucket, geom_node,
                            (BuilderPrimI *)0);
  }

  return count;
}

