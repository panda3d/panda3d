// Filename: builder.h
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

#ifndef BUILDER_H
#define BUILDER_H

////////////////////////////////////////////////////////////////////
//
// Builder
//
// The builder accepts as input a loose collection of polygons with
// various attributes, sizes, and shapes, and does all the work of
// grouping relating polygons and creating triangle strips, etc.,
// ultimately storing the resulting optimized geometry into one or
// more GeomNodes.
//
// It is intended that the builder should be the single entry point
// for all code wishing to create geometry in the scene graph.  The
// builder can know about the kinds of geometry that are optimal for a
// particular platform, or even about the kinds of geometry that are
// available for a given platform.  (For instance, perhaps on some
// bizarre platform, triangle strips do not exist, but quadstrips are
// really fast.  User code should not create triangle strips
// directly.)
//
// Actually, there are two fairly separate pieces in this package.
// The first is the builder itself, which handles the interface to
// user code, and is responsible for collecting polygons from the
// caller, sorting them according to their attributes, and creating
// Geoms that represent the resulting geometry.  The second piece is
// the mesher, which receives geometry from the builder and tries to
// create optimal triangle strips (or whichever kind of higher-level
// structure is most appropriate) from them, which it hands back to
// the builder.
//
// It is possible to use the builder without invoking the mesher, in
// which case the builder will create Geoms with the individual prims
// exactly as the user passed them in.  It is not possible to use the
// mesher without first going through the builder.
//
//
// The general system of using the builder is as follows:
//
//  (a) Create a Builder object.
//
//  (b) Iterate through the polygons.  For each polygon:
//
//      (c) Create a BuilderBucket object and assign to it the
//          scene-graph level attributes, such as texture, lighting,
//          etc. for your polygon.  If several polygons share the same
//          attributes, they can of course use the same bucket.  But
//          there's no reason to be afraid of creating a new bucket
//          object each time, if that's more convenient.
//
//      (d) Create a BuilderPrim object to describe the polygon.  If
//          the polygon is to have a polygon color or polygon normal,
//          set these on the BuilderPrim.
//
//      (e) Iterate through the polygon vertices, in counterclockwise
//          order when viewed from the front of the polygon.  For each
//          vertex:
//
//          (f) Create a BuilderVertex object.  If the vertex has a
//              texture coordinate, normal, or color, set this on the
//              BuilderVertex.
//
//          (g) Add the BuilderVertex to the BuilderPrim.
//
//      (h) Add the BuilderPrim to the Builder.
//
//  (i) Call Builder::build() and receive your new geometry!
//
// All of these objects--BuilderBucket, BuilderPrim, and
// BuilderVertex--can, and probably should, be ordinary local
// variables.  When they are added into their respective data
// structures they are copied, not referenced, so there's no need to
// try to keep them around after that.
//
// The BuilderBucket is the builder's system for grouping polygons
// that share similar characteristings.  Polygons that were added to
// the builder with equivalent (though not necessarily identical)
// buckets may be candidates for joining together into triangle strips
// when possible.
//
//
// That's the basic operation.  There are lots of fancy features on
// top of that.
//
//  * Other kinds of geometry than polygons are supported.  Presently,
//    these are light points and line segments.  To add these kinds of
//    geometry, call set_type() on your BuilderPrim with either
//    BPT_point or BPT_line.
//
//  * Indexed geometry is supported as well as nonindexed.  Indexed
//    geometry means that the vertices, UV's, etc. are referenced
//    indirectly; an index number into a table is stored instead of
//    the actual coordinate values.  Indexed geometry may be freely
//    mixed in with nonindexed geometry; the builder will sort them
//    out (although each polygon must be either entirely indexed or
//    entirely nonindexed).  To create indexed geometry, use a
//    BuilderPrimI object, and assign to it a number of BuilderVertexI
//    vertices.  The coordinate values you will assign are ushort
//    array index numbers.  Store the array pointers these refer to in
//    the BuilderBucket, via set_coords(), set_normals(), etc.
//
//  * The builder is to a certain extent scene-graph aware.  In the
//    normal usage, you give it a bunch of polygons which are all
//    lumped together, and when you call build() it allocates and
//    returns a GeomNode which has all of your geometry in it.
//    However, you can also ask it to distribute the geometry
//    throughout a pre-existing scene graph.  To do this, assign the
//    _node pointer of your BuilderBucket to point to the node each
//    polygon belongs under, as you create the polygons.  Now when you
//    call build(), the builder will create all the polygons under the
//    nodes you indicated, creating new GeomNodes whenever necessary.
//    The advantage to this method is that you don't have to process
//    your polygons in scene-graph order; the builder can sort them
//    out for you.  Another advantage is it allows the builder to set
//    up the state for you, see the next point:
//
//  * It is only when you are taking advantage of the scene-graph
//    awareness of the builder that the builder can assign the state
//    transitions (like texturing, etc.) you specify to the geometry
//    it builds.  This is because the state transitions are stored on
//    the arcs of the scene graph, and in non-scene graph mode there
//    are no arcs!
//
//  * You can fine-tune the mesher behavior via a number of parameters
//    on the BuilderBucket.  Look in builderProperties.h for these
//    parameters (BuilderBucket inherits from BuilderProperties).
//    This is also where you turn the mesher off if you don't want it.
//
//  * You can set global properties on all buckets easily either by
//    creating your own default BuilderBucket that you use to
//    initialize each individual BuilderBucket you create, or by
//    changing the parameters stored in the bucket pointed to by
//    BuilderBucket::get_default_bucket(), which is what is used to
//    initialize any BuilderBucket created with a default constructor.
//    It is suggested that the get_default_bucket() pointer be used to
//    define global defaults at applications start-up, while a local
//    default BuilderBucket should be used for local defaults.
//
//  * You can also control the binning behavior, if you have some
//    particular user-specific parameters you want your geometry to be
//    grouped on.  To do this, subclass from BuilderBucket and
//    redefine the comparison operator (it's a virtual function), as
//    well as the make_copy() function.
//
////////////////////////////////////////////////////////////////////



#include "pandabase.h"

#include "builderAttrib.h"
#include "builderBucketNode.h"

#include "pointerTo.h"

#include "pset.h"


class GeomNode;


///////////////////////////////////////////////////////////////////
//       Class : Builder
// Description : The main driver class to the builder package.  See
//               the comments above.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG Builder {
public:
  Builder();
  ~Builder();

  INLINE bool add_prim(const BuilderBucket &bucket,
                       const BuilderPrim &prim);
  INLINE bool add_prim(const BuilderBucket &bucket,
                       const BuilderPrimI &prim);
  INLINE bool add_prim_nonindexed(const BuilderBucket &bucket,
                                  const BuilderPrimI &prim);

  GeomNode *build(const string &default_name = "");

protected:
  void add_bucket(const BuilderBucket &bucket);

  typedef pset<BuilderBucketNode> Buckets;

  Buckets _buckets;
  Buckets::iterator _bi;
};

#include "builder.I"

#endif
