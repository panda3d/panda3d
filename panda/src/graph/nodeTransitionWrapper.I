// Filename: nodeTransitionWrapper.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeRelation.h"
#include "nodeTransitionCacheEntry.h"

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionWrapper::
NodeTransitionWrapper(TypeHandle handle) : _handle(handle) {
  nassertv(_handle != TypeHandle::none());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionWrapper::
NodeTransitionWrapper(const NodeTransitionWrapper &copy) :
  _handle(copy._handle),
  _entry(copy._entry)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
operator = (const NodeTransitionWrapper &copy) {
  _handle = copy._handle;
  _entry = copy._entry;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               NodeTransitionWrapper ready to access the same type
//               of NodeTransition as the other.
////////////////////////////////////////////////////////////////////
INLINE NodeTransitionWrapper NodeTransitionWrapper::
init_from(const NodeTransitionWrapper &other) {
  return NodeTransitionWrapper(other._handle);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypeHandle NodeTransitionWrapper::
get_handle() const {
  return _handle;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::has_trans
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransitionWrapper::
has_trans() const {
  return _entry.has_trans();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::get_trans
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransition *NodeTransitionWrapper::
get_trans() const {
  return _entry.get_trans();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::is_identity
//       Access: Public
//  Description: Returns true if the wrapper represents an identity
//               transition.
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransitionWrapper::
is_identity() const {
  return _entry.is_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int NodeTransitionWrapper::
compare_to(const NodeTransitionWrapper &other) const {
  return _entry.compare_to(other._entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::make_identity
//       Access: Public
//  Description: Resets the wrapper to the identity transition.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
make_identity() {
  _entry.clear_trans();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::extract_from
//       Access: Public
//  Description: Sets the wrapper to the transition contained on the
//               arc.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
extract_from(const NodeRelation *arc) {
  nassertv(arc != (NodeRelation *)NULL);
  nassertv(_handle != TypeHandle::none());
  _entry.set_trans(arc->get_transition(_handle));
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::store_to
//       Access: Public
//  Description: Stores the transition in the wrapper to the arc.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
store_to(NodeRelation *arc) const {
  nassertv(arc != (NodeRelation *)NULL);
  arc->set_transition(_handle, _entry.get_trans());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::compose_in_place
//       Access: Public
//  Description: Sets this transition to the composition of this
//               transition and the following one.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
compose_in_place(const NodeTransitionWrapper &other) {
  nassertv(_handle == other._handle);
  _entry = NodeTransitionCacheEntry::compose(_entry, other._entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::invert_in_place
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
invert_in_place() {
  _entry = NodeTransitionCacheEntry::invert(_entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::invert_compose_in_place
//       Access: Public
//  Description: Sets this transition to the composition of this
//               transition and the following one.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
invert_compose_in_place(const NodeTransitionWrapper &other) {
  _entry = NodeTransitionCacheEntry::invert_compose(_entry, other._entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::extract_from_cache
//       Access: Public
//  Description: Sets this transition to the value indicated in the
//               cache on the arc.  Also returns the arc's top_subtree
//               indication.
////////////////////////////////////////////////////////////////////
INLINE Node *NodeTransitionWrapper::
extract_from_cache(const NodeRelation *arc) {
  nassertr(_handle != TypeHandle::none(), NULL);

  if (arc->_net_transitions == (NodeTransitionCache *)NULL) {
    _entry.clear();
  } else {
    arc->_net_transitions->lookup_entry(_handle, _entry);
  }
  return arc->_top_subtree;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::store_to_cache
//       Access: Public
//  Description: Stores this transition into the arc's cache, and
//               updates the arc's top_subtree.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
store_to_cache(NodeRelation *arc, Node *top_subtree) {
  arc->_top_subtree = top_subtree;
  if (arc->_net_transitions == (NodeTransitionCache *)NULL) {
    arc->_net_transitions = new NodeTransitionCache;

  } else if (arc->_net_transitions->get_ref_count() > 1) {
    // Copy-on-write.
    arc->_net_transitions = new NodeTransitionCache(*arc->_net_transitions);
  }

  arc->_net_transitions->store_entry(_handle, _entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::is_cache_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransitionWrapper::
is_cache_verified(UpdateSeq now) const {
  return _entry.is_cache_verified(now);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::set_computed_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
set_computed_verified(UpdateSeq now) {
  _entry.set_computed_verified(now);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::cached_compose
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NodeTransitionWrapper::
cached_compose(const NodeTransitionWrapper &cache, 
	       const NodeTransitionWrapper &value,
	       UpdateSeq now) {
  _entry = 
    NodeTransitionCacheEntry::cached_compose(_entry, cache._entry,
					     value._entry, now);
}

////////////////////////////////////////////////////////////////////
//     Function: get_transition_into
//  Description: This external template function is handy for
//               extracting the transition (of a known type) from the
//               wrapper.  If the transition exists, it is
//               automatically downcasted to the correct type and
//               stored in the pointer given in the first parameter,
//               and the return value is true.  If the transition does
//               not exist, the pointer is filled with NULL and the
//               return value is false.
////////////////////////////////////////////////////////////////////
template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeTransitionWrapper &trans) {
  NodeTransition *nt = trans.get_trans();
  if (nt == (NodeTransition *)NULL) {
    ptr = (Transition *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
