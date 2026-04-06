/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indirectCompareTo.h
 * @author drose
 * @date 2000-04-04
 */

#ifndef INDIRECTCOMPARETO_H
#define INDIRECTCOMPARETO_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that contain a compare_to() method.  It
 * defines the order of the pointers via compare_to().
 */
template<class ObjectType>
class IndirectCompareTo {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectCompareTo.I"

#endif
