// Filename: nodeRelation.cxx
// Created by:  drose (26Oct98)
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

#include "nodeRelation.h"
#include "node.h"
#include "config_graph.h"

#include <boundingSphere.h>
#include <notify.h>

#include <algorithm>

TypeHandle NodeRelation::_type_handle;
TypeHandle NodeRelation::_stashed_type_handle;

Factory<NodeRelation> *NodeRelation::_factory = NULL;

typedef pmap<TypeHandle, UpdateSeq> LastGraphUpdate;
static LastGraphUpdate *last_graph_update_map = NULL;

////////////////////////////////////////////////////////////////////
//     Function: last_graph_update
//  Description: Returns a modifiable reference to the sequence number
//               indicating the last update to the indicated type of
//               graph.
////////////////////////////////////////////////////////////////////
UpdateSeq &
last_graph_update(TypeHandle graph_type) {
#ifndef NDEBUG
  static UpdateSeq initial;
  nassertr(last_graph_update_map != (LastGraphUpdate *)NULL, initial);
#endif
  return (*last_graph_update_map)[graph_type];
}

////////////////////////////////////////////////////////////////////
//     Function: init_last_graph_update
//  Description: Initializes the last_graph_update map.  This call
//               must be made before last_graph_update() can be safely
//               called.
////////////////////////////////////////////////////////////////////
void
init_last_graph_update() {
  if (last_graph_update_map == NULL) {
    last_graph_update_map = new LastGraphUpdate;
  }
}

// Following are a handful of local template functions that provide
// support for manipulating the list of arcs on nodes.  They are
// template functions because they work as well on UpRelationPointers
// as as they do on DownRelationPointers, which are slightly different
// things (DownRelationPointers are reference-counting).

////////////////////////////////////////////////////////////////////
//     Function: verify_arc_list
//  Description: A local template function that verifies that the list
//               of arcs (either UpRelationPointers or
//               DownRelationPointers) is correctly sorted, if
//               paranoid_graph is set.  Otherwise, it does nothing.
////////////////////////////////////////////////////////////////////
#ifdef NDEBUG
template<class Iterator>
INLINE_GRAPH void
verify_arc_list(Iterator, Iterator) {
}
#else  // NDEBUG
template<class Iterator>
static void
verify_arc_list(Iterator begin, Iterator end) {
  if (paranoid_graph) {
    if (begin < end) {
      Iterator i = begin;
      int sort = (*i)->get_sort();
      ++i;
      while (i < end) {
        nassertv(sort <= (*i)->get_sort());
        sort = (*i)->get_sort();
        ++i;
      }
    }
  }
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: find_insert_position
//  Description: A local template function that performs a binary
//               search on the arcs list (either UpRelationPointers or
//               DownRelationPointers) and finds the place to insert
//               the indicated arc.
//
//               This place will be at the end of the similarly-sorted
//               arcs.
////////////////////////////////////////////////////////////////////
template<class Iterator>
static Iterator
r_find_insert_position(Iterator begin, Iterator end, NodeRelation *arc) {
  if (begin == end) {
    // The list is empty; the insert position is the end of the list.
    return end;
  }

  Iterator center = begin + (end - begin) / 2;
  nassertr(center < end, end);

  if ((*center)->get_sort() > arc->get_sort()) {
    // Insert before the center.
    return r_find_insert_position(begin, center, arc);

  } else { // (*center)->get_sort() <= arc->get_sort();
    // Insert after the center.
    return r_find_insert_position(center + 1, end, arc);
  }
}

template<class Iterator>
static Iterator
find_insert_position(Iterator begin, Iterator end, NodeRelation *arc) {
  Iterator result = r_find_insert_position(begin, end, arc);
#ifndef NDEBUG
  // Verify the result.
  if (paranoid_graph) {
    // If there is a node before the indicated position, it must have
    // a sort value either less than or equal to this arc's value.
    if (begin < result) {
      nassertr((*(result - 1))->get_sort() <= arc->get_sort(), result);
    }

    // If there is a node after the indicated position, it must have a
    // sort value greater than this arc's value.
    if (result < end) {
      nassertr((*result)->get_sort() > arc->get_sort(), result);
    }
  }
#endif
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: find_arc
//  Description: A local template function that performs a binary
//               search on the arcs list (either UpRelationPointers or
//               DownRelationPointers) and finds the arc's position
//               within the list.  It returns end if the arc is not
//               within the list.
////////////////////////////////////////////////////////////////////
template<class Iterator>
static Iterator
find_arc(Iterator begin, Iterator end, NodeRelation *arc) {
  if (begin == end) {
    // The list is empty; the arc is not on the list.
    return end;
  }

  Iterator center = begin + (end - begin) / 2;
  nassertr(center < end, end);

  if ((*center)->get_sort() > arc->get_sort()) {
    // It must be before the center.
    return find_arc(begin, center, arc);

  } else if ((*center)->get_sort() < arc->get_sort()) {
    // It must be after the center.
    return find_arc(center + 1, end, arc);

  } else {
    // The center's sort matches the arc's sort.  It could be either
    // before or after the center.  First try after.
    Iterator i = center;
    while (i < end && (*i)->get_sort() == arc->get_sort()) {
      if ((*i) == arc) {
        return i;
      }
      ++i;
    }

    // No, try before.
    i = center;
    --i;
    while (i >= begin && (*i)->get_sort() == arc->get_sort()) {
      if ((*i) == arc) {
        return i;
      }
      --i;
    }

    // No such arc!
    return end;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: internal_insert_arc
//  Description: A local template function that inserts the arc into
//               its appropriate place in the list and returns true if
//               successful, false if there was some kind of error.
////////////////////////////////////////////////////////////////////
template<class ArcList>
static bool
internal_insert_arc(ArcList &alist, NodeRelation *arc) {
  nassertr(arc != (NodeRelation *)NULL, false);

  TYPENAME ArcList::iterator position =
    find_insert_position(alist.begin(), alist.end(), arc);
  nassertr(position >= alist.begin() && position <= alist.end(), false);

  /*
  if (graph_cat.is_debug()) {
    if (position == alist.end()) {
      graph_cat.debug()
        << "Inserting " << *arc << " at end\n";
    } else {
      graph_cat.debug()
        << "Inserting " << *arc << " before " << *(*position) << "\n";
    }
  }
  */

  alist.insert(position, arc);
  verify_arc_list(alist.begin(), alist.end());

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: internal_remove_arc
//  Description: A local template function that removes the arc from
//               its place in the list and returns true if successful,
//               false if there was some kind of error.
////////////////////////////////////////////////////////////////////
template<class ArcList>
static bool
internal_remove_arc(ArcList &alist, NodeRelation *arc) {
  nassertr(arc != (NodeRelation *)NULL, false);

  TYPENAME ArcList::iterator position =
    find_arc(alist.begin(), alist.end(), arc);
  nassertr(position >= alist.begin() && position <= alist.end(), false);
  nassertr(position != alist.end(), false);

  /*
  if (graph_cat.is_debug()) {
    TYPENAME ArcList::iterator next = position + 1;
    if (next == list.end()) {
      graph_cat.debug()
        << "Removing " << *arc << " from end\n";
    } else {
      graph_cat.debug()
        << "Removing " << *arc << " from before " << *(*next) << "\n";
    }
  }
  */

  alist.erase(position);
  verify_arc_list(alist.begin(), alist.end());

  return true;
}




////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Constructor
//       Access: Public
//  Description: Creates a new arc of the scene graph, and immediately
//               attaches it.
////////////////////////////////////////////////////////////////////
NodeRelation::
NodeRelation(Node *parent, Node *to, int sort, TypeHandle graph_type) :
  _parent(parent), _child(to), _sort(sort),
  _graph_type(graph_type), _num_transitions(0)
{
  _top_subtree = NULL;
  _attached = false;
  _parent_ref = 0;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for passing through the factory to
//               create an arc based on a particular type using
//               create_typed_arc(), below.  You shouldn't, in
//               general, attempt to create an unattached arc.
////////////////////////////////////////////////////////////////////
NodeRelation::
NodeRelation(TypeHandle graph_type) :
  _graph_type(graph_type)
{
  _parent = NULL;
  _child = NULL;
  _sort = 0;
  _top_subtree = NULL;
  _attached = false;
  _parent_ref = 0;
  _num_transitions = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Copy Constructor
//       Access: Private
//  Description: It's not legal to copy a NodeRelation.
////////////////////////////////////////////////////////////////////
NodeRelation::
NodeRelation(const NodeRelation &) {
  graph_cat.error()
    << "NodeRelation copy constructor called!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Copy Assignment Operator
//       Access: Private
//  Description: It's not legal to copy a NodeRelation.
////////////////////////////////////////////////////////////////////
void NodeRelation::
operator = (const NodeRelation &) {
  graph_cat.error()
    << "NodeRelation copy assignment operator called!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeRelation::
~NodeRelation() {
  // An attached arc should never be deleted.  If this assertion
  // fails, it's most likely that someone attempted to explicitly
  // delete an arc.  You should use remove_arc() instead.
  nassertv(!_attached);

  // Similarly, we shouldn't be deleting arcs with a nonzero
  // parent_ref.  This means someone lost a reference count somewhere,
  // or someone was sloppy calling ref_parent()/unref_parent().
  nassertv(_parent_ref == 0);

  _transitions.remove_all_from_arc(this);
}


////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::output
//       Access: Public
//  Description: Writes a brief description of the arc to the
//               indicated output stream.  This function is called by
//               the << operator.
////////////////////////////////////////////////////////////////////
void NodeRelation::
output(ostream &out) const {
  if (_parent == (Node*)NULL) {
    out << "(null)";
  } else {
    out << *_parent;
  }
  out << " -> ";
  if (_child == (Node*)NULL) {
    out << "(null)";
  } else {
    out << *_child;
  }

  if (!_attached) {
    out << " (unattached)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::ref_parent
//       Access: Public
//  Description: Normally, the arc keeps a reference count to its
//               child node, but not to its parent node.  This allows
//               the arc (and all of its children) to be destructed
//               when its parent node destructs.
//
//               Sometimes this behavior is not desirable, and you'd
//               like to have an arc keep a reference count to its
//               parent.  This is particularly true when you have a
//               reference-counting pointer to the arc in some other
//               context, and you'd like to ensure that its parent and
//               child node pointers are always valid while you keep
//               the pointer.
//
//               In this case, you should call ref_parent() to
//               increment the reference count on the parent node.  Be
//               sure to call unref_parent() explicitly when you lose
//               your reference to the arc, or you will leak reference
//               counts.
//
//               Note that if the parent of the arc changes while
//               ref_parent() is held, the old parent will
//               automatically be dereferenced and the new parent will
//               be referenced instead, as if it were a normal
//               reference-counting pointer.
////////////////////////////////////////////////////////////////////
void NodeRelation::
ref_parent() {
  nassertv(_parent_ref >= 0);
  if (_parent_ref == 0 && _attached) {
    // If we are the first to request a reference count on the parent,
    // actually increment it now.
    _parent->ref();
  }
  _parent_ref++;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::unref_parent
//       Access: Public
//  Description: Removes the reference count on the arc's parent node
//               and allows the arc to be deleted normally.  See
//               ref_parent().
////////////////////////////////////////////////////////////////////
void NodeRelation::
unref_parent() {
  nassertv(_parent_ref > 0);

  _parent_ref--;
  if (_parent_ref == 0 && _attached) {
    // If we are the last to request a reference count on the parent,
    // actually decrement it now.
    unref_delete(_parent);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::copy_transitions_from
//       Access: Public
//  Description: Copies all of the transitions stored on the other arc
//               to this arc.  Any existing transitions on this arc,
//               for which there was not a corresponding transition of
//               the same type on the other arc, are left undisturbed.
////////////////////////////////////////////////////////////////////
void NodeRelation::
copy_transitions_from(const NodeRelation *arc) {
  copy_transitions_from(arc->_transitions);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::compose_transitions_from
//       Access: Public
//  Description: Similar to copy_transitions_from(), except that if
//               the same type of transition exists on both arcs, the
//               composition of the two is stored.  The result
//               represents the same set of transitions that would
//               result from composing the two individual sets of
//               transitions.
////////////////////////////////////////////////////////////////////
void NodeRelation::
compose_transitions_from(const NodeRelation *arc) {
  compose_transitions_from(arc->_transitions);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::copy_transitions_from
//       Access: Public
//  Description: Copies all of the transitions stored in the indicated
//               set to this arc.  Any existing transitions on this
//               arc, for which there was not a corresponding
//               transition of the same type on the other arc, are
//               left undisturbed.
////////////////////////////////////////////////////////////////////
void NodeRelation::
copy_transitions_from(const NodeTransitions &trans) {
  if (!trans.is_empty()) {
    _transitions.copy_transitions_from(trans, this);

    // Now mark that *all* transitions have changed, even though many
    // of them might not have, because we're not really sure.
    _net_transitions.clear();
    NodeTransitions::const_iterator ti;
    for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
      changed_transition((*ti).first);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::compose_transitions_from
//       Access: Public
//  Description: Similar to copy_transitions_from(), except that if
//               the same type of transition exists in both places,
//               the composition of the two is stored.  The result
//               represents the same set of transitions that would
//               result from composing the two individual sets of
//               transitions.
////////////////////////////////////////////////////////////////////
void NodeRelation::
compose_transitions_from(const NodeTransitions &trans) {
  if (!trans.is_empty()) {
    _transitions.compose_transitions_from(trans, this);

    // Now mark that *all* transitions have changed, even though many
    // of them might not have, because we're not really sure.
    _net_transitions.clear();
    NodeTransitions::const_iterator ti;
    for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
      changed_transition((*ti).first);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::adjust_all_proriorities
//       Access: Public
//  Description: Adds the indicated adjustment amount (which may be
//               negative) to the priority for all transitions on the
//               arc.  If the priority would drop below zero, it is
//               set to zero.
////////////////////////////////////////////////////////////////////
void NodeRelation::
adjust_all_priorities(int adjustment) {
  _net_transitions.clear();
  _transitions.adjust_all_priorities(adjustment, this);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::sub_render_trans
//       Access: Public
//  Description: Calls sub_render() on each transition assigned to the
//               arc.  Returns true if all transitions returned true,
//               false if any returned false.
////////////////////////////////////////////////////////////////////
bool NodeRelation::
sub_render_trans(const AllTransitionsWrapper &input_trans,
                 AllTransitionsWrapper &modify_trans,
                 RenderTraverser *trav) {
  bool all_true = true;
  NodeTransitions::const_iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    if (!(*ti).second->sub_render(this, input_trans, modify_trans, trav)) {
      all_true = false;
    }
  }

  return all_true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::has_sub_render_trans
//       Access: Public
//  Description: Returns true if any transition on the arc has a
//               sub_render() function.
////////////////////////////////////////////////////////////////////
bool NodeRelation::
has_sub_render_trans() const {
  NodeTransitions::const_iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    if ((*ti).second->has_sub_render()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_num_sub_render_trans
//       Access: Public
//  Description: Returns the number of transitions on the arc that
//               have a sub_render() function.
////////////////////////////////////////////////////////////////////
int NodeRelation::
get_num_sub_render_trans() const {
  int count = 0;

  NodeTransitions::const_iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    if ((*ti).second->has_sub_render()) {
      count++;
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::make_arc
//       Access: Public, Static
//  Description: This function is passed to the Factory to make a new
//               NodeRelation by type.  Don't try to call this
//               function directly.
////////////////////////////////////////////////////////////////////
NodeRelation *NodeRelation::
make_arc(const FactoryParams &) {
  return new NodeRelation(NodeRelation::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::attach
//       Access: Protected
//  Description: Makes the arc an official part of the graph by
//               informing the nodes that it connects of its
//               existence.  It is an error to attach an arc that has
//               already been attached.
//
//               It is also an error to attach an arc that is not
//               grounded at both ends.
////////////////////////////////////////////////////////////////////
void NodeRelation::
attach() {
  nassertv(_parent != (Node*)NULL);
  nassertv(_child != (Node*)NULL);
  nassertv(!_attached);

  NodeConnection *parent_connection = _parent->update_connection(_graph_type);
  NodeConnection *child_connection = _child->update_connection(_graph_type);

  if (parent_connection == (NodeConnection *)NULL) {
    graph_cat.error()
      << "Attempt to attach " << *_parent << " simultaneously to more than "
      << max_node_graphs << " different graph types.\n";
    nassertv(false);
  }

  if (child_connection == (NodeConnection *)NULL) {
    graph_cat.error()
      << "Attempt to attach " << *_child << " simultaneously to more than "
      << max_node_graphs << " different graph types.\n";
    nassertv(false);
  }

  DownRelationPointers &parent_list = parent_connection->get_down();
  UpRelationPointers &child_list = child_connection->get_up();

  bool inserted_one = internal_insert_arc(parent_list, this);
  bool inserted_two = internal_insert_arc(child_list, this);
  nassertv(inserted_one && inserted_two);

  // Blow out the cache and increment the current update sequence.
  _net_transitions.clear();
  _last_update = ++last_graph_update(_graph_type);

  // If we have just added a new parent arc to a node that previously
  // had exactly one parent, we also need to set the update sequence
  // counter for the other parent arc, since we have fundamentally
  // changed the nature of the node from non-instanced to instanced.
  if (child_list.size() == 2) {
    child_list[0]->_last_update = _last_update;
    child_list[1]->_last_update = _last_update;
  }

  _attached = true;
  if (_parent_ref != 0) {
    _parent->ref();
  }

  _parent->force_bound_stale();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::detach
//       Access: Protected
//  Description: Removes the arc from the graph.  The arc remains and
//               still refers to its connected nodes, but the nodes
//               themselves no longer know about the arc.  It is an
//               error to detach an arc that is not already attached.
//
//               detach() returns a PointerTo for the arc itself.
//               This is useful not so much for the return value
//               itself, but more to prevent the arc from destructing
//               until detach() returns, since the arc will destruct
//               when its last reference count is removed, and it is
//               generally a bad idea to destruct a class within its
//               own method.
//
//               It must be a PT(TypedWritableReferenceCount), and
//               not PT_NodeRelation, because of circularity problems
//               trying to export the template class PT_NodeRelation.
////////////////////////////////////////////////////////////////////
PT(TypedWritableReferenceCount) NodeRelation::
detach() {
  PT(TypedWritableReferenceCount) result = this;

  nassertr(_parent != (Node*)NULL, result);
  nassertr(_child != (Node*)NULL, result);
  nassertr(_attached, result);

  force_bound_stale();

  NodeConnection *parent_connection = _parent->update_connection(_graph_type);
  NodeConnection *child_connection = _child->update_connection(_graph_type);

  // These should never be NULL, because we're already attached, after
  // all.
  nassertr(parent_connection != (NodeConnection *)NULL &&
           child_connection != (NodeConnection *)NULL, result);

  DownRelationPointers &parent_list = parent_connection->get_down();
  UpRelationPointers &child_list = child_connection->get_up();

  bool removed_one = internal_remove_arc(parent_list, this);
  bool removed_two = internal_remove_arc(child_list, this);

  nassertr(removed_one, result);
  nassertr(removed_two, result);

  _attached = false;
  if (_parent_ref != 0) {
    unref_delete(_parent);
  }

  // Blow out the cache and increment the current update sequence.
  _net_transitions.clear();
  _last_update = ++last_graph_update(_graph_type);

  // If we have just removed a parent arc from a node, leaving exactly
  // one parent, we also need to set the update sequence
  // counter for the other parent arc, since we have fundamentally
  // changed the nature of the node from instanced to non-instanced.
  if (child_list.size() == 1) {
    child_list[0]->_last_update = _last_update;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::detach_below
//       Access: Protected
//  Description: This is a special method that is only called from the
//               Node destructor.  It detaches the arc, but does not
//               remove it from its parent's arc list, which is
//               presumably about to be destroyed anyway.
////////////////////////////////////////////////////////////////////
PT(TypedWritableReferenceCount) NodeRelation::
detach_below() {
  PT(TypedWritableReferenceCount) result = this;

  nassertr(_parent != (Node*)NULL, result);
  nassertr(_child != (Node*)NULL, result);
  nassertr(_attached, result);

  force_bound_stale();

  NodeConnection *child_connection = _child->update_connection(_graph_type);
  nassertr(child_connection != (NodeConnection *)NULL, result);

  UpRelationPointers &child_list = child_connection->get_up();
  bool removed = internal_remove_arc(child_list, this);

  nassertr(removed, result);

  _attached = false;

  // Blow out the cache and increment the current update sequence.
  _net_transitions.clear();
  _last_update = ++last_graph_update(_graph_type);

  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::changed_transition
//       Access: Public, Virtual
//  Description: This is called by set_transition() or
//               clear_transition() whenever a transition is added,
//               updated, or removed from the arc.  It is just a
//               callback to the arc so it can decide whether it needs
//               to update any internal data as a response to this
//               adjustment (for instance, by marking the bounding
//               sphere stale).
////////////////////////////////////////////////////////////////////
void NodeRelation::
changed_transition(TypeHandle trans_type) {
  if (_net_transitions != (NodeTransitionCache *)NULL) {
    _net_transitions->clear_transition(trans_type);
  }
  _last_update = ++last_graph_update(_graph_type);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void NodeRelation::
propagate_stale_bound() {
  // Mark all of our parent arcs stale as well.
  Node *node = _parent;
  nassertv(node != (Node*)NULL);

  const UpRelationPointers &urp = node->find_connection(_graph_type).get_up();
  UpRelationPointers::const_iterator urpi;
  for (urpi = urp.begin(); urpi != urp.end(); ++urpi) {
    (*urpi)->mark_bound_stale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this arc
//               (and all of its descendants).
////////////////////////////////////////////////////////////////////
BoundingVolume *NodeRelation::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)NULL, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our child bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  Node *node = _child;
  nassertr(node != (Node*)NULL, bound);

  child_volumes.push_back(&node->get_bound());

  const DownRelationPointers &drp =
    node->find_connection(_graph_type).get_down();
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    child_volumes.push_back(&(*drpi)->get_bound());
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();

  bool success =
    bound->around(child_begin, child_end);

#ifndef NDEBUG
  if (!success) {
    graph_cat.error()
      << "Unable to recompute bounding volume for " << *this << ":\n"
      << "Cannot put " << bound->get_type() << " around:\n";
    for (int i = 0; i < (int)child_volumes.size(); i++) {
      graph_cat.error(false)
        << "  " << *child_volumes[i] << "\n";
    }
  }
#endif

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void NodeRelation::
write_datagram(BamWriter *manager, Datagram &me)
{
  //Write out the "dynamic" type
  manager->write_handle(me, _graph_type);

  //We should always be attached if we are trying to write out
  nassertv(_attached);
  //Neither the Child nor the Parent should be NULL
  nassertv(get_parent() != Node::Null && get_child() != Node::Null);

  //Write out the pointer to my parent
  manager->write_pointer(me, _parent);
  //Write out the pointer to my child
  manager->write_pointer(me, _child);

  //Write out the sort relation for this object
  me.add_uint16(_sort);

  //Now write out all the Transitions on this arc
  me.add_uint16(_transitions.size());
  NodeTransitions::iterator ci;
  for(ci = _transitions.begin(); ci != _transitions.end(); ci++) {
    manager->write_pointer(me, (*ci).second);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointers to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int NodeRelation::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  nassertr(p_list[0] != TypedWritable::Null &&
           p_list[1] != TypedWritable::Null, 0);
  _parent = DCAST(Node, p_list[0]);
  _child = DCAST(Node, p_list[1]);

  if (manager->get_file_minor_ver() < 3) {
    // In bam versions before 3.3, we let the NodeRelation completely
    // handle its attachment to its parents.
    attach();

  } else {
    // Beginning in bam version 3.3, we let the parent Node handle the
    // attachment of its children arcs, so we can guarantee correct
    // ordering of children.

    // We must explicitly add the arc to its child node, but not to
    // its parent node.
    NodeConnection *child_connection = _child->update_connection(_graph_type);
    if (child_connection != (NodeConnection *)NULL) {
      UpRelationPointers &child_list = child_connection->get_up();
      bool inserted = internal_insert_arc(child_list, this);
      nassertr(inserted, _num_transitions + 2);
      _attached = true;
    }
  }

  //The rest of this is the list of Transitions
  for(int i = 2; i < _num_transitions + 2; i++)
  {
    //Ignore Null pointers.  This SHOULD mean that
    //we have received a Transition that the current
    //version doesn't know about, so we want to be able
    //to gracefully handle new functionality being thrown
    //at old code
    if (p_list[i] == TypedWritable::Null)
    {
      graph_cat->warning()
        << get_type().get_name()
        << ": Ignoring null Transition" << endl;
    }
    else
    {
      //Let set_transition do the work for storing
      //a reference to this transition, determing it's
      //exact type, telling the transition about me, etc...
      set_transition(DCAST(NodeTransition, p_list[i]));
    }
  }
  return _num_transitions+2;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::make_NodeRelation
//       Access: Protected, Static
//  Description: Factory method to generate a NodeRelation object
////////////////////////////////////////////////////////////////////
TypedWritable* NodeRelation::
make_NodeRelation(const FactoryParams &params)
{
  NodeRelation *me = new NodeRelation(get_class_type());
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void NodeRelation::
fillin(DatagramIterator& scan, BamReader* manager)
{
  _graph_type = manager->read_handle(scan);
  //Read in my parent
  manager->read_pointer(scan);
  //Read in my child
  manager->read_pointer(scan);
  //Get my sort relation
  _sort = scan.get_uint16();

  //Now read in all of my transitions
  _num_transitions = scan.get_uint16();
  for(int i = 0; i < _num_transitions; i++)
  {
    manager->read_pointer(scan);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a NodeRelation object
////////////////////////////////////////////////////////////////////
void NodeRelation::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_NodeRelation);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::init_type
//       Access: Public, Static
//  Description: Initializes the TypeHandles associated with this
//               class.
////////////////////////////////////////////////////////////////////
void NodeRelation::
init_type() {
  TypedWritableReferenceCount::init_type();
  BoundedObject::init_type();
  register_type(_type_handle, "NodeRelation",
                TypedWritableReferenceCount::get_class_type(),
                BoundedObject::get_class_type());
  register_type(_stashed_type_handle, "StashedNodeRelation",
                get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_type
//       Access: Public, Virtual
//  Description: Returns the particular type represented by this
//               instance of the class.
////////////////////////////////////////////////////////////////////
TypeHandle NodeRelation::
get_type() const {
  return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::force_init_type
//       Access: Public, Virtual
//  Description: Called by the TypeHandle system when it is detected
//               that init_type() was not called for some reason.
//               This is only called in an error situation, and it
//               attempts to remedy the problem so we can recover and
//               continue.
////////////////////////////////////////////////////////////////////
TypeHandle NodeRelation::
force_init_type() {
  init_type();
  return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a specific
//               TypeHandle, indicating the type of transition that we
//               are setting, and a NodeTransition pointer indicating
//               the value of the transition.  The NodeTransition may
//               be NULL indicating that the transition should be
//               cleared.  If the NodeTransition is not NULL, it must
//               match the type indicated by the TypeHandle.
//
//               The return value is a pointer to the *previous*
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) NodeRelation::
set_transition(TypeHandle handle, NodeTransition *trans) {
  PT(NodeTransition) old_trans =
    _transitions.set_transition(handle, trans);

  if (old_trans != (NodeTransition *)NULL) {
    old_trans->removed_from_arc(this);
  }
  if (trans != (NodeTransition *)NULL) {
    trans->added_to_arc(this);
  }

  changed_transition(handle);
  return old_trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the arc.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) NodeRelation::
clear_transition(TypeHandle handle) {
  PT(NodeTransition) old_trans =
    _transitions.clear_transition(handle);
  if (old_trans != (NodeTransition *)NULL) {
    old_trans->removed_from_arc(this);
  }

  changed_transition(handle);
  return old_trans;
}
