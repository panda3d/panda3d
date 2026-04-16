/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticObject.h
 * @author rdb
 * @date 2026-04-16
 */

#ifndef STATICOBJECT_H
#define STATICOBJECT_H

#include "dtoolbase.h"

/**
 * Constructs an object in static storage that is never destroyed, avoiding
 * static destruction order issues.  Unlike a raw placement new into a static
 * char array, this wrapper leverages C++ static local variable semantics to
 * guarantee the object is constructed exactly once.
 *
 * Usage:
 * @code
 * static StaticObject<Foo> obj(arg1, arg2);
 * Foo *p = obj;
 * @endcode
 */
template<class T>
class StaticObject {
public:
  template<class... Args>
  StaticObject(Args&&... args) {
    new (_storage) T(std::forward<Args>(args)...);
  }

  operator T *() {
    return (T *)_storage;
  }

  T *operator ->() {
    return (T *)_storage;
  }

private:
  alignas(T) char _storage[sizeof(T)];
};

#endif
