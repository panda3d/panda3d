// Filename: nodeTransitionCacheEntry.I
// Created by:  drose (20Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_graph.h"

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry::
NodeTransitionCacheEntry(NodeTransition *trans) : _trans(trans) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry::
NodeTransitionCacheEntry(const NodeTransitionCacheEntry &copy) :
  _trans(copy._trans),
  _computed(copy._computed),
  _verified(copy._verified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
operator = (const NodeTransitionCacheEntry &copy) {
  _trans = copy._trans;
  _computed = copy._computed;
  _verified = copy._verified;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::is_identity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeTransitionCacheEntry::
is_identity() const {
  return (_trans == (NodeTransition *)NULL);
  //  ||  _trans->is_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeTransitionCacheEntry::
compare_to(const NodeTransitionCacheEntry &other) const {
  bool this_ident = is_identity();
  bool other_ident = other.is_identity();
  if (this_ident && other_ident) {
    return 0;
  }
  if (this_ident) {
    return -1;
  }
  if (other_ident) {
    return 1;
  }

  // This should be guaranteed by the above logic.
  nassertr(_trans != (NodeTransition *)NULL &&
           other._trans != (NodeTransition *)NULL, false);

  return _trans->compare_to(*other._trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::generate_hash
//       Access: Public
//  Description: Adds the transition to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
generate_hash(GraphHashGenerator &hashgen) const {
  if (_trans != (NodeTransition *)NULL) {
    _trans->generate_hash(hashgen);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::clear
//       Access: Public
//  Description: Reinitializes the cache entry to its original state.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
clear() {
  _trans.clear();
  _computed.clear();
  _verified.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::set_trans
//       Access: Public
//  Description: Changes the transition associated with the cache
//               entry without changing the associated time stamps.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
set_trans(NodeTransition *trans) {
  _trans = trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::clear_trans
//       Access: Public
//  Description: Removes the transitoin associated with the cache
//               entry without changing the associated time stamps.
//               This is of limited usefulness.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
clear_trans() {
  _trans.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::has_trans
//       Access: Public
//  Description: Returns true if the cache entry has an associated
//               NodeTransition, false if it doesn't (and the
//               transition should be assumed to be identity).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeTransitionCacheEntry::
has_trans() const {
  return _trans != (NodeTransition *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::get_trans
//       Access: Public
//  Description: Returns the NodeTransition associated with the cache
//               entry, or NULL if there is no associated
//               NodeTransition (in which case it should be assumed to
//               be identity).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransition *NodeTransitionCacheEntry::
get_trans() const {
  return _trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::is_cache_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeTransitionCacheEntry::
is_cache_verified(UpdateSeq as_of) const {
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "NodeTransitionCacheEntry::is_cache_verified(" << as_of
      << "), _verified = " << _verified << ", result = "
      << (as_of <= _verified) << "\n";
  }
  return as_of <= _verified;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::is_freshly_computed
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeTransitionCacheEntry::
is_freshly_computed(UpdateSeq changed) const {
  return changed < _computed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::set_computed_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeTransitionCacheEntry::
set_computed_verified(UpdateSeq now) {
  _computed = now;
  _verified = now;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::Typecast Operator
//       Access: Public
//  Description: This typecast operator allows the cache entry to be
//               directly assigned to a NodeTransition pointer.  It's
//               primarily useful for using the code in
//               setTransitionHelpers.h interchangeably between
//               NodeTransitions and NodeTransitionCaches.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry::
operator const PT(NodeTransition) &() const {
  return _trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::get_computed
//       Access: Public
//  Description: Returns the timestamp at which this cache entry was
//               computed.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH UpdateSeq NodeTransitionCacheEntry::
get_computed() const {
  return _computed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::get_verified
//       Access: Public
//  Description: Returns the last timestamp at which this cache entry
//               was verified; i.e. when the graph was last walked up
//               to the root to ensure the cache entry is still valid.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH UpdateSeq NodeTransitionCacheEntry::
get_verified() const {
  return _verified;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::invert
//       Access: Public, Static
//  Description: Returns a new cache entry that represents the inverse
//               of a.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry NodeTransitionCacheEntry::
invert(const NodeTransitionCacheEntry &a) {
  if (a.is_identity()) {
    return a;
  }
  return NodeTransitionCacheEntry(a._trans->invert());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::compose
//       Access: Public, Static
//  Description: Returns a new cache entry that represents the
//               composition of a and b.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry NodeTransitionCacheEntry::
compose(const NodeTransitionCacheEntry &a,
        const NodeTransitionCacheEntry &b) {
  if (a.is_identity()) {
    return b;
  }
  if (b.is_identity()) {
    return a;
  }
  return NodeTransitionCacheEntry(a._trans->compose(b._trans));
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::invert_compose
//       Access: Public, Static
//  Description: Returns a new cache entry that represents the
//               composition of invert(a) and b.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry NodeTransitionCacheEntry::
invert_compose(const NodeTransitionCacheEntry &a,
               const NodeTransitionCacheEntry &b) {
  if (a.is_identity()) {
    return b;
  }
  if (b.is_identity()) {
    return NodeTransitionCacheEntry(a._trans->invert());
  }
  if (a._trans->compare_to(*b._trans) == 0) {
    return NodeTransitionCacheEntry();
  }

  PT(NodeTransition) inv = a._trans->invert();
  if (inv == (NodeTransition *)NULL) {
    return b;
  }

  // We don't care about priority for this operation.

  // This is broken!  We don't have the right the change the priority
  // on this node.  For now, leave this out, since we aren't using
  // priority on any transitions for which we're computing wrt() at
  // the moment.  Fix me soon.
  //  inv->set_priority(b._trans->get_priority());

  return NodeTransitionCacheEntry(inv->compose(b._trans));
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::cached_compose
//       Access: Public, Static
//  Description: Sets this cache entry to the result of compose(a, b),
//               as computed using the cache value as a hint.  Mark
//               the result as computed at time 'now'.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitionCacheEntry NodeTransitionCacheEntry::
cached_compose(const NodeTransitionCacheEntry &a,
               const NodeTransitionCacheEntry &cache,
               const NodeTransitionCacheEntry &b,
               UpdateSeq now) {
  if (wrt_cat.is_spam()) {
    wrt_cat.spam()
      << "Composing:\n"
      << "     a) " << a << " = " << a._computed << "\n"
      << " cache) " << cache << " = " << cache._computed << "\n"
      << "     b) " << b << " = " << b._computed << "\n";
  }

  if (cache._computed == UpdateSeq::initial() ||
      (a._computed == UpdateSeq::initial() &&
       b._computed == UpdateSeq::initial()) ||
      cache._computed < a._computed) {

    // If the cache value has not yet been computed, or if the values
    // in a and b are both empty, or if the cache is older than the
    // source value (a), discard the cache and compute the composition
    // directly.
    NodeTransitionCacheEntry result = compose(a, b);
    result.set_computed_verified(now);

    if (wrt_cat.is_spam()) {
      wrt_cat.spam() << "computed result is " << result << "\n";
    }

    return result;

  } else {
    // Otherwise, the cache value is recent and we can simply use it
    // directly.
    NodeTransitionCacheEntry result = cache;
    result._verified = now;

    if (wrt_cat.is_spam()) {
      wrt_cat.spam() << "cached result is " << result << "\n";
    }

    return result;
  }
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitionCacheEntry &e) {
  e.output(out);
  return out;
}
