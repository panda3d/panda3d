// Filename: typedWritable.h
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TYPEDWRITABLE_H
#define TYPEDWRITABLE_H

#include "typedObject.h"
#include "vector_typedWritable.h"
#include "pvector.h"
#include "lightMutex.h"
#include "updateSeq.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;
class ReferenceCount;

////////////////////////////////////////////////////////////////////
//       Class : TypedWritable
// Description : Base class for objects that can be written to and
//               read from Bam files.
//
//               See also TypedObject for detailed instructions.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL TypedWritable : public TypedObject {
public:
  static TypedWritable* const Null;

  INLINE TypedWritable();
  INLINE TypedWritable(const TypedWritable &copy);
  INLINE void operator = (const TypedWritable &copy);

  virtual ~TypedWritable();

  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void update_bam_nested(BamWriter *manager);

  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager);
  virtual bool require_fully_complete() const;

  virtual void fillin(DatagramIterator &scan, BamReader *manager);
  virtual void finalize(BamReader *manager);

  virtual ReferenceCount *as_reference_count();

PUBLISHED:
  INLINE void mark_bam_modified();
  INLINE UpdateSeq get_bam_modified() const;

  EXTENSION(PyObject *__reduce__(PyObject *self) const);
  EXTENSION(PyObject *__reduce_persist__(PyObject *self, PyObject *pickler) const);

  INLINE string encode_to_bam_stream() const;
  bool encode_to_bam_stream(string &data, BamWriter *writer = NULL) const;
  static bool decode_raw_from_bam_stream(TypedWritable *&ptr,
                                         ReferenceCount *&ref_ptr,
                                         const string &data,
                                         BamReader *reader = NULL);

private:
  void add_bam_writer(BamWriter *writer);
  void remove_bam_writer(BamWriter *writer);

  // We may need to store a list of the BamWriter(s) that have a
  // reference to this object, so that we can remove the object from
  // those tables when it destructs.
  struct BamWriterLink {
    BamWriter *_writer;
    BamWriterLink *_next;
  };
  AtomicAdjust::Pointer _bam_writers; // Tagged pointer

  UpdateSeq _bam_modified;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "TypedWritable",
                  TypedObject::get_class_type());
    TypeRegistry::ptr()->record_alternate_name(_type_handle, "TypedWriteable");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class BamWriter;
};

#include "typedWritable.I"

#endif
