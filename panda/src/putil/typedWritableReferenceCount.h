// Filename: typedWritableReferenceCount.h
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TYPEDWRITABLEREFERENCECOUNT_H
#define TYPEDWRITABLEREFERENCECOUNT_H

#include "pandabase.h"

#include "typedWritable.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : TypedWritableReferenceCount
// Description : A base class for things which need to inherit from
//               both TypedWritable and from ReferenceCount.  It's
//               convenient to define this intermediate base class
//               instead of multiply inheriting from the two classes
//               each time they are needed, so that we can sensibly
//               pass around pointers to things which are both
//               TypedWritables and ReferenceCounters.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TypedWritableReferenceCount : public TypedWritable, public ReferenceCount {
public:
  INLINE TypedWritableReferenceCount();
  INLINE TypedWritableReferenceCount(const TypedWritableReferenceCount &copy);
  INLINE void operator = (const TypedWritableReferenceCount &copy);

public:
  virtual void write_datagram(BamWriter *, Datagram &) = 0;

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

#include "typedWritableReferenceCount.I"

#endif
