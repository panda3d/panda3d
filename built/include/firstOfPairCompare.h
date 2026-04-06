/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file firstOfPairCompare.h
 * @author drose
 * @date 2001-06-27
 */

#ifndef FIRSTOFPAIRCOMPARE_H
#define FIRSTOFPAIRCOMPARE_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pairs of objects.  It invokes the indicated comparison
 * function object on the first object of its pair.
 */
template<class ObjectType, class Compare>
class FirstOfPairCompare {
public:
  INLINE FirstOfPairCompare(Compare compare = Compare());
  INLINE bool operator () (const ObjectType &a, const ObjectType &b) const;
  Compare _compare;
};

#include "firstOfPairCompare.I"

#endif
