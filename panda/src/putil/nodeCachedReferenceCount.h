// Filename: nodeCachedReferenceCount.h
// Created by:  drose (07May05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef NODECACHEDREFERENCECOUNT_H
#define NODECACHEDREFERENCECOUNT_H

#include "pandabase.h"

#include "cachedTypedWritableReferenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : NodeCachedReferenceCount
// Description : This class further specializes
//               CachedTypedWritableReferenceCount to also add a
//               node_ref_count, for the purposes of counting the
//               number of times the object is referenced by a "node",
//               presumably a PandaNode.
//
//               The intended design is to use this as a base class
//               for RenderState and TransformState, both of which are
//               held by PandaNodes, and also have caches which are
//               independently maintained.  By keeping track of how
//               many nodes hold a pointer to a particular object, we
//               can classify each object into node-referenced,
//               cache-referenced, or other, which is primarily useful
//               for PStats reporting.
//
//               As with CachedTypedWritableReferenceCount's
//               cache_ref() and cache_unref(), the new methods
//               node_ref() and node_unref() automatically increment
//               and decrement the primary reference count as well.
//               In this case, however, there does exist a
//               NodePointerTo<> class to maintain the node_ref
//               counters automatically.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeCachedReferenceCount : public CachedTypedWritableReferenceCount {
protected:
  INLINE NodeCachedReferenceCount();
  INLINE NodeCachedReferenceCount(const NodeCachedReferenceCount &copy);
  INLINE void operator = (const NodeCachedReferenceCount &copy);
  INLINE ~NodeCachedReferenceCount();

PUBLISHED:
  INLINE int get_node_ref_count() const;
  INLINE int node_ref() const;
  INLINE int node_unref() const;
  INLINE bool test_ref_count_integrity() const;

  enum Referenced {
    R_node  = 0x001,
    R_cache = 0x002,
  };

  INLINE int get_referenced_bits() const;

protected:
  bool do_test_ref_count_integrity() const;
  
private:
  int _node_ref_count;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    CachedTypedWritableReferenceCount::init_type();
    register_type(_type_handle, "NodeCachedReferenceCount",
                  CachedTypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

template<class RefCountType>
INLINE void node_unref_delete(RefCountType *ptr);

#include "nodeCachedReferenceCount.I"

#endif  

