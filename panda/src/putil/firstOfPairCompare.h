// Filename: firstOfPairCompare.h
// Created by:  drose (27Jun01)
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

#ifndef FIRSTOFPAIRCOMPARE_H
#define FIRSTOFPAIRCOMPARE_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : FirstOfPairCompare
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pairs of objects.
//               It invokes the indicated comparison function object
//               on the first object of its pair.
////////////////////////////////////////////////////////////////////
template<class ObjectType, class Compare>
class FirstOfPairCompare {
public:
  INLINE FirstOfPairCompare(Compare compare = Compare());
  INLINE bool operator () (const ObjectType &a, const ObjectType &b) const;
  Compare _compare;
};

#include "firstOfPairCompare.I"

#endif

