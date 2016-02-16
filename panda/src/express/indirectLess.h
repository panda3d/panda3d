/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indirectLess.h
 * @author drose
 * @date 2001-06-27
 */

#ifndef INDIRECTLESS_H
#define INDIRECTLESS_H

#include "pandabase.h"

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that contain an operator <() method.  It
 * defines the order of the pointers via operator <().
 */
template<class ObjectType>
class IndirectLess {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectLess.I"

#endif
