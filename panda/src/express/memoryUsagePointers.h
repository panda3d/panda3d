// Filename: memoryUsagePointers.h
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MEMORYUSAGEPOINTERS_H
#define MEMORYUSAGEPOINTERS_H

#include <pandabase.h>

#include "typeHandle.h"
#include "pointerTo.h"
#include "referenceCount.h"

#include <vector>

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
// 	 Class : MemoryUsagePointers
// Description : This is a list of pointers returned by a MemoryUsage
//               object in response to some query.  

//               Warning: once pointers are stored in a
//               MemoryUsagePointers object, they are
//               reference-counted, and will not be freed until the
//               MemoryUsagePointers object is freed (or clear() is
//               called on the object).  However, they may not even be
//               freed then; pointers may leak once they have been
//               added to this structure.  This is because we don't
//               store enough information in this structure to
//               correctly free the pointers that have been added.
//               Since this is intended primarily as a debugging tool,
//               this is not a major issue.
//
//               This class is just a user interface to talk about
//               pointers stored in a MemoryUsage object.  It doesn't
//               even exist when compiled with NDEBUG.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MemoryUsagePointers {
public:
  MemoryUsagePointers();
  ~MemoryUsagePointers();
  
  int get_num_pointers() const;
  ReferenceCount *get_pointer(int n) const;
  TypedObject *get_typed_pointer(int n) const;
  TypeHandle get_type(int n) const;
  string get_type_name(int n) const;
  double get_age(int n) const;

  void clear();

private:
  void add_entry(ReferenceCount *ptr, TypeHandle type, double age);

  class Entry {
  public:
    INLINE Entry(ReferenceCount *ptr, TypeHandle type, double age);
    INLINE Entry(const Entry &copy);
    INLINE void operator = (const Entry &copy);
    INLINE ~Entry();

    // We have an ordinary pointer to a type ReferenceCount, and not a
    // PT(ReferenceCount), because we can't actually delete this thing
    // (since ReferenceCount has no public destructor).  If we can't
    // delete it, we can't make a PointerTo it, since PointerTo wants
    // to be able to delete things.
    ReferenceCount *_ptr;
    TypeHandle _type;
    double _age;
  };

  typedef vector<Entry> Entries;
  Entries _entries;
  friend class MemoryUsage;
};

#include "memoryUsagePointers.I"

#endif  // NDEBUG

#endif

