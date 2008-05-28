// Filename: weakNodePath.I
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


////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE WeakNodePath::
WeakNodePath(const NodePath &node_path) :
  _head(node_path._head),
  _backup_key(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE WeakNodePath::
WeakNodePath(const WeakNodePath &copy) :
  _head(copy._head),
  _backup_key(copy._backup_key)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE WeakNodePath::
~WeakNodePath() {
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator = 
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void WeakNodePath::
operator = (const NodePath &node_path) {
  _head = node_path._head;
  _backup_key = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator = 
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void WeakNodePath::
operator = (const WeakNodePath &copy) {
  _head = copy._head;
  _backup_key = copy._backup_key;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::is_empty
//       Access: Public
//  Description: Returns true if the NodePath contains no nodes, or if
//               it has been deleted.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
is_empty() const {
  return _head == (NodePathComponent *)NULL || _head.was_deleted();
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::was_deleted
//       Access: Public
//  Description: Returns true if the NodePath we were referencing has
//               been quietly deleted outside of the WeakNodePath.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
was_deleted() const {
  return _head != (NodePathComponent *)NULL && _head.was_deleted();
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::get_node_path
//       Access: Public
//  Description: Returns the NodePath held within this object.
////////////////////////////////////////////////////////////////////
INLINE NodePath WeakNodePath::
get_node_path() const {
  nassertr_always(!was_deleted(), NodePath::fail());
  NodePath result;
  result._head = _head;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::node
//       Access: Public
//  Description: Returns the PandaNode held within this object.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *WeakNodePath::
node() const {
  nassertr_always(!is_empty(), (PandaNode *)NULL);
  return _head->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator ==
//       Access: Published
//  Description: Returns true if the two paths are equivalent; that
//               is, if they contain the same list of nodes in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator == (const NodePath &other) const {
  return _head == other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator !=
//       Access: Published
//  Description: Returns true if the two paths are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator != (const NodePath &other) const {
  return _head != other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator <
//       Access: Published
//  Description: Returns true if this NodePath sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent NodePaths is consistent but undefined,
//               and is useful only for storing NodePaths in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator < (const NodePath &other) const {
  return _head < other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this NodePath
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two NodePaths are considered equivalent if they
//               consist of exactly the same list of nodes in the same
//               order.  Otherwise, they are different; different
//               NodePaths will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the NodePaths in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE int WeakNodePath::
compare_to(const NodePath &other) const {
  if (_head != other._head) {
    return _head < other._head ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator ==
//       Access: Published
//  Description: Returns true if the two paths are equivalent; that
//               is, if they contain the same list of nodes in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator == (const WeakNodePath &other) const {
  return _head == other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator !=
//       Access: Published
//  Description: Returns true if the two paths are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator != (const WeakNodePath &other) const {
  return _head != other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::operator <
//       Access: Published
//  Description: Returns true if this WeakNodePath sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent WeakNodePaths is consistent but undefined,
//               and is useful only for storing WeakNodePaths in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool WeakNodePath::
operator < (const WeakNodePath &other) const {
  return _head < other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this WeakNodePath
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two WeakNodePaths are considered equivalent if they
//               consist of exactly the same list of nodes in the same
//               order.  Otherwise, they are different; different
//               WeakNodePaths will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the WeakNodePaths in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE int WeakNodePath::
compare_to(const WeakNodePath &other) const {
  if (_head != other._head) {
    return _head < other._head ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::get_key
//       Access: Public
//  Description: Returns the same values as NodePath::get_key().
////////////////////////////////////////////////////////////////////
INLINE int WeakNodePath::
get_key() const {
  if (is_empty() || was_deleted()) {
    return _backup_key;
  }
  ((WeakNodePath *)this)->_backup_key = _head->get_key();
  return _backup_key;
}

INLINE ostream &operator << (ostream &out, const WeakNodePath &node_path) {
  node_path.output(out);
  return out;
}
