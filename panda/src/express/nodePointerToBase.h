/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePointerToBase.h
 * @author drose
 * @date 2005-05-07
 */

#ifndef NODEPOINTERTOBASE_H
#define NODEPOINTERTOBASE_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "memoryUsage.h"
#include "config_express.h"

/**
 * This is similar to PointerToBase, but it manages objects of type
 * NodeReferenceCount or NodeCachedReferenceCount, and it updates the
 * node_ref_count instead of the regular ref_count.  It is intended for use
 * only in PandaNode, to hold a pointer to RenderState and TransformState,
 * although it could be used by any object that wanted to maintain a separate
 * reference count for reporting purposes.
 */
template <class T>
class NodePointerToBase : public PointerToVoid {
public:
  typedef T To;

protected:
  INLINE NodePointerToBase(To *ptr);
  INLINE NodePointerToBase(const NodePointerToBase<T> &copy);
  INLINE ~NodePointerToBase();
  INLINE NodePointerToBase(NodePointerToBase<T> &&from) noexcept;

  INLINE void reassign(NodePointerToBase<To> &&from) noexcept;

  void reassign(To *ptr);
  INLINE void reassign(const NodePointerToBase<To> &copy);

  // No assignment or retrieval functions are declared in NodePointerToBase,
  // because we will have to specialize on const vs.  non-const later.

PUBLISHED:
  INLINE void clear();

  void output(std::ostream &out) const;
};

template<class T>
INLINE std::ostream &operator <<(std::ostream &out, const NodePointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "nodePointerToBase.I"

#endif
