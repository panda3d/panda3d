// Filename: builderBucketNode.I
// Created by:  drose (10Sep97)
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



////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderBucketNode::
BuilderBucketNode(BuilderBucket *bucket) : _bucket(bucket) {
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderBucketNode::
BuilderBucketNode(const BuilderBucketNode &copy) {
  (*this) = copy;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BuilderBucketNode::
operator = (const BuilderBucketNode &copy) {
  _bucket = copy._bucket;
  _prims = copy._prims;
  _iprims = copy._iprims;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::add_prim_nonindexed
//       Access: Public
//  Description: Adds the indicated indexed primitive to the bucket as
//               if it were nonindexed, and returns true if the
//               primitive was valid.  Intended to be called from
//               Builder::add_prim_nonindexed().
////////////////////////////////////////////////////////////////////
INLINE bool BuilderBucketNode::
add_prim_nonindexed(const BuilderPrimI &prim) {
  BuilderPrim nonindexed;
  nonindexed.nonindexed_copy(prim, *_bucket);
  return add_prim(nonindexed);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::get_bucket
//       Access: Public
//  Description: Returns the BuilderBucket object associated with this
//               node.
////////////////////////////////////////////////////////////////////
INLINE BuilderBucket *BuilderBucketNode::
get_bucket() const {
  return _bucket;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Ordering operator
//       Access: Public
//  Description: Returns true if this bucket precedes the indicated
//               one in ordering.  This function is used by the STL
//               set in the Builder object to sort buckets into order,
//               and to collect similar buckets together.
////////////////////////////////////////////////////////////////////
INLINE bool BuilderBucketNode::
operator < (const BuilderBucketNode &other) const {
  return (*_bucket) < (*other._bucket);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Equivalence operator
//       Access: Public
//  Description: Returns true if the two buckets are equivalent, based
//               on the ordering operator, above.
////////////////////////////////////////////////////////////////////
INLINE bool BuilderBucketNode::
operator == (const BuilderBucketNode &other) const {
  return !((*_bucket) < (*other._bucket) ||
           (*other._bucket) < (*_bucket));
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucketNode::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BuilderBucketNode::
operator != (const BuilderBucketNode &other) const {
  return !operator == (other);
}
