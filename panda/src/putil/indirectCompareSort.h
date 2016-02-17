/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indirectCompareSort.h
 * @author drose
 * @date 2005-03-01
 */

#ifndef INDIRECTCOMPARESORT_H
#define INDIRECTCOMPARESORT_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that define a get_sort() method.  It
 * defines the order of the pointers by sort comparison.
 */
template<class ObjectType>
class IndirectCompareSort {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectCompareSort.I"

#endif
