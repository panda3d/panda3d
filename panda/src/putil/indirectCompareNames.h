// Filename: indirectCompareNames.h
// Created by:  drose (23Feb01)
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

#ifndef INDIRECTCOMPARENAMES_H
#define INDIRECTCOMPARENAMES_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : IndirectCompareNames
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pointers to classes
//               that define a get_name() method, particularly for
//               things that derive from Namable.  It defines the
//               order of the pointers by case-sensitive name
//               comparison.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
class IndirectCompareNames {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectCompareNames.I"

#endif

