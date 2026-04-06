/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globalPointerRegistry.h
 * @author drose
 * @date 2000-02-03
 */

#ifndef GLOBALPOINTERREGISTRY_H
#define GLOBALPOINTERREGISTRY_H

#include "pandabase.h"

#include "typedObject.h"

#include "pmap.h"

/**
 * This class maintains a one-to-one mapping from TypeHandle to a void *
 * pointer.  Its purpose is to store a pointer to some class data for a given
 * class.
 *
 * Normally, one would simply use a static data member to store class data.
 * However, when the static data is associated with a template class, the
 * dynamic loader may have difficulty in properly resolving the statics.
 *
 * Consider: class foo<int> defines a static member, _a.  There should be only
 * one instance of _a shared between all instances of foo<int>, and there will
 * be a different instance of _a shared between all instances of foo<float>.
 *
 * Now suppose that two different shared libraries instantiate foo<int>.  In
 * each .so, there exists a different foo<int>::_a.  It is the loader's job to
 * recognize this and collapse them together when both libraries are loaded.
 * This usually works, but sometimes it doesn't, and you end up with two
 * different instances of foo<int>::_a; some functions see one instance, while
 * others see the other.  We have particularly seen this problem occur under
 * Linux with gcc.
 *
 * This class attempts to circumvent the problem by managing pointers to data
 * based on TypeHandle.  Since the TypeHandle will already be unique based on
 * the string name supplied to the init_type() function, it can be used to
 * differentiate foo<int> from foo<float>, while allowing different instances
 * of foo<int> to guarantee that they share the same static data.
 */
class EXPCL_PANDA_PUTIL GlobalPointerRegistry {
public:
  INLINE static void *get_pointer(TypeHandle type);
  INLINE static void store_pointer(TypeHandle type, void *ptr);
  INLINE static void clear_pointer(TypeHandle type);

private:
  // Nonstatic implementations of the above functions.
  void *ns_get_pointer(TypeHandle type) const;
  void ns_store_pointer(TypeHandle type, void *ptr);
  void ns_clear_pointer(TypeHandle type);

  INLINE static GlobalPointerRegistry *get_global_ptr();
  static GlobalPointerRegistry *_global_ptr;

private:
  typedef phash_map<TypeHandle, void *> Pointers;
  Pointers _pointers;
};

#include "globalPointerRegistry.I"

#endif
