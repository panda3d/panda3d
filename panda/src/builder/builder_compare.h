// Filename: builder_compare.h
// Created by:  drose (31Jul01)
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

#ifndef BUILDERCOMPARE_H
#define BUILDERCOMPARE_H

#include "pandabase.h"

#include "luse.h"


// This file defines some trivial functions that return strcmp-style
// comparisonresults for the major types of our vertices.  This allows
// us to write type-independent template code to compare vertex
// values for equivalence or order relationships.

INLINE int builder_compare(const LVecBase2f &a, const LVecBase2f &b);
INLINE int builder_compare(const LVecBase3f &a, const LVecBase3f &b);
INLINE int builder_compare(const LVecBase4f &a, const LVecBase4f &b);
INLINE int builder_compare(ushort a, ushort b);

#include "builder_compare.I"

#endif

