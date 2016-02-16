/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indirectCompareNames.h
 * @author drose
 * @date 2001-02-23
 */

#ifndef INDIRECTCOMPARENAMES_H
#define INDIRECTCOMPARENAMES_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that define a get_name() method,
 * particularly for things that derive from Namable.  It defines the order of
 * the pointers by case-sensitive name comparison.
 */
template<class ObjectType>
class IndirectCompareNames {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectCompareNames.I"

#endif
