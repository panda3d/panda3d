// Filename: nodePointerToBase.h
// Created by:  drose (07May05)
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

#ifndef NODEPOINTERTOBASE_H
#define NODEPOINTERTOBASE_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "memoryUsage.h"
#include "config_express.h"

////////////////////////////////////////////////////////////////////
//       Class : NodePointerToBase
// Description : This is similar to PointerToBase, but it manages
//               objects of type NodeReferenceCount or
//               NodeCachedReferenceCount, and it updates the
//               node_ref_count instead of the regular ref_count.  It
//               is intended for use only in PandaNode, to hold a
//               pointer to RenderState and TransformState, although
//               it could be used by any object that wanted to
//               maintain a separate reference count for reporting
//               purposes.
////////////////////////////////////////////////////////////////////
template <class T>
class NodePointerToBase : public PointerToVoid {
public:
  typedef T To;

protected:
  INLINE NodePointerToBase(To *ptr);
  INLINE NodePointerToBase(const NodePointerToBase<T> &copy);
  INLINE ~NodePointerToBase();

#ifdef USE_MOVE_SEMANTICS
  INLINE NodePointerToBase(NodePointerToBase<T> &&from) NOEXCEPT;
  INLINE void reassign(NodePointerToBase<To> &&from) NOEXCEPT;
#endif

  void reassign(To *ptr);
  INLINE void reassign(const NodePointerToBase<To> &copy);

  // No assignment or retrieval functions are declared in
  // NodePointerToBase, because we will have to specialize on const
  // vs. non-const later.

PUBLISHED:
  INLINE void clear();

  void output(ostream &out) const;
};

template<class T>
INLINE ostream &operator <<(ostream &out, const NodePointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "nodePointerToBase.I"

#endif
