/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritableReferenceCount.h
 * @author jason
 * @date 2000-06-08
 */

#ifndef TYPEDWRITABLEREFERENCECOUNT_H
#define TYPEDWRITABLEREFERENCECOUNT_H

#include "pandabase.h"

#include "typedWritable.h"
#include "referenceCount.h"

/**
 * A base class for things which need to inherit from both TypedWritable and
 * from ReferenceCount.  It's convenient to define this intermediate base
 * class instead of multiply inheriting from the two classes each time they
 * are needed, so that we can sensibly pass around pointers to things which
 * are both TypedWritables and ReferenceCounters.
 *
 * See also TypedObject for detailed instructions.
 */
class EXPCL_PANDA_PUTIL TypedWritableReferenceCount : public TypedWritable, public ReferenceCount {
public:
  INLINE TypedWritableReferenceCount();
  INLINE TypedWritableReferenceCount(const TypedWritableReferenceCount &copy);
  INLINE void operator = (const TypedWritableReferenceCount &copy);

  virtual ReferenceCount *as_reference_count();

PUBLISHED:
  static PT(TypedWritableReferenceCount) decode_from_bam_stream(vector_uchar data, BamReader *reader = nullptr);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "TypedWritableReferenceCount",
                  TypedWritable::get_class_type(),
                  ReferenceCount::get_class_type());
    TypeRegistry::ptr()->record_alternate_name(_type_handle,
                                               "TypedWriteableReferenceCount");
  }

private:
  static TypeHandle _type_handle;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<TypedWritableReferenceCount>::update_type(To *ptr) {}

#include "typedWritableReferenceCount.I"

#endif
