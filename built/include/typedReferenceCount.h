/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedReferenceCount.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef TYPEDREFERENCECOUNT_H
#define TYPEDREFERENCECOUNT_H

#include "pandabase.h"

#include "typedObject.h"
#include "referenceCount.h"

/**
 * A base class for things which need to inherit from both TypedObject and
 * from ReferenceCount.  It's convenient to define this intermediate base
 * class instead of multiply inheriting from the two classes each time they
 * are needed, so that we can sensibly pass around pointers to things which
 * are both TypedObjects and ReferenceCounters.
 *
 * See also TypedObject for detailed instructions.
 */
class EXPCL_PANDA_EXPRESS TypedReferenceCount : public TypedObject, public ReferenceCount {
public:
  INLINE TypedReferenceCount();
  INLINE TypedReferenceCount(const TypedReferenceCount &copy);
  INLINE void operator = (const TypedReferenceCount &copy);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "TypedReferenceCount",
                  TypedObject::get_class_type(),
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "typedReferenceCount.I"

#endif
