/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakNodePath.I
 * @author drose
 * @date 2004-09-29
 */

/**
 *
 */
INLINE WeakNodePath::
WeakNodePath(const NodePath &node_path) :
  _head(node_path._head),
  _backup_key(0)
{
}

/**
 *
 */
INLINE WeakNodePath::
WeakNodePath(const WeakNodePath &copy) :
  _head(copy._head),
  _backup_key(copy._backup_key)
{
}

/**
 *
 */
INLINE WeakNodePath::
~WeakNodePath() {
}

/**
 *
 */
INLINE void WeakNodePath::
operator = (const NodePath &node_path) {
  _head = node_path._head;
  _backup_key = 0;
}

/**
 *
 */
INLINE void WeakNodePath::
operator = (const WeakNodePath &copy) {
  _head = copy._head;
  _backup_key = copy._backup_key;
}

/**
 * Sets this NodePath to the empty NodePath.  It will no longer point to any
 * node.
 */
INLINE void WeakNodePath::
clear() {
  _head.clear();
  _backup_key = 0;
}

/**
 * Returns true if this NodePath points to a valid, non-null node.
 */
INLINE WeakNodePath::
operator bool () const {
  return _head.is_valid_pointer();
}

/**
 * Returns true if the NodePath contains no nodes, or if it has been deleted.
 */
INLINE bool WeakNodePath::
is_empty() const {
  return _head == nullptr || _head.was_deleted();
}

/**
 * Returns true if the NodePath we were referencing has been quietly deleted
 * outside of the WeakNodePath.
 */
INLINE bool WeakNodePath::
was_deleted() const {
  return _head != nullptr && _head.was_deleted();
}

/**
 * Returns the NodePath held within this object, or an empty NodePath with the
 * error flag set if the object was deleted.
 */
INLINE NodePath WeakNodePath::
get_node_path() const {
  NodePath result;
  result._head = _head.lock();
  if (!_head.is_null() && result._head == nullptr) {
    result._error_type = NodePath::ET_fail;
  }
  return result;
}

/**
 * Returns the PandaNode held within this object, or nullptr if the object was
 * deleted.
 */
INLINE PT(PandaNode) WeakNodePath::
node() const {
  if (auto head = _head.lock()) {
    return head->get_node();
  } else {
    return nullptr;
  }
}

/**
 * Returns true if the two paths are equivalent; that is, if they contain the
 * same list of nodes in the same order.
 */
INLINE bool WeakNodePath::
operator == (const NodePath &other) const {
  return _head.get_orig() == other._head && !_head.was_deleted();
}

/**
 * Returns true if the two paths are not equivalent.
 */
INLINE bool WeakNodePath::
operator != (const NodePath &other) const {
  return !operator == (other);
}

/**
 * Returns true if this NodePath sorts before the other one, false otherwise.
 * The sorting order of two nonequivalent NodePaths is consistent but
 * undefined, and is useful only for storing NodePaths in a sorted container
 * like an STL set.
 */
INLINE bool WeakNodePath::
operator < (const NodePath &other) const {
  return _head.owner_before(other._head);
}

/**
 * Returns a number less than zero if this NodePath sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 *
 * Two NodePaths are considered equivalent if they consist of exactly the same
 * list of nodes in the same order.  Otherwise, they are different; different
 * NodePaths will be ranked in a consistent but undefined ordering; the
 * ordering is useful only for placing the NodePaths in a sorted container
 * like an STL set.
 */
INLINE int WeakNodePath::
compare_to(const NodePath &other) const {
  if (operator != (other)) {
    return _head.owner_before(other._head) ? -1 : 1;
  }
  return 0;
}

/**
 * Returns true if the two paths are equivalent; that is, if they contain the
 * same list of nodes in the same order.
 */
INLINE bool WeakNodePath::
operator == (const WeakNodePath &other) const {
  return !_head.owner_before(other._head) && !other._head.owner_before(_head);
}

/**
 * Returns true if the two paths are not equivalent.
 */
INLINE bool WeakNodePath::
operator != (const WeakNodePath &other) const {
  return _head.owner_before(other._head) || other._head.owner_before(_head);
}

/**
 * Returns true if this WeakNodePath sorts before the other one, false
 * otherwise.  The sorting order of two nonequivalent WeakNodePaths is
 * consistent but undefined, and is useful only for storing WeakNodePaths in a
 * sorted container like an STL set.
 */
INLINE bool WeakNodePath::
operator < (const WeakNodePath &other) const {
  return _head.owner_before(other._head);
}

/**
 * Returns a number less than zero if this WeakNodePath sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 *
 * Two WeakNodePaths are considered equivalent if they consist of exactly the
 * same list of nodes in the same order.  Otherwise, they are different;
 * different WeakNodePaths will be ranked in a consistent but undefined
 * ordering; the ordering is useful only for placing the WeakNodePaths in a
 * sorted container like an STL set.
 */
INLINE int WeakNodePath::
compare_to(const WeakNodePath &other) const {
  return other._head.owner_before(_head) - _head.owner_before(other._head);
}

/**
 * Returns the same values as NodePath::get_key().
 */
INLINE int WeakNodePath::
get_key() const {
  if (auto head = _head.lock()) {
    _backup_key = head->get_key();
  }
  return _backup_key;
}

INLINE std::ostream &operator << (std::ostream &out, const WeakNodePath &node_path) {
  node_path.output(out);
  return out;
}
