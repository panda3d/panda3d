// Filename: weakNodePath.h
// Created by:  drose (29Sep04)
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

#ifndef WEAKNODEPATH_H
#define WEAKNODEPATH_H

#include "pandabase.h"

#include "nodePath.h"
#include "nodePathComponent.h"
#include "weakPointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakNodePath
// Description : This class is a wrapper around a NodePath that,
//               unlike the actual NodePath class, doesn't hold a
//               reference count to the node.  Thus the node may be
//               detached from the scene graph and destructed at any
//               time.
//
//               You can call is_valid() or was_deleted() at any time
//               to determine whether the node is still around; if it
//               is, get_node_path() will return the associated
//               NodePath.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH WeakNodePath {
public:
  INLINE WeakNodePath(const NodePath &node_path);
  INLINE WeakNodePath(const WeakNodePath &copy);
  INLINE ~WeakNodePath();

  INLINE void operator = (const NodePath &node_path);
  INLINE void operator = (const WeakNodePath &copy);

  INLINE bool is_empty() const;
  INLINE bool was_deleted() const;

  INLINE NodePath get_node_path() const;
  INLINE PandaNode *node() const;

  INLINE bool operator == (const NodePath &other) const;
  INLINE bool operator != (const NodePath &other) const;
  INLINE bool operator < (const NodePath &other) const;
  INLINE int compare_to(const NodePath &other) const;

  INLINE bool operator == (const WeakNodePath &other) const;
  INLINE bool operator != (const WeakNodePath &other) const;
  INLINE bool operator < (const WeakNodePath &other) const;
  INLINE int compare_to(const WeakNodePath &other) const;

  INLINE int get_key() const;

  void output(ostream &out) const;

private:
  WPT(NodePathComponent) _head;
  int _backup_key;
};

INLINE ostream &operator << (ostream &out, const WeakNodePath &node_path);

#include "weakNodePath.I"

#endif
