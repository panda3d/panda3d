// Filename: builderFuncs.h
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
#ifndef BUILDERFUNCS_H
#define BUILDERFUNCS_H

#include "pandabase.h"
#include "pvector.h"

class BuilderBucket;
class GeomNode;


////////////////////////////////////////////////////////////////////
//     Function: expand
//  Description: Receives a single primitive as a BuilderPrim or
//               BuilderPrimI object, as input by the user.  Does some
//               initial processing on the primitive to verify
//               internal consistency (for instance, that a quad has
//               four vertices), and returns a new BuilderPrim or
//               series of BuilderPrim objects, suitable for building
//               with.
//
//               More than one primitive might be returned because
//               higher-order polygons may be broken up into
//               triangles, and linestrips and points are broken into
//               their component pieces.  The output primitives are
//               written into the STL container defined by result.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
bool
expand(const PrimType &prim, BuilderBucket &bucket,
       OutputIterator result);


////////////////////////////////////////////////////////////////////
//     Function: mesh_and_build
//  Description: Accepts a list of BuilderPrim or BuilderPrimI
//               objects, defined by the iterators first and list,
//               runs them through the mesher if specified by the
//               bucket, and builds them into the indicated GeomNode.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
int
mesh_and_build(InputIterator first, InputIterator last,
               BuilderBucket &bucket, GeomNode *geom_node);



////////////////////////////////////////////////////////////////////
//     Function: split
//  Description: Splits an STL list into two other lists, according to
//               the return value from pred.
////////////////////////////////////////////////////////////////////
template <class InputIterator, class OutputIterator, class Predicate>
OutputIterator split(InputIterator first, InputIterator last,
                     OutputIterator true_result, OutputIterator false_result,
                     Predicate pred);

#include "builderFuncs.I"

#endif
