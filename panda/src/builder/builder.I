// Filename: builder.I
// Created by:  drose (09Sep97)
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
//     Function: Builder::add_prim
//       Access: Public
//  Description: Adds the indicated nonindexed primitive, along with
//               its associated bucket, to the builder's internal
//               structures.  This copies all relevant information
//               into the builder.
//
//               Returns true if the primitive was successfully added,
//               false if it was invalid for some reason (for
//               instance, a polygon with two vertices).
////////////////////////////////////////////////////////////////////
INLINE bool Builder::
add_prim(const BuilderBucket &bucket,
         const BuilderPrim &prim) {
  add_bucket(bucket);
  return ((BuilderBucketNode &)(*_bi)).add_prim(prim);
}


////////////////////////////////////////////////////////////////////
//     Function: Builder::add_prim
//       Access: Public
//  Description: Adds the indicated indexed primitive, along with
//               its associated bucket, to the builder's internal
//               structures.  This copies all relevant information
//               into the builder.
//
//               Returns true if the primitive was successfully added,
//               false if it was invalid for some reason (for
//               instance, a polygon with two vertices).
////////////////////////////////////////////////////////////////////
INLINE bool Builder::
add_prim(const BuilderBucket &bucket,
         const BuilderPrimI &prim) {
  add_bucket(bucket);
  return ((BuilderBucketNode &)(*_bi)).add_prim(prim);
}


////////////////////////////////////////////////////////////////////
//     Function: Builder::add_prim_nonindexed
//       Access: Public
//  Description: Adds the specified indexed primitive as if it were
//               nonindexed.  This simply looks up each coordinate
//               value on the prim in its associated array and stores
//               a nonindexed primitive with the actual coordinate
//               values, instead of the index numbers.
//
//               This is handy for having code that calls the builder
//               and might want to build either indexed or nonindexed
//               geometry, as selected by the user at run-time.
//               Simply build indexed geometry in all cases, then call
//               either add_prim or add_prim_nonindexed, according to
//               the user's selection.
////////////////////////////////////////////////////////////////////
INLINE bool Builder::
add_prim_nonindexed(const BuilderBucket &bucket,
                    const BuilderPrimI &prim) {
  add_bucket(bucket);
  return ((BuilderBucketNode &)(*_bi)).add_prim_nonindexed(prim);
}
