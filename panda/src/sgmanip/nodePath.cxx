// Filename: nodePath.cxx
// Created by:  drose (05Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodePath.h"
#include "config_sgmanip.h"
#include "findApproxLevel.h"

#include <renderRelation.h>
#include <namedNode.h>
#include <wrt.h>
#include <indent.h>
#include <look_at.h>
#include <nodeTransitionWrapper.h>
#include <nodeAttributeWrapper.h>
#include <drawBoundsTransition.h>
#include <geometricBoundingVolume.h>
#include <boundingSphere.h>
#include <sceneGraphAnalyzer.h>
#include <sceneGraphReducer.h>
#include <nodeTransitionWrapper.h>
#include <nodeAttributeWrapper.h>
#include <nullLevelState.h>
#include <traverserVisitor.h>
#include <dftraverser.h>
#include <bamFile.h>

#include <list>

// This class is used in prepare_scene() to traverse the scene graph
// and register textures with the gsg.
class ScenePrepareVisitor : public TraverserVisitor<NodeTransitionWrapper, NullLevelState> {
public:
  bool forward_arc(NodeRelation *, NodeTransitionWrapper &trans,
		   NodeAttributeWrapper &, NodeAttributeWrapper &,
		   NullLevelState &) {
    TextureTransition *tt;
    if (get_transition_into(tt, trans)) {
      if (tt->is_on()) {
	tt->get_texture()->prepare(_gsg);
      }
    }
    return true;
  }

  GraphicsStateGuardianBase *_gsg;
};

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Public
//  Description: Appends a new child node onto the bottom end of the
//               path.  This will search for an existing arc between
//               the current bottom node and the indicated node;
//               returns true if the arc is found and appended, false
//               if no such arc exists.
//
//               This operation may be ambiguous if there are multiple
//               arcs connecting the bottom node of the path with the
//               indicated node.  In such case, the first arc found
//               will be quietly chosen.  Use append_arc() when
//               unambiguous behavior is required.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(Node *node) {
  nassertr(verify_connectivity(), false);
  nassertr(node != (Node *)NULL, false);
  
  if (is_empty()) {
    nassertr(_head == (ArcComponent *)NULL, false);
    _top_node = node;
    return true;
  }

  Node *bottom_node = get_bottom_node();
  NodeRelation *arc = find_arc(bottom_node, node, _graph_type);
  if (arc == (NodeRelation *)NULL) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
	<< "Cannot extend " << *this << " by "
	<< *node << "; no connection.\n";
    }
    return false;
  }

  _head = new ArcComponent(arc, _head);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Public
//  Description: Adds the indicated arc to the end of the path.  The
//               arc must connect the bottom node of the path to some
//               other node.  Returns true if this is so, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(NodeRelation *arc) {
  nassertr(verify_connectivity(), false);
  nassertr(arc != (NodeRelation *)NULL, false);
  // Make sure the graph types are consistent.
  nassertr(arc->get_type() == _graph_type, false);

  if (is_empty()) {
    nassertr(_head == (ArcComponent *)NULL, false);
    _top_node = arc->get_parent();
  }

  if (arc->get_parent() != get_bottom_node()) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
	<< "Cannot extend " << *this << " by arc " << *arc << "\n";
    }
    return false;
  }

  _head = new ArcComponent(arc, _head);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Public
//  Description: Adds the indicated NodePath to the end of this path.
//               The top node of the other NodePath must be the same
//               node as the bottom node of this path.  Returns true
//               if this is so, false otherwise or if the other path
//               is no longer accurately connected.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(const NodePath &other) {
  nassertr(verify_connectivity(), false);
  if (is_empty()) {
    // A special case: extending an empty path by another path is just
    // an assignment.
    (*this) = other;
    return true;
  }

  return r_extend_by(other._head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Public
//  Description: Extends the NodePath by the NodePath represented by
//               the given approximate string.  If the match is
//               ambiguous, returns the first match found (which will
//               be the shortest path).  Returns true if the extension
//               was made, false if the node could not be found.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(const string &path) {
  nassertr(verify_connectivity(), false);
  NodePathCollection col;
  find_matches(col, path, 1);

  if (col.is_empty()) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
	<< "Could not extend " << *this << " by "
	<< path << "; no match found.\n";
    }
    return false;
  }

  (*this) = col.get_path(0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_down_to
//       Access: Public
//  Description: Extends the NodePath down along the shortest
//               available path to the indicated node, even if the
//               node is several levels below the NodePath's bottom
//               node.  Returns true if the extension was made, false
//               if the node was not below this NodePath.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_down_to(Node *node) {
  if (!is_empty() && get_bottom_node() == node) {
    // We're already there!
    return true;
  }

  nassertr(verify_connectivity(), false);
  NodePathCollection col;
  FindApproxPath approx_path;
  approx_path.add_match_many();
  approx_path.add_match_pointer(node);
  find_matches(col, approx_path, -1);

  if (col.is_empty()) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
	<< "Could not extend " << *this << " down to "
	<< *node << "; no connection found.\n";
    }
    return false;
  }

  (*this) = col.get_path(0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::shorten
//       Access: Public
//  Description: Shortens the NodePath by removing the indicated
//               number of nodes from the bottom of the path.  It
//               is an error to shorten the path to less than zero
//               nodes; thus, num_nodes must be <= get_num_nodes().
////////////////////////////////////////////////////////////////////
void NodePath::
shorten(int num_nodes) {
  nassertv(num_nodes >= 0 && num_nodes <= get_num_nodes());

  if (is_singleton()) {
    // A special case: shortening a singleton path by (presumably) one
    // node.
    _top_node = (Node *)NULL;
    return;
  }

  int count = num_nodes;
  while (count > 0) {
    nassertv(_head != (ArcComponent *)NULL);
    if (_head->_next == (ArcComponent *)NULL) {
      // If we're about to shorten the path to a singleton, identify
      // the top node first.
      _top_node = _head->_arc->get_parent();
    }
    _head = _head->_next;
    count--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear
//       Access: Public
//  Description: Resets the NodePath to an empty path.
////////////////////////////////////////////////////////////////////
void NodePath::
clear() {
  _head.clear();
  _top_node.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_children
//       Access: Public
//  Description: Returns the set of all child nodes of the bottom
//               node.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
get_children() const {
  NodePathCollection result;
  nassertr(verify_connectivity(), result);
  nassertr(!is_empty(), result);

  Node *node = get_bottom_node();
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      NodePath child(*this);
      child.extend_by(arc);
      result.add_path(child);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_siblings
//       Access: Public
//  Description: Returns the set of all child nodes of the bottom
//               node's parent, except the bottom node itself.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
get_siblings() const {
  NodePathCollection result;
  nassertr(verify_connectivity(), result);
  nassertr(has_arcs(), result);
  nassertr(_head != (ArcComponent *)NULL, result);
  NodeRelation *my_arc = _head->_arc;
  
  NodePath parent = *this;
  parent.shorten(1);

  Node *node = parent.get_bottom_node();
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      if (arc != my_arc) {
	NodePath sib(parent);
	sib.extend_by(arc);
	result.add_path(sib);
      }
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_all_paths_down_to
//       Access: Public
//  Description: Returns the set of all NodePaths that extend from
//               this NodePath down to the indicated node.  The
//               shortest paths will be listed first.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
find_all_paths_down_to(Node *node) const {
  NodePathCollection col;
  nassertr(verify_connectivity(), col);
  nassertr(node != (Node *)NULL, col);
  FindApproxPath approx_path;
  approx_path.add_match_many();
  approx_path.add_match_pointer(node);
  find_matches(col, approx_path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_all_matches
//       Access: Public
//  Description: Returns the complete set of all NodePaths that begin
//               with this NodePath and can be extended by
//               path.  The shortest paths will be listed
//               first.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
find_all_matches(const string &path) const {
  NodePathCollection col;
  nassertr(verify_connectivity(), col);
  find_matches(col, path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_node
//       Access: Public
//  Description: Returns the nth node of the path, where 0 is the
//               bottom node and get_num_nodes() - 1 is the top node.
//               This requires iterating through the path.
////////////////////////////////////////////////////////////////////
Node *NodePath::
get_node(int index) const {
  nassertr(index >= 0 && index < get_num_nodes(), NULL);

  if (index == 0) {
    if (_head == (ArcComponent *)NULL) {
      // A singleton or empty list.
      return _top_node;
    }
    return _head->_arc->get_child();
  }

  ArcComponent *comp = _head;
  index--;
  while (index > 0) {
    // If this assertion fails, the index was out of range.
    nassertr(comp != (ArcComponent *)NULL, NULL);
    comp = comp->_next;
    index--;
  }

  // If this assertion fails, the index was out of range.
  nassertr(comp != (ArcComponent *)NULL, NULL);
  return comp->_arc->get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_num_arcs
//       Access: Public
//  Description: Returns the number of arcs in the path.  This is
//               always one less than the number of nodes (except for
//               a completely empty path).
////////////////////////////////////////////////////////////////////
int NodePath::
get_num_arcs() const {
  int num = 0;
  ArcComponent *comp = _head;
  while (comp != (ArcComponent *)NULL) {
    num++;
    comp = comp->_next;
  }
  return num;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_arc
//       Access: Public
//  Description: Returns the nth arc of the path, where 0 is the arc
//               above the bottom node node and get_num_arcs() - 1 is
//               the arc below the top node.  This requires iterating
//               through the path.
////////////////////////////////////////////////////////////////////
NodeRelation *NodePath::
get_arc(int index) const {
  nassertr(index >= 0 && index < get_num_arcs(), NULL);

  ArcComponent *comp = _head;
  while (index > 0) {
    // If this assertion fails, the index was out of range.
    nassertr(comp != (ArcComponent *)NULL, NULL);
    comp = comp->_next;
    index--;
  }

  // If this assertion fails, the index was out of range.
  nassertr(comp != (ArcComponent *)NULL, NULL);
  return comp->_arc;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_top_node
//       Access: Public
//  Description: Returns the top node of the path, or NULL if the path
//               is empty.  This requires iterating through the path.
////////////////////////////////////////////////////////////////////
Node *NodePath::
get_top_node() const {
  if (_head == (ArcComponent *)NULL) {
    // A singleton or empty list.
    return _top_node;
  }

  ArcComponent *comp = _head;
  while (comp->_next != (ArcComponent *)NULL) {
    comp = comp->_next;
  }

  // This assertion should not fail unless there is a logic error in
  // the above.
  nassertr(comp != (ArcComponent *)NULL, NULL);
  return comp->_arc->get_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::share_with
//       Access: Public
//  Description: Adjusts this NodePath so that it shares components
//               with the other NodePath, even if the two paths were
//               independently generated.
//
//               Normally, if two NodePaths will share arcs, you
//               should use the first one to derive the second; this
//               will ensure that if one NodePath is reparented, the
//               second will automatically be updated to reflect the
//               change.  If the two NodePaths are computed
//               independently of each other, but end up sharing arcs,
//               then reparenting one could invalidate the second.
//
//               This operation can be applied to a NodePath that is
//               known to share arcs with another NodePath, even if it
//               was generated separately, so that both NodePaths will
//               be considered related in the future.
//
//               The return value is the number of arcs from the top
//               of the path that both NodePaths had in common.
////////////////////////////////////////////////////////////////////
int NodePath::
share_with(const NodePath &other) {
  typedef list<ArcComponent *> Arcs;
  Arcs other_arcs;
  Arcs this_arcs;

  // First, we have to reverse the lists of this path and the other
  // path, so we can compare their initial subsets.
  ArcComponent *comp = other._head;
  while (comp != (ArcComponent *)NULL) {
    other_arcs.push_front(comp);
    comp = comp->_next;
  }

  comp = _head;
  while (comp != (ArcComponent *)NULL) {
    this_arcs.push_front(comp);
    comp = comp->_next;
  }

  // Now determine how many arcs this and the other path have in
  // common.
  int in_common = 0;
  Arcs::const_iterator oi = other_arcs.begin();
  Arcs::const_iterator ti = this_arcs.begin();
  while (oi != other_arcs.end() && 
	 ti != this_arcs.end() && 
	 (*oi)->_arc == (*ti)->_arc) {
    ++oi;
    ++ti;
    ++in_common;
  }

  if (in_common > 0) {
    ArcComponent *dest;
    if (oi == other_arcs.end()) {
      dest = other._head;
    } else {
      dest = (*oi);
    }

    if (ti == this_arcs.end()) {
      _head = dest;
    } else {
      (*ti)->_next = dest;
    }
  }
  
  return in_common;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::verify_connectivity
//       Access: Public
//  Description: Returns true if all of the arcs described in the
//               NodePath are still connected, false otherwise.
////////////////////////////////////////////////////////////////////
bool NodePath::
verify_connectivity() const {
  if (!has_arcs()) {
    return true;
  }

  const ArcComponent *comp = _head;
  nassertr(comp != (const ArcComponent *)NULL, false);

  Node *parent = comp->_arc->get_parent();
  Node *child = comp->_arc->get_child();
  if (parent == (Node *)NULL || child == (Node *)NULL) {
    return false;
  }

  // We also want to verify that all of the arcs are of the proper
  // type.
  if (comp->_arc->get_type() != _graph_type) {
    return false;
  }

  comp = comp->_next;
  while (comp != (const ArcComponent *)NULL) {
    Node *next_parent = comp->_arc->get_parent();
    Node *next_child = comp->_arc->get_child();
    if (next_parent == (Node *)NULL || next_child == (Node *)NULL) {
      return false;
    }
    if (next_child != parent) {
      return false;
    }
    if (comp->_arc->get_type() != _graph_type) {
      return false;
    }

    parent = next_parent;
    child = next_child;
    comp = comp->_next;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::amputate_badness
//       Access: Public
//  Description: If a NodePath is inadvertantly broken
//               (i.e. verify_connectivity() returns false), this
//               function may be called to attempt to contain the
//               damage.  It does this by lopping off the top of the
//               NodePath above the lowest disconnected arc.  You
//               might end up with a very short NodePath indeed, but
//               at least it will be fully connected.  It returns the
//               same value as verify_connectivity(): true if the path
//               was already connected, false if it had been broken
//               (and has therefore been truncated).
//
//               See also repair_connectivity().
////////////////////////////////////////////////////////////////////
bool NodePath::
amputate_badness() {
  if (!has_arcs()) {
    return true;
  }

  ArcComponent *comp = _head;
  nassertr(comp != (ArcComponent *)NULL, false);

  Node *parent = comp->_arc->get_parent();
  Node *child = comp->_arc->get_child();
  if (parent == (Node *)NULL || child == (Node *)NULL) {
    // Eek!  The bottom arc is broken!
    _top_node = child;
    _head = (ArcComponent *)NULL;
    return false;
  }

  // We also want to verify that all of the arcs are of the proper
  // type.
  if (comp->_arc->get_type() != _graph_type) {
    // Eek!  The bottom arc is broken!
    _top_node = child;
    _head = (ArcComponent *)NULL;
    return false;
  }

  ArcComponent *prev = comp;
  comp = comp->_next;
  while (comp != (const ArcComponent *)NULL) {
    Node *next_parent = comp->_arc->get_parent();
    Node *next_child = comp->_arc->get_child();
    if (next_parent == (Node *)NULL || next_child == (Node *)NULL) {
      prev->_next = (ArcComponent *)NULL;
      return false;
    }
    if (next_child != parent) {
      prev->_next = (ArcComponent *)NULL;
      return false;
    }
    if (comp->_arc->get_type() != _graph_type) {
      prev->_next = (ArcComponent *)NULL;
      return false;
    }

    parent = next_parent;
    child = next_child;
    prev = comp;
    comp = comp->_next;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::repair_connectivity
//       Access: Public
//  Description: When a NodePath has been inadvertantly disconnected,
//               this attempts to repair the damage by finding a path
//               from the bottom of the top NodePath to the top of the
//               still-connected part of this NodePath.  It returns
//               true if the connection can be found, false if not (in
//               which case the NodePath is unchanged).
////////////////////////////////////////////////////////////////////
bool NodePath::
repair_connectivity(const NodePath &top) {
  NodePath new_path(*this);
  new_path.amputate_badness();
  if (new_path.is_empty()) {
    return false;
  }

  Node *top_node = new_path.get_top_node();
  NodePath full_path(top);
  if (!full_path.extend_down_to(top_node)) {
    return false;
  }

  if (!full_path.extend_by(new_path)) {
    return false;
  }

  (*this) = full_path;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::reparent_to
//       Access: Public
//  Description: Removes the bottom node of the NodePath from its
//               current parent and attaches it to the bottom node of
//               the indicated NodePath.  The arc above the bottom
//               node is moved without destroying any transitions
//               attached to it.
//
//               It is an error to call this method on a NodePath that
//               refers to just a single node, with no arcs.  If you
//               really want to parent just a single node somewhere,
//               use instance_to() or attach_new_node().
//
//               This also updates the NodePath itself, as well as all
//               NodePaths that were derived from this one, to reflect
//               the new path to the node.
////////////////////////////////////////////////////////////////////
void NodePath::
reparent_to(const NodePath &other) {
  nassertv(other.verify_connectivity());
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  nassertv(!other.is_empty());

  NodeRelation *arc = _head->_arc;

  arc->change_parent(other.get_bottom_node());

  // Move our head pointer to the bottom of the new chain.  This will
  // update our own path, as well as all paths that share the same
  // head pointer (i.e. all paths derived from this one).
  _head->_next = other._head;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::wrt_reparent_to
//       Access: Public
//  Description: This functions identically to reparent_to(), except
//               the transform above this node is also adjusted so
//               that the node remains in the same place in world
//               coordinates, even if it is reparented into a
//               different coordinate system.
////////////////////////////////////////////////////////////////////
void NodePath::
wrt_reparent_to(const NodePath &other) {
  nassertv(other.verify_connectivity());
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  nassertv(!other.is_empty());

  LMatrix4f mat = get_mat(other);
  set_mat(mat);

  reparent_to(other);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::instance_to
//       Access: Public
//  Description: Adds the bottom node of the NodePath as a child of
//               the bottom node of the indicated other NodePath.  Any
//               other parent-child relations of the node are
//               unchanged; in particular, the node is not removed
//               from its existing parent, if any.  
//
//               A new arc is created, and its transitions are copied
//               from the bottom arc of this NodePath, if any.
//
//               If the node already had an existing parent, this
//               method will create a new instance of the node within
//               the scene graph.
//
//               This does not change the NodePath itself, but does
//               return a new NodePath that reflects the new instance
//               node.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
instance_to(const NodePath &other) const {
  nassertr(verify_connectivity(), NodePath());
  nassertr(!is_empty(), NodePath());
  nassertr(!other.is_empty(), NodePath());

  Node *node = get_bottom_node();
  NodeRelation *arc = 
    NodeRelation::create_typed_arc(_graph_type, other.get_bottom_node(), node);
  nassertr(arc != (NodeRelation *)NULL, NodePath());
  nassertr(arc->is_exact_type(_graph_type), NodePath());

  if (has_arcs()) {
    // Copy the transitions from this one's bottom arc, so the
    // instance will inherit the same local state by default.
    arc->copy_transitions_from(get_bottom_arc());
  }

  NodePath instance(*this);
  instance._head = new ArcComponent(arc, other._head);
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::copy_to
//       Access: Public
//  Description: Functions exactly like instance_to(), except a deep
//               copy is made of the bottom node and all of its
//               descendents, which is then parented to the indicated
//               node.  A NodePath to the newly created copy is
//               returned.
//
//               Certain kinds of nodes may not be copied; if one of
//               these is encountered, the node will be "copied" as
//               the nearest copyable base class.  For instance, a
//               Camera node in the graph will become a simple
//               NamedNode.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
copy_to(const NodePath &other) const {
  nassertr(verify_connectivity(), NodePath());
  nassertr(!is_empty(), NodePath());
  nassertr(!other.is_empty(), NodePath());

  Node *source_node = get_bottom_node();
  PT_Node copy_node = source_node->copy_subgraph(_graph_type);
  nassertr(copy_node != (Node *)NULL, NodePath());

  NodeRelation *arc = 
    NodeRelation::create_typed_arc(_graph_type, other.get_bottom_node(), 
				   copy_node);
  nassertr(arc != (NodeRelation *)NULL, NodePath());
  nassertr(arc->is_exact_type(_graph_type), NodePath());

  if (has_arcs()) {
    // Copy the transitions from this one's bottom arc, so the
    // duplicate will inherit the same local state by default.
    arc->copy_transitions_from(get_bottom_arc());
  }

  NodePath instance(*this);
  instance._head = new ArcComponent(arc, other._head);
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::attach_new_node
//       Access: Public
//  Description: Attaches a new node, with or without existing
//               parents, to the scene graph below the bottom node of
//               this NodePath.  This is the preferred way to add
//               nodes to the graph.
//
//               This does *not* automatically extend the current
//               NodePath to reflect the attachment; however, a
//               NodePath that does reflect this extension is
//               returned.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
attach_new_node(Node *node) const {
  nassertr(verify_connectivity(), NodePath());
  nassertr(!is_empty(), NodePath(_graph_type));
  nassertr(node != (Node *)NULL, NodePath(_graph_type));

  NodePath path(node, _graph_type);
  return path.instance_to(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::remove_node
//       Access: Public
//  Description: Disconnects the bottom node from the scene graph and
//               destroys its arc (along with all associated
//               transitions).  This will also delete the node if
//               there are no other pointers to it.
//
//               Normally, this should be called only when you are
//               really done with the node.  If you want to remove a
//               node from the scene graph but keep it around for
//               later, you should probably use reparent_to() and put
//               it under a holding node instead.
//
//               It is an error to call this method on a NodePath that
//               refers to just a single node, with no arcs.
//
//               After the node is removed, the NodePath will have
//               been cleared.
////////////////////////////////////////////////////////////////////
void NodePath::
remove_node() {
  nassertv(verify_connectivity());
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  PT(NodeRelation) arc = _head->_arc;
  clear();
  remove_arc(arc);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::as_string
//       Access: Public
//  Description: Formats the NodePath as a string, showing each node
//               name (or possibly type, if the node is unnamed),
//               separated by slashes.
//
//               The resulting string is not necessarily unique to
//               this NodePath.  Some nodes may be unnamed; different
//               nodes in the graph may share the same name.  Even if
//               this is not the case, there might be multiple
//               different arcs between the same two nodes, which
//               cannot be described in this string.
//
//               start_at_node is the node number, zero-based, at
//               which to start the string.  Normally this would be 0
//               to format the entire path, or 1 to format the path
//               beginning at the node below the top node (which could
//               then be used to restore the path later, subject to
//               the ambiguities described above).
////////////////////////////////////////////////////////////////////
string NodePath::
as_string(int start_at_node) const {
  nassertr(start_at_node >= 0, string());

  if (is_empty()) {
    // An empty path always returns an empty string.
    return string();

  } else if (is_singleton()) {
    // A singleton path is a special case.  We either return the name
    // of the singleton, if start_at_node is 0, or the empty string in
    // any other case.

    if (start_at_node == 0) {
      return format_node_name(_top_node);
    }
    return string();
  }

  // In the normal case, we have at least one arc.  We must walk to
  // the end of the list, then back up start_at_node times, and start
  // formatting names from there.
  string result;
  r_as_string(_head, result, start_at_node);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::write_transitions
//       Access: Public
//  Description: Writes to the indicated output stream a list of all
//               transitions encountered on all arcs along the
//               NodePath.
////////////////////////////////////////////////////////////////////
void NodePath::
write_transitions(ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "Empty NodePath, no transitions.\n";
  } else if (is_singleton()) {
    out << "Singleton NodePath, no transitions.\n";
  } else {
    r_write_transitions(_head, out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_net_transitions
//       Access: Public
//  Description: Returns the set of all accumulated transitions along
//               the NodePath.
////////////////////////////////////////////////////////////////////
AllTransitionsWrapper NodePath::
get_net_transitions() const {
  AllTransitionsWrapper trans;

  if (has_arcs()) {
    r_get_net_transitions(_head, trans);
  }

  return trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::analyze
//       Access: Public
//  Description: Reports some statistics about the scene graph
//               properties at and below this node.
////////////////////////////////////////////////////////////////////
void NodePath::
analyze() const {
  nassertv(!is_empty());
  SceneGraphAnalyzer sga(_graph_type);
  sga.add_node(get_bottom_node());
  sga.write(nout);
}


////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_light
//       Access: Public
//  Description: Lightly flattens out the hierarchy below this node by
//               removing unneeded grouping nodes--nodes that have
//               exactly one child, for instance, but have no special
//               properties in themselves.  This will not affect any
//               transforms or other transitions.
//
//               This is the same level of flattening that is normally
//               automatically applied by the egg loader.  It's
//               generally completely safe (except that it may remove
//               a node you expected to be there).
//
//               The return value is the number of arcs removed.
////////////////////////////////////////////////////////////////////
int NodePath::
flatten_light() {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  int num_removed = gr.flatten(get_bottom_node(), false);

  if (sgmanip_cat.is_debug()) {
    sgmanip_cat.debug()
      << "flatten_light() removed " << num_removed << " arcs.\n";
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_medium
//       Access: Public
//  Description: A more thorough flattening than flatten_light(), this
//               first applies all the transforms, colors, and texture
//               matrices from the arcs onto the vertices, and then
//               performs a flatten.  This results in substantially
//               improved perforamance over flatten_light() because
//               (a) there are now fewer transforms, and (b) once the
//               transforms are gone, it can safely remove more nodes.
//
//               Since this operation will remove transforms from the
//               scene graph, it may be dangerous to apply to arcs
//               where you expect to dynamically modify the transform,
//               or where you expect the geometry to remain in a
//               particular local coordinate system.
////////////////////////////////////////////////////////////////////
int NodePath::
flatten_medium() {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  gr.apply_transitions(get_bottom_node());
  int num_removed = gr.flatten(get_bottom_node(), false);

  if (sgmanip_cat.is_debug()) {
    sgmanip_cat.debug()
      << "flatten_medium() removed " << num_removed << " arcs.\n";
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_strong
//       Access: Public
//  Description: The strongest possible flattening.  This first
//               applies all of the transforms to the vertices, as in
//               flatten_medium(), but then it will combine sibling
//               nodes together when possible, in addition to removing
//               unnecessary parent-child nodes.  This can result in
//               substantially fewer nodes, but any nicely-grouped
//               hierachical bounding volumes may be lost.
//
//               It is generally a good idea to apply this kind of
//               flattening only to nodes that will be culled largely
//               as a single unit, like a car.  Applying this to an
//               entire scene may result in overall poorer performance
//               because of less-effective culling.
////////////////////////////////////////////////////////////////////
int NodePath::
flatten_strong() {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  gr.apply_transitions(get_bottom_node());
  int num_removed = gr.flatten(get_bottom_node(), true);

  if (sgmanip_cat.is_debug()) {
    sgmanip_cat.debug()
      << "flatten_strong() removed " << num_removed << " arcs.\n";
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::write_bam_file
//       Access: Public
//  Description: Writes the contents of this node and below out to a
//               bam file with the indicated filename.  This file may
//               then be read in again, as is, at some later point.
//               Returns true if successful, false on some kind of
//               error.
////////////////////////////////////////////////////////////////////
bool NodePath::
write_bam_file(const string &filename) const {
  nassertr(!is_empty(), false);
  BamFile bam_file;

  bool okflag = false;

  if (bam_file.open_write(filename)) {
    if (bam_file.write_object(get_bottom_node())) {
      okflag = true;
    }
    bam_file.close();
  }

  return okflag;
}
  

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Public
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos(const LVecBase3f &pos) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  mat.set_row(3, pos);
  set_mat(mat);
}

void NodePath::
set_x(float x) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  mat(3, 0) = x;
  set_mat(mat);
}

void NodePath::
set_y(float y) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  mat(3, 1) = y;
  set_mat(mat);
}

void NodePath::
set_z(float z) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  mat(3, 2) = z;
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_pos
//       Access: Public
//  Description: Retrieves the translation component of the transform.
////////////////////////////////////////////////////////////////////
LPoint3f NodePath::
get_pos() const {
  nassertr(has_arcs(), LPoint3f(0.0, 0.0, 0.0));
  nassertr(_head != (ArcComponent *)NULL, LPoint3f(0.0, 0.0, 0.0));
  LMatrix4f mat = get_mat();
  return mat.get_row3(3);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Public
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_hpr(const LVecBase3f &hpr) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_h(float h) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[0] = h;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(mat);
}

void NodePath::
set_p(float p) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[1] = p;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(mat);
}

void NodePath::
set_r(float r) {
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[2] = r;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hpr
//       Access: Public
//  Description: Retrieves the rotation component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_hpr() const { 
  nassertr(has_arcs(), LVecBase3f(0.0, 0.0, 0.0));
  nassertr(_head != (ArcComponent *)NULL, LVecBase3f(0.0, 0.0, 0.0));
  LMatrix4f mat = get_mat();
  LVecBase3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Public
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_scale(const LVecBase3f &sv3) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  compose_matrix(mat, sv3, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_sx(float sx) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[0] = sx;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_sy(float sy) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[1] = sy;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_sz(float sz) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[2] = sz;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Public
//  Description: Retrieves the scale component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_scale() const {
  nassertr(has_arcs(), LVecBase3f(1.0, 1.0, 1.0));
  nassertr(_head != (ArcComponent *)NULL, LVecBase3f(1.0, 1.0, 1.0));
  LMatrix4f mat = get_mat();
  LVecBase3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos);
  return scale;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color_scale
//       Access: Public
//  Description: Sets the color scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_color_scale(const LVecBase4f &sv4) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting color scale on data graph arc.\n"
      << "(This is meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  if (sv4[0] != 1 || sv4[1] != 1 || sv4[2] != 1) {
    LMatrix4f mat = LMatrix4f::scale_mat(sv4[0], sv4[1], sv4[2]);
    _head->_arc->set_transition(new ColorMatrixTransition(mat));
  }
  else {
    _head->_arc->clear_transition(ColorMatrixTransition::get_class_type());
  }

  if (sv4[3] != 1) {
    _head->_arc->set_transition(new AlphaTransformTransition(0, sv4[3]));
  }
  else {
    _head->_arc->clear_transition(AlphaTransformTransition::get_class_type());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_color_scale
//       Access: Public
//  Description: Returns the complete transform vector that has been
//               applied to the bottom arc, or the all 1's if no
//               scale has been applied
////////////////////////////////////////////////////////////////////
LVecBase4f NodePath::
get_color_scale() const {
  nassertr(has_arcs(), LVecBase4f(1,1,1,1));
  nassertr(_head != (ArcComponent *)NULL, LVecBase4f(1,1,1,1));

  LVecBase4f scale;

  const ColorMatrixTransition *ct;
  if (!get_transition_into(ct, _head->_arc)) {
    // No relative transform.
    scale[0] = 1;
    scale[1] = 1;
    scale[2] = 1;
  }
  else
  {
    LVecBase3f old_scale, hpr, pos;
    decompose_matrix(ct->get_matrix(), old_scale, hpr, pos);
    scale[0] = old_scale[0];
    scale[1] = old_scale[1];
    scale[2] = old_scale[2];
  }

  const AlphaTransformTransition *att;
  if (!get_transition_into(att, _head->_arc)) {
    scale[3] = 1;
  }
  else {
    scale[3] = att->get_scale();
  }

  return scale;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Public
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr(const LVecBase3f &pos, const LVecBase3f &hpr) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, old_pos;
  decompose_matrix(mat, scale, old_hpr, old_pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Public
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr_scale(const LVecBase3f &pos, const LVecBase3f &hpr,
		  const LVecBase3f &scale) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat;
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_mat
//       Access: Public
//  Description: Directly sets an arbitrary 4x4 transform matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
set_mat(const LMatrix4f &mat) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting transform on data graph arc.\n"
      << "(This is probably meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  _head->_arc->set_transition(new TransformTransition(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Public
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
void NodePath::
look_at(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::look_at(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Public
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::heads_up(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Public
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos(const NodePath &other, const LVecBase3f &pos) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  mat.set_row(3, pos);
  set_mat(other, mat);
}

void NodePath::
set_x(const NodePath &other, float x) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  mat(3, 0) = x;
  set_mat(other, mat);
}

void NodePath::
set_y(const NodePath &other, float y) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  mat(3, 1) = y;
  set_mat(other, mat);
}

void NodePath::
set_z(const NodePath &other, float z) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  mat(3, 2) = z;
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Public
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_hpr(const NodePath &other, const LVecBase3f &hpr) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_h(const NodePath &other, float h) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[0] = h;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_p(const NodePath &other, float p) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[1] = p;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_r(const NodePath &other, float r) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[2] = r;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Public
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_scale(const NodePath &other, const LVecBase3f &scale) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_sx(const NodePath &other, float sx) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[0] = sx;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_sy(const NodePath &other, float sy) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[1] = sy;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_sz(const NodePath &other, float sz) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[2] = sz;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Public
//  Description: Returns the relative scale of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_scale(const NodePath &other) const {
  LMatrix4f mat = get_mat(other);

  // We decompose the scale directly instead of calling
  // decompose_matrix(), since we don't care about the hpr and don't
  // need to go through all the work of unrolling the axes.

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  x = mat.get_row3(0);
  y = mat.get_row3(1);
  z = mat.get_row3(2);

  // Now return the lengths of these axes as the scale.
  return LVecBase3f(length(x), length(y), length(z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Public
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr(const NodePath &other, const LVecBase3f &pos, 
	    const LVecBase3f &hpr) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, old_pos;
  decompose_matrix(mat, scale, old_hpr, old_pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Public
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr_scale(const NodePath &other, 
		  const LVecBase3f &pos, const LVecBase3f &hpr,
		  const LVecBase3f &scale) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  LMatrix4f mat;
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_mat
//       Access: Public
//  Description: Converts the indicated matrix from the other's
//               coordinate space to the local coordinate space, and
//               applies it to the arc.
////////////////////////////////////////////////////////////////////
void NodePath::
set_mat(const NodePath &other, const LMatrix4f &mat) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting transform on data graph arc.\n"
      << "(This is probably meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  LMatrix4f new_mat;

  // First, we perform a wrt to the node's parent, to get the
  // conversion matrix.
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  if (other.is_empty()) {
    wrt(NULL,
	_head->_arc->get_parent(), ForwardIterator(_head->_next), ForwardIterator(),
	ntw, _graph_type);
  } else {
    wrt(other.get_bottom_node(), ForwardIterator(other._head), ForwardIterator(),
	_head->_arc->get_parent(), ForwardIterator(_head->_next), ForwardIterator(),
	ntw, _graph_type);
  }
  const TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    new_mat = mat;
  } else {
    new_mat = mat * tt->get_matrix();
  }

  _head->_arc->set_transition(new TransformTransition(new_mat));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_mat
//       Access: Public
//  Description: Returns the matrix that describes the coordinate
//               space of the bottom node, relative to the other
//               path's bottom node's coordinate space.
////////////////////////////////////////////////////////////////////
LMatrix4f NodePath::
get_mat(const NodePath &other) const {
  nassertr(!is_empty(), LMatrix4f::ident_mat());

  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  if (other.is_empty()) {
    wrt(get_bottom_node(), ForwardIterator(_head), ForwardIterator(),
	(Node *)NULL, ntw, _graph_type);
  } else {
    wrt(get_bottom_node(), ForwardIterator(_head), ForwardIterator(),
	other.get_bottom_node(), ForwardIterator(other._head), ForwardIterator(),
	ntw, _graph_type);
  }
  const TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    return LMatrix4f::ident_mat();
  } else {
    return tt->get_matrix();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Public
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space, which
//               is relative to the other NodePath.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
void NodePath::
look_at(const NodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  LPoint3f pos = get_pos();

  NodePath parent(*this);
  parent.shorten(1);
  LPoint3f rel_point = point * other.get_mat(parent);

  LMatrix4f mat;
  ::look_at(mat, rel_point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Public
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up(const NodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  LPoint3f pos = get_pos();

  NodePath parent(*this);
  parent.shorten(1);
  LPoint3f rel_point = point * other.get_mat(parent);

  LMatrix4f mat;
  ::heads_up(mat, rel_point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color
//       Access: Public
//  Description: Sets the color transition for a render relation 
////////////////////////////////////////////////////////////////////
void NodePath::
set_color(const Colorf &color, int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  ColorTransition *col_trans = new ColorTransition(color);
  _head->_arc->set_transition(col_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_color
//       Access: Public
//  Description: Returns the color that has been assigned to the arc,
//               or black if no color has been assigned.
////////////////////////////////////////////////////////////////////
Colorf NodePath::
get_color() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  const ColorTransition *ct;
  if (get_transition_into(ct, _head->_arc)) {
    if (ct->is_on() && ct->is_real()) {
      return ct->get_color();
    }
  }

  sgmanip_cat.warning()
    << "get_color() called on " << *this << " which has no color set.\n";
  
  return Colorf(0.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_texture
//       Access: Public
//  Description: Sets the geometry at this level and below to render
//               using the indicated texture.
////////////////////////////////////////////////////////////////////
void NodePath::
set_texture(Texture *tex, int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  nassertv(tex != NULL);

  TextureTransition *tex_trans = new TextureTransition(tex);
  _head->_arc->set_transition(tex_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_texture_off
//       Access: Public
//  Description: Sets the geometry at this level and below to render
//               using no texture.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_texture() at a higher node level (or, with a
//               priority, to override a set_texture() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void NodePath::
set_texture_off(int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  TextureTransition *tex_trans = 
    new TextureTransition(TextureTransition::off());
  _head->_arc->set_transition(tex_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_texture
//       Access: Public
//  Description: Returns true if a texture has been applied to this
//               particular arc via set_texture(), false otherwise.
//               This is not the same thing as asking whether the
//               geometry at this node will be rendered with
//               texturing, as there may be a texture in effect from a
//               higher or lower level.
////////////////////////////////////////////////////////////////////
bool NodePath::
has_texture() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  const TextureTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    return tt->is_on();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_texture_off
//       Access: Public
//  Description: Returns true if a texture has been specifically
//               disabled on this particular arc via
//               set_texture_off(), false otherwise.  This is not the
//               same thing as asking whether the geometry at this
//               node will be rendered untextured, as there may be a
//               texture in effect from a higher or lower level.
////////////////////////////////////////////////////////////////////
bool NodePath::
has_texture_off() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  const TextureTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    return tt->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_texture
//       Access: Public
//  Description: Returns the texture that has been set on this
//               particular arc, or NULL if no texture has been set.
//               This is not necessarily the texture that will be
//               applied to the geometry at or below this level, as
//               another texture at a higher or lower level may
//               override.
////////////////////////////////////////////////////////////////////
Texture *NodePath::
get_texture() const {
  nassertr(has_arcs(), NULL);
  nassertr(_head != (ArcComponent *)NULL, NULL);

  const TextureTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    if (tt->is_on()) {
      return tt->get_texture();
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fog
//       Access: Public
//  Description: Sets the geometry at this level and below to render
//               using the indicated fog.
////////////////////////////////////////////////////////////////////
void NodePath::
set_fog(Fog *fog, int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  nassertv(fog != NULL);

  FogTransition *fog_trans = new FogTransition(fog);
  _head->_arc->set_transition(fog_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fog_off
//       Access: Public
//  Description: Sets the geometry at this level and below to render
//               using no fog.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_fog() at a higher node level (or, with a
//               priority, to override a set_fog() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void NodePath::
set_fog_off(int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  FogTransition *fog_trans = 
    new FogTransition(FogTransition::off());
  _head->_arc->set_transition(fog_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_fog
//       Access: Public
//  Description: Returns true if a fog has been applied to this
//               particular arc via set_fog(), false otherwise.
//               This is not the same thing as asking whether the
//               geometry at this node will be rendered with
//               fog, as there may be a fog in effect from a higher or
//               lower level.
////////////////////////////////////////////////////////////////////
bool NodePath::
has_fog() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  const FogTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    return tt->is_on();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_fog_off
//       Access: Public
//  Description: Returns true if a fog has been specifically
//               disabled on this particular arc via
//               set_fog_off(), false otherwise.  This is not the
//               same thing as asking whether the geometry at this
//               node will be rendered unfogged, as there may be a
//               fog in effect from a higher or lower level.
////////////////////////////////////////////////////////////////////
bool NodePath::
has_fog_off() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  const FogTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    return tt->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_fog
//       Access: Public
//  Description: Returns the fog that has been set on this
//               particular arc, or NULL if no fog has been set.
//               This is not necessarily the fog that will be
//               applied to the geometry at or below this level, as
//               another fog at a higher or lower level may
//               override.
////////////////////////////////////////////////////////////////////
Fog *NodePath::
get_fog() const {
  nassertr(has_arcs(), NULL);
  nassertr(_head != (ArcComponent *)NULL, NULL);

  const FogTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    if (tt->is_on()) {
      return tt->get_fog();
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_render_mode_wireframe
//       Access: Public
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in wireframe mode.
////////////////////////////////////////////////////////////////////
void NodePath::
set_render_mode_wireframe(int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new RenderModeTransition(RenderModeProperty::M_wireframe), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_render_mode_filled
//       Access: Public
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in filled (i.e. not wireframe)
//               mode.
////////////////////////////////////////////////////////////////////
void NodePath::
set_render_mode_filled(int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  RenderModeTransition *rmt =
    new RenderModeTransition(RenderModeProperty::M_filled);
  _head->_arc->set_transition(rmt, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_two_sided
//       Access: Public
//  Description: Specifically sets or disables two-sided rendering
//               mode on this particular arc.  If no other arcs
//               override, this will cause backfacing polygons to be
//               drawn (in two-sided mode, true) or culled (in
//               one-sided mode, false).
////////////////////////////////////////////////////////////////////
void NodePath::
set_two_sided(bool two_sided, int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  CullFaceProperty::Mode mode =
    two_sided ? 
    CullFaceProperty::M_cull_none :
    CullFaceProperty::M_cull_clockwise;

  CullFaceTransition *cft = new CullFaceTransition(mode);
  _head->_arc->set_transition(cft, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_two_sided
//       Access: Public
//  Description: Returns true if two-sided rendering has been
//               specifically set on this arc via set_two_sided(), or
//               false if one-sided rendering has been specifically
//               set, or if nothing has been specifically set.  See
//               also has_two_sided().  This does not necessarily
//               imply that the geometry will or will not be rendered
//               two-sided, as there may be other arcs that override.
////////////////////////////////////////////////////////////////////
bool NodePath::
get_two_sided() const {
  nassertr(has_arcs(), NULL);
  nassertr(_head != (ArcComponent *)NULL, NULL);

  const CullFaceTransition *cft;
  if (get_transition_into(cft, _head->_arc)) {
    return (cft->get_mode() == CullFaceProperty::M_cull_none);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_transparency
//       Access: Public
//  Description: Specifically sets or disables transparent rendering
//               mode on this particular arc.  If no other arcs
//               override, this will cause items with a non-1 value
//               for alpha color to be rendered partially transparent.
////////////////////////////////////////////////////////////////////
void NodePath::
set_transparency(bool transparency, int priority) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  TransparencyProperty::Mode mode =
    transparency ? 
    TransparencyProperty::M_alpha :
    TransparencyProperty::M_none;

  TransparencyTransition *tt = new TransparencyTransition(mode);
  _head->_arc->set_transition(tt, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_transparency
//       Access: Public
//  Description: Returns true if transparent rendering has been
//               specifically set on this arc via set_transparency(), or
//               false if nontransparent rendering has been specifically
//               set, or if nothing has been specifically set.  See
//               also has_transparency().  This does not necessarily
//               imply that the geometry will or will not be rendered
//               transparent, as there may be other arcs that override.
////////////////////////////////////////////////////////////////////
bool NodePath::
get_transparency() const {
  nassertr(has_arcs(), NULL);
  nassertr(_head != (ArcComponent *)NULL, NULL);

  const TransparencyTransition *tt;
  if (get_transition_into(tt, _head->_arc)) {
    return (tt->get_mode() != TransparencyProperty::M_none);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_hidden
//       Access: Public
//  Description: Returns true if some arc above this bottom node has
//               been set to 'hide', false if it should be visible.
////////////////////////////////////////////////////////////////////
bool NodePath::
is_hidden() const {
  return !get_hidden_ancestor().is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hidden_ancestor
//       Access: Public
//  Description: Returns a NodePath indicating the lowest arc above
//               this node which has been set to 'hide'.  Calling
//               show() on the NodePath returned by this function
//               should make the node visible again (unless there is
//               another arc further up that also has the 'hide'
//               transition set).
//
//               This function returns an empty NodePath if no
//               ancestors have been set to 'hide'.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
get_hidden_ancestor() const {
  if (!has_arcs()) {
    return NodePath();
  }
  nassertr(_head != (ArcComponent *)NULL, NodePath());
  if (_head->_arc->has_transition(PruneTransition::get_class_type())) {
    return *this;
  }

  NodePath next(*this);
  next.shorten();
  return next.get_hidden_ancestor();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::prepare_scene
//       Access: Public
//  Description: Walks through the scene graph beginning at the bottom
//               node, and does whatever initialization is required to
//               render the scene properly with the indicated GSG.  It
//               is not strictly necessary to call this, since the GSG
//               will initialize itself when the scene is rendered,
//               but this may take some of the overhead away from that
//               process.
////////////////////////////////////////////////////////////////////
void NodePath::
prepare_scene(GraphicsStateGuardianBase *gsg) {
  nassertv(!is_empty());

  // Use the ScenePrepareVisitor and fire off a traversal of the scene
  // beginning at the bottom node.  The ScenePrepareVisitor (defined
  // above) will call prepare() on each texture it finds in the scene
  // graph at this point and below.
  ScenePrepareVisitor visitor;
  visitor._gsg = gsg;

  NodeAttributeWrapper initial(TextureTransition::get_class_type());
  df_traverse(node(), visitor, initial, NullLevelState(), 
	      RenderRelation::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show_bounds
//       Access: Public
//  Description: Causes the bounding volume of the bottom node and all
//               of its descendants (that is, the bounding volume
//               associated with the the bottom arc) to be rendered,
//               if possible.  The rendering method is less than
//               optimal; this is intended primarily for debugging.
////////////////////////////////////////////////////////////////////
void NodePath::
show_bounds() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new DrawBoundsTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide_bounds
//       Access: Public
//  Description: Stops the rendering of the bounding volume begun with
//               show_bounds().
////////////////////////////////////////////////////////////////////
void NodePath::
hide_bounds() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->clear_transition(DrawBoundsTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_bounds
//       Access: Public
//  Description: Returns a newly-allocated bounding volume containing
//               the bottom node and all of its descendants.  This is
//               the bounding volume on the bottom arc, converted to
//               the local coordinate space of the node.
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) NodePath::
get_bounds() const {
  nassertr(has_arcs(), new BoundingSphere);

  PT(BoundingVolume) bv = _head->_arc->get_bound().make_copy();
  if (bv->is_of_type(GeometricBoundingVolume::get_class_type()) &&
      _head->_arc->has_transition(TransformTransition::get_class_type())) {

    // The bounding volume has already been transformed by the arc's
    // matrix.  We'd rather return a bounding volume in the node's
    // space, so we have to untransform it now.  Ick.
    GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bv);
    NodePath parent(*this);
    parent.shorten(1);
    LMatrix4f mat = parent.get_mat(*this);
    gbv->xform(mat);
  }

  return bv;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::write_bounds
//       Access: Public
//  Description: Writes a description of the bounding volume
//               containing the bottom node and all of its descendants
//               to the indicated output stream.
////////////////////////////////////////////////////////////////////
void NodePath::
write_bounds(ostream &out) const {
  get_bounds()->write(out);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_equiv
//       Access: Private
//  Description: The recursive implementation of operator ==.  Returns
//               true if the list of arcs is equivalent, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool NodePath::
r_equiv(const ArcComponent *next, const ArcComponent *other) const {
  if (next == (const ArcComponent *)NULL) { 
    nassertr(other == (const ArcComponent *)NULL, false);
    return true;
  }

  nassertr(next != (const ArcComponent *)NULL, false);
  nassertr(other != (const ArcComponent *)NULL, false);

  return (next == other ||
	  (next->_arc == other->_arc && r_equiv(next->_next, other->_next)));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_extend
//       Access: Private
//  Description: The recursive implementation of extend_by(NodePath).
//               This must walk to the end of the other path's
//               component list and extend in forward order.
////////////////////////////////////////////////////////////////////
bool NodePath::
r_extend_by(const ArcComponent *other) {
  if (other == (const ArcComponent *)NULL) {
    return true;
  }
  if (!r_extend_by(other->_next)) {
    return false;
  }

  if (!extend_by(other->_arc)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_as_string
//       Access: Private
//  Description: The recursive implementation of as_string.  This
//               must reverse the order of the path components.
////////////////////////////////////////////////////////////////////
int NodePath::
r_as_string(const ArcComponent *comp, string &result, int skip_nodes) const {
  nassertr(comp != (const ArcComponent *)NULL, 0);

  if (comp->_next == (const ArcComponent *)NULL) {
    // Here's the end of the chain.
    if (skip_nodes == 0) {
      // Skip no nodes; return the full name.
      result = format_node_name(comp->_arc->get_parent()) + "/" +
	format_node_name(comp->_arc->get_child());

    } else if (skip_nodes == 1) {
      // Skip the first node.
      result = format_node_name(comp->_arc->get_child());
    }

    return 2;

  } else {
    int nodes_before = r_as_string(comp->_next, result, skip_nodes);
    if (skip_nodes <= nodes_before) {
      if (skip_nodes != nodes_before) {
	// This is not the first node, so format a slash between the
	// previous node and this node.

	if (comp->_arc->get_parent() == comp->_next->_arc->get_child()) {
	  result += "/";
	} else {
	  // Unless the path is broken here.  In this case, insert a
	  // visual indication of the break.
	  result += "/.../" + format_node_name(comp->_arc->get_parent()) + "/";
	}
      }
      result += format_node_name(comp->_arc->get_child());
    }
    return nodes_before + 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_write_transitions
//       Access: Private
//  Description: The recursive implementation of write_transitions().
////////////////////////////////////////////////////////////////////
void NodePath::
r_write_transitions(const ArcComponent *comp,
		    ostream &out, int indent_level) const {
  nassertv(comp != (const ArcComponent *)NULL);
  nassertv(comp->_arc != (NodeRelation *)NULL);

  if (comp->_next != (const ArcComponent *)NULL) {
    r_write_transitions(comp->_next, out, indent_level);
  }

  indent(out, indent_level) << *comp->_arc << "\n";
  comp->_arc->write_transitions(out, indent_level + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_get_net_transitions
//       Access: Private
//  Description: The recursive implementation of get_net_transitions().
////////////////////////////////////////////////////////////////////
void NodePath::
r_get_net_transitions(const ArcComponent *comp, 
		      AllTransitionsWrapper &trans) const {
  nassertv(comp != (const ArcComponent *)NULL);
  nassertv(comp->_arc != (NodeRelation *)NULL);

  if (comp->_next != (const ArcComponent *)NULL) {
    r_get_net_transitions(comp->_next, trans);
  }

  AllTransitionsWrapper arc_trans;
  arc_trans.extract_from(comp->_arc);
  trans.compose_in_place(arc_trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::format_node_name
//       Access: Private
//  Description: Formats one component of the as_string string: a
//               node, either unnamed as a type, or as a name.
////////////////////////////////////////////////////////////////////
string NodePath::
format_node_name(Node *node) const {
  string name;
  if (node->is_of_type(NamedNode::get_class_type())) {
    name = DCAST(NamedNode, node)->get_name();
  }

  if (name.empty()) {
    // No name.  If the type isn't one of the trivial types (Node or
    // NamedNode), use the type name instead, since it's likely to be
    // more unique.
    if (!node->is_of_type(Node::get_class_type()) &&
	!node->is_of_type(NamedNode::get_class_type())) {
      return "-" + node->get_type().get_name();
    }
  }

  // Use the node's name.
  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_matches
//       Access: Private
//  Description: Finds up to max_matches matches against the given
//               path string from this node and deeper.  The
//               max_matches count indicates the maximum number of
//               matches to return, or -1 not to limit the number
//               returned.
////////////////////////////////////////////////////////////////////
void NodePath::
find_matches(NodePathCollection &result, const string &path,
	     int max_matches) const {
  if (is_empty()) {
    sgmanip_cat.warning()
      << "Attempt to extend an empty NodePath by '" << path
      << "'.\n";
    return;
  }
  FindApproxPath approx_path;
  if (approx_path.add_string(path)) {
    find_matches(result, approx_path, max_matches);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_matches
//       Access: Private
//  Description: Finds up to max_matches matches against the given
//               approx_path from this node and deeper.  The
//               max_matches count indicates the maximum number of
//               matches to return, or -1 not to limit the number
//               returned.
////////////////////////////////////////////////////////////////////
void NodePath::
find_matches(NodePathCollection &result, FindApproxPath &approx_path,
	     int max_matches) const {
  if (is_empty()) {
    sgmanip_cat.warning()
      << "Attempt to extend an empty NodePath by: " << approx_path << ".\n";
    return;
  }
  FindApproxLevelEntry start(*this, approx_path);
  FindApproxLevel level;
  level.add_entry(start);
  r_find_matches(result, level, max_matches, _max_search_depth);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_find_matches
//       Access: Private
//  Description: The recursive implementation of find_matches.
////////////////////////////////////////////////////////////////////
void NodePath::
r_find_matches(NodePathCollection &result,
	       const FindApproxLevel &level, 
	       int max_matches, int num_levels_remaining) const {
  // Go on to the next level.  If we exceeded the requested maximum
  // depth, stop.
  if (num_levels_remaining <= 0) {
    return;
  }
  num_levels_remaining--;

  FindApproxLevel next_level;

  // For each node in the current level, build up the set of possible
  // matches in the next level.
  FindApproxLevel::Vec::const_iterator li;
  for (li = level._v.begin(); li != level._v.end(); ++li) {
    const FindApproxLevelEntry &entry = (*li);
    if (entry.is_solution()) {
      // Does this entry already represent a solution?
      result.add_path(entry._node_path);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
	return;
      }

    } else {
      Node *node = entry._node_path.get_bottom_node();
      nassertv(node != (Node *)NULL);

      DownRelations::const_iterator dri;
      dri = node->_children.find(_graph_type);
      if (dri != node->_children.end()) {
	const DownRelationPointers &drp = (*dri).second;
	
	DownRelationPointers::const_iterator drpi;
	for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
	  NodeRelation *arc = (*drpi);
	  entry.consider_next_step(result, arc, next_level, max_matches);
	  if (max_matches > 0 && result.get_num_paths() >= max_matches) {
	    return;
	  }
	}
      }
    }
  }

  // Now recurse on the next level.
  r_find_matches(result, next_level, max_matches, num_levels_remaining);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_list_descendants
//       Access: Private
//  Description: The recursive implementation of ls().
////////////////////////////////////////////////////////////////////
void NodePath::
r_list_descendants(ostream &out, int indent_level) const {
  Node *node = get_bottom_node();
  nassertv(node != (Node *)NULL);
  indent(out, indent_level) << *node << "\n";
  
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      NodePath next(*this);
      next.extend_by(arc);
      next.r_list_descendants(out, indent_level + 2);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_list_transitions
//       Access: Private
//  Description: The recursive implementation of ls_transitions().
////////////////////////////////////////////////////////////////////
void NodePath::
r_list_transitions(ostream &out, int indent_level) const {
  Node *node = get_bottom_node();
  nassertv(node != (Node *)NULL);

  out << "\n+";
  indent(out, indent_level + 1) << *node << "\n\n";
  
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      NodePath next(*this);
      next.extend_by(arc);

      indent(out, indent_level + 2) << *arc << ":\n";
      arc->write_transitions(out, indent_level + 2);
      next.r_list_transitions(out, indent_level + 2);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_adjust_all_priorities
//       Access: Private
//  Description: The recursive implementation of
//               adjust_all_priorities().  This walks through the
//               subgraph defined by the indicated arc and below.
////////////////////////////////////////////////////////////////////
void NodePath::
r_adjust_all_priorities(NodeRelation *arc, int adjustment) {
  arc->adjust_all_priorities(adjustment);

  Node *node = arc->get_child();
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      r_adjust_all_priorities(*drpi, adjustment);
    }
  }
}
