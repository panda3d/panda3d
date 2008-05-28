// Filename: firstOfPairLess.h
// Created by:  drose (27Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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

