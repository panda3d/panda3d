/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsagePointers.h
 * @author drose
 * @date 2000-05-25
 */

#ifndef MEMORYUSAGEPOINTERS_H
#define MEMORYUSAGEPOINTERS_H

#include "pandabase.h"
#include "typedObject.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "pvector.h"

/**
 * This is a list of pointers returned by a MemoryUsage object in response to
 * some query.
 *
 * Warning: once pointers are stored in a MemoryUsagePointers object, they are
 * reference-counted, and will not be freed until the MemoryUsagePointers
 * object is freed (or clear() is called on the object).  However, they may
 * not even be freed then; pointers may leak once they have been added to this
 * structure.  This is because we don't store enough information in this
 * structure to correctly free the pointers that have been added.  Since this
 * is intended primarily as a debugging tool, this is not a major issue.
 *
 * This class is just a user interface to talk about pointers stored in a
 * MemoryUsage object.  It doesn't even exist when compiled with NDEBUG.
 */
class EXPCL_PANDA_EXPRESS MemoryUsagePointers {
PUBLISHED:
  MemoryUsagePointers();
  ~MemoryUsagePointers();

  size_t get_num_pointers() const;
  ReferenceCount *get_pointer(size_t n) const;
  MAKE_SEQ(get_pointers, get_num_pointers, get_pointer);
  TypedObject *get_typed_pointer(size_t n) const;
  MAKE_SEQ(get_typed_pointers, get_num_pointers, get_typed_pointer);

  TypeHandle get_type(size_t n) const;
  std::string (get_type_name)(size_t n) const;
  double get_age(size_t n) const;

#ifdef DO_MEMORY_USAGE
  EXTENSION(PyObject *get_python_pointer(size_t n) const);
#endif

  void clear();

  void output(std::ostream &out) const;

private:
  void add_entry(ReferenceCount *ref_ptr, TypedObject *typed_ptr,
                 TypeHandle type, double age);

  class Entry {
  public:
    INLINE Entry(ReferenceCount *ref_ptr, TypedObject *typed_ptr,
                 TypeHandle type, double age);
    INLINE Entry(const Entry &copy);
    INLINE void operator = (const Entry &copy);
    INLINE ~Entry();

    // We have an ordinary pointer to a type ReferenceCount, and not a
    // PT(ReferenceCount), because we can't actually delete this thing (since
    // ReferenceCount has no public destructor).  If we can't delete it, we
    // can't make a PointerTo it, since PointerTo wants to be able to delete
    // things.
    ReferenceCount *_ref_ptr;
    TypedObject *_typed_ptr;
    TypeHandle _type;
    double _age;
  };

  typedef pvector<Entry> Entries;
  Entries _entries;
  friend class MemoryUsage;
};

INLINE std::ostream &operator << (std::ostream &out, const MemoryUsagePointers &mup) {
  mup.output(out);
  return out;
}

#include "memoryUsagePointers.I"

#endif
