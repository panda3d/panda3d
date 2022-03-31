/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakNodePath.h
 * @author drose
 * @date 2004-09-29
 */

#ifndef WEAKNODEPATH_H
#define WEAKNODEPATH_H

#include "pandabase.h"

#include "nodePath.h"
#include "nodePathComponent.h"
#include "weakPointerTo.h"

/**
 * This class is a wrapper around a NodePath that, unlike the actual NodePath
 * class, doesn't hold a reference count to the node.  Thus the node may be
 * detached from the scene graph and destructed at any time.
 *
 * You can call is_valid() or was_deleted() at any time to determine whether
 * the node is still around; if it is, get_node_path() will return the
 * associated NodePath.
 */
class EXPCL_PANDA_PGRAPH WeakNodePath {
PUBLISHED:
  INLINE WeakNodePath(const NodePath &node_path);
  INLINE WeakNodePath(const WeakNodePath &copy);
  INLINE ~WeakNodePath();

  INLINE void operator = (const NodePath &node_path);
  INLINE void operator = (const WeakNodePath &copy);

  INLINE void clear();

  INLINE operator bool () const;
  INLINE bool is_empty() const;
  INLINE bool was_deleted() const;

  INLINE NodePath get_node_path() const;
  INLINE PT(PandaNode) node() const;

  INLINE bool operator == (const NodePath &other) const;
  INLINE bool operator != (const NodePath &other) const;
  INLINE bool operator < (const NodePath &other) const;
  INLINE int compare_to(const NodePath &other) const;

  INLINE bool operator == (const WeakNodePath &other) const;
  INLINE bool operator != (const WeakNodePath &other) const;
  INLINE bool operator < (const WeakNodePath &other) const;
  INLINE int compare_to(const WeakNodePath &other) const;

  INLINE int get_key() const;

  void output(std::ostream &out) const;

private:
  WPT(NodePathComponent) _head;
  mutable int _backup_key;

  friend class NodePath;
};

INLINE std::ostream &operator << (std::ostream &out, const WeakNodePath &node_path);

#include "weakNodePath.I"

#endif
