/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compareTo.h
 * @author drose
 * @date 2002-02-22
 */

#ifndef COMPARETO_H
#define COMPARETO_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of classes that contain a compare_to() method.  It defines the
 * order of the pointers via compare_to().
 */
template<class ObjectType>
class CompareTo {
public:
  INLINE bool operator () (const ObjectType &a, const ObjectType &b) const;
};

#include "compareTo.I"

#endif
