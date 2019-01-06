/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodeReferenceCount.h
 * @author drose
 * @date 2006-05-01
 */

#ifndef NODEREFERENCECOUNT_H
#define NODEREFERENCECOUNT_H

#include "pandabase.h"

#include "referenceCount.h"

/**
 * This class specializes ReferenceCount to add an additional counter, called
 * node_ref_count, for the purposes of counting the number of times the object
 * is referenced by a "node", whatever that may mean in context.
 *
 * The new methods node_ref() and node_unref() automatically increment and
 * decrement the primary reference count as well.  There also exists a
 * NodePointerTo<> class to maintain the node_ref counters automatically.
 *
 * See also CachedTypedWritableReferenceCount, which is similar in principle,
 * as well as NodeCachedReferenceCount, which combines both of these.
 */
class EXPCL_PANDA_EXPRESS NodeReferenceCount : public ReferenceCount {
protected:
  INLINE NodeReferenceCount();
  INLINE NodeReferenceCount(const NodeReferenceCount &copy);
  INLINE void operator = (const NodeReferenceCount &copy);
  INLINE ~NodeReferenceCount();

PUBLISHED:
  INLINE int get_node_ref_count() const;
  INLINE void node_ref() const;
  INLINE bool node_unref() const;
  INLINE bool test_ref_count_integrity() const;
  INLINE void node_unref_only() const;

protected:
  bool do_test_ref_count_integrity() const;

private:
  mutable AtomicAdjust::Integer _node_ref_count;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "NodeReferenceCount",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

template<class RefCountType>
INLINE void node_unref_delete(RefCountType *ptr);

/**
 * This works like RefCountObj, but it inherits from NodeReferenceCount
 * instead of ReferenceCount.
 */
template<class Base>
class NodeRefCountObj : public NodeReferenceCount, public Base {
public:
  INLINE NodeRefCountObj();
  INLINE NodeRefCountObj(const Base &copy);
  ALLOC_DELETED_CHAIN(NodeRefCountObj<Base>);

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "nodeReferenceCount.I"

#endif
