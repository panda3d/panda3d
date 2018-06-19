/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxObjectCollection.h
 * @author enn0x
 * @date 2009-11-08
 */

#ifndef PHYSXOBJECTCOLLECTION_H
#define PHYSXOBJECTCOLLECTION_H

#include "pandabase.h"

#include "config_physx.h"

/**
 *
 */
template <class T>
class EXPCL_PANDAPHYSX PhysxObjectCollection {

public:
  INLINE unsigned int size() const;
  INLINE void add(PT(T) object);
  INLINE void remove(PT(T) object);
  INLINE T *get(unsigned int index) const;
  INLINE T *operator [] (unsigned int index) const;

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

private:
  pvector<PT(T)> _objects;
};

#include "physxObjectCollection.I"

#endif // PHYSXOBJECTCOLLECTION_H
