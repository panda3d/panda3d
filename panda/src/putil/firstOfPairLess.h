// Filename: firstOfPairLess.h
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

#ifndef FIRSTOFPAIRLESS_H
#define FIRSTOFPAIRLESS_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : FirstOfPairLess
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pairs of objects.
//               It uses < to compare the first elements of the pair.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
class FirstOfPairLess {
public:
  INLINE bool operator () (const ObjectType &a, const ObjectType &b) const;
};

#include "firstOfPairLess.I"

#endif

