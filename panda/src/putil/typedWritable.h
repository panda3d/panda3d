// Filename: typedWritable.h
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

#ifndef __TYPED_WRITABLE_
#define __TYPED_WRITABLE_

#include "typedObject.h"
#include "writable.h"
#include "vector_typedWritable.h"

class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : TypedWritable
// Description : Base class for objects that can be written to and
//               read from Bam files.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA TypedWritable : public TypedObject {
public:
  static TypedWritable* const Null;

  INLINE TypedWritable();
  INLINE TypedWritable(const TypedWritable &copy);
  INLINE void operator = (const TypedWritable &copy);

  virtual ~TypedWritable();

  virtual void write_datagram(BamWriter *, Datagram &);

  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager);

  virtual void finalize();

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    // We no longer derive from Writable, but we continue to record
    // that derivation for now, just to keep bam files consistent
    // until we move to bam version 4.0.
    TypedObject::init_type();
    Writable::init_type();
    register_type(_type_handle, "TypedWritable",
                  TypedObject::get_class_type(),
                  Writable::get_class_type());
    TypeRegistry::ptr()->record_alternate_name(_type_handle, "TypedWriteable");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "typedWritable.I"

#endif


