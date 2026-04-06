/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cachedTypedWritableReferenceCount.h
 * @author drose
 * @date 2005-01-25
 */

#ifndef CACHEDTYPEDWRITABLEREFERENCECOUNT_H
#define CACHEDTYPEDWRITABLEREFERENCECOUNT_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"

/**
 * This is a special extension to ReferenceCount that includes dual reference
 * counts: the standard reference count number, which includes all references
 * to the object, and a separate number (the cache reference count) that
 * counts the number of references to the object just within its cache alone.
 * When get_ref_count() == get_cache_ref_count(), the object is not referenced
 * outside the cache.
 *
 * The cache refs must be explicitly maintained; there is no PointerTo<> class
 * to maintain the cache reference counts automatically.  The cache reference
 * count is automatically included in the overall reference count: calling
 * cache_ref() and cache_unref() automatically calls ref() and unref().
 */
class EXPCL_PANDA_PUTIL CachedTypedWritableReferenceCount : public TypedWritableReferenceCount {
protected:
  INLINE CachedTypedWritableReferenceCount();
  INLINE CachedTypedWritableReferenceCount(const CachedTypedWritableReferenceCount &copy);
  INLINE void operator = (const CachedTypedWritableReferenceCount &copy);
  INLINE ~CachedTypedWritableReferenceCount();

PUBLISHED:
  INLINE int get_cache_ref_count() const;
  INLINE void cache_ref() const;
  INLINE bool cache_unref() const;
  INLINE bool test_ref_count_integrity() const;

  MAKE_PROPERTY(cache_ref_count, get_cache_ref_count);

public:
  INLINE void cache_ref_only() const;

protected:
  INLINE void cache_unref_only() const;
  bool do_test_ref_count_integrity() const;

private:
  mutable AtomicAdjust::Integer _cache_ref_count;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "CachedTypedWritableReferenceCount",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

template<class RefCountType>
INLINE void cache_unref_delete(RefCountType *ptr);

#include "cachedTypedWritableReferenceCount.I"

#endif
