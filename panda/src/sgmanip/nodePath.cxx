// Filename: nodePath.cxx
// Created by:  drose (05Mar00)
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
#include <materialPool.h>
#include <pt_NodeRelation.h>
#include <config_gobj.h>
#include <geomNode.h>
#include <geom.h>

#include "plist.h"

int NodePath::_max_search_depth = 10000;
TypeHandle NodePath::_type_handle;


// This class is used in prepare_scene() to traverse the scene graph
// and register textures and geoms with the gsg.
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

  bool reached_node(Node *node, NodeAttributeWrapper &,
                    NullLevelState &) {
    if (_retained_mode && node->is_of_type(GeomNode::get_class_type())) {
      GeomNode *gnode = DCAST(GeomNode, node);
      gnode->prepare(_gsg);
    }
    return true;
  }

  GraphicsStateGuardianBase *_gsg;
  bool _retained_mode;
};

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Published
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
extend_by(Node *dnode) {
  nassertr(_error_type == ET_ok, false);
  nassertr(verify_connectivity(), false);
  nassertr(dnode != (Node *)NULL, false);

  if (is_empty()) {
    // Extending an empty NodePath by a single node is the same thing
    // as creating a new NodePath with just the single node.
    (*this) = NodePath(dnode, _graph_type);
    return true;
  }

  Node *bottom_node = node();
  NodeRelation *arc = find_arc(bottom_node, dnode, _graph_type);
  if (arc == (NodeRelation *)NULL) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
        << "Cannot extend " << *this << " by "
        << *dnode << "; no connection.\n";
    }
    return false;
  }

  _head = new ArcComponent(arc, _head);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Published
//  Description: Adds the indicated arc to the end of the path.  The
//               arc must connect the bottom node of the path to some
//               other node.  Returns true if this is so, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(NodeRelation *darc) {
  nassertr(_error_type == ET_ok, false);
  nassertr(verify_connectivity(), false);
  nassertr(darc != (NodeRelation *)NULL, false);

  if (is_empty()) {
    // If the NodePath is initially empty, we must start with its top
    // node.
    (*this) = NodePath(darc->get_parent(), darc->get_type());
  }

  if (darc->get_parent() != node()) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
        << "Cannot extend " << *this << " by arc " << *darc
        << "; no connection.\n";
    }
    return false;
  }

  _head = new ArcComponent(darc, _head);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::extend_by
//       Access: Published
//  Description: Adds the indicated NodePath to the end of this path.
//               The top node of the other NodePath must be the same
//               node as the bottom node of this path.  Returns true
//               if this is so, false otherwise or if the other path
//               is no longer accurately connected.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(const NodePath &other) {
  nassertr(_error_type == ET_ok, false);
  nassertr(other._error_type == ET_ok, false);
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
//       Access: Published
//  Description: Extends the NodePath by the NodePath represented by
//               the given approximate string.  If the match is
//               ambiguous, returns the first match found (which will
//               be the shortest path).  Returns true if the extension
//               was made, false if the node could not be found.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_by(const string &path) {
  nassertr(_error_type == ET_ok, false);
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
//       Access: Published
//  Description: Extends the NodePath down along the shortest
//               available path to the indicated node, even if the
//               node is several levels below the NodePath's bottom
//               node.  Returns true if the extension was made, false
//               if the node was not below this NodePath.
////////////////////////////////////////////////////////////////////
bool NodePath::
extend_down_to(Node *dnode) {
  nassertr(_error_type == ET_ok, false);
  if (!is_empty() && node() == dnode) {
    // We're already there!
    return true;
  }

  nassertr(verify_connectivity(), false);
  NodePathCollection col;
  FindApproxPath approx_path;
  approx_path.add_match_many(0);
  approx_path.add_match_pointer(dnode, 0);
  find_matches(col, approx_path, -1);

  if (col.is_empty()) {
    if (sgmanip_cat.is_debug()) {
      sgmanip_cat.debug()
        << "Could not extend " << *this << " down to "
        << *dnode << "; no connection found.\n";
    }
    return false;
  }

  (*this) = col.get_path(0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::shorten
//       Access: Published
//  Description: Shortens the NodePath by removing the indicated
//               number of nodes from the bottom of the path.  It
//               is an error to shorten the path to less than zero
//               nodes; thus, num_nodes must be <= get_num_nodes().
////////////////////////////////////////////////////////////////////
void NodePath::
shorten(int num_nodes) {
  nassertv(_error_type == ET_ok);
  nassertv(num_nodes >= 0 && num_nodes <= get_num_nodes());

  int count = num_nodes;
  while (count > 0) {
    nassertv(_head != (ArcComponent *)NULL);
    _head = _head->get_next();
    count--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear
//       Access: Published
//  Description: Resets the NodePath to an empty path.
////////////////////////////////////////////////////////////////////
void NodePath::
clear() {
  _head.clear();
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_children
//       Access: Published
//  Description: Returns the set of all child nodes of the bottom
//               node.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
get_children() const {
  NodePathCollection result;
  nassertr(verify_connectivity(), result);
  nassertr(!is_empty(), result);

  Node *bottom_node = node();

  int num_children = bottom_node->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = bottom_node->get_child(_graph_type, i);
    NodePath child(*this);
    child.extend_by(child_arc);
    result.add_path(child);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_siblings
//       Access: Published
//  Description: Returns the set of all child nodes of the bottom
//               node's parent, except the bottom node itself.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
get_siblings() const {
  NodePathCollection result;
  nassertr(verify_connectivity(), result);
  nassertr(has_arcs(), result);
  NodeRelation *my_arc = arc();

  NodePath parent = *this;
  parent.shorten(1);

  Node *parent_node = parent.node();

  int num_children = parent_node->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = parent_node->get_child(_graph_type, i);

    if (child_arc != my_arc) {
      NodePath sib(parent);
      sib.extend_by(child_arc);
      result.add_path(sib);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_all_paths_down_to
//       Access: Published
//  Description: Returns the set of all NodePaths that extend from
//               this NodePath down to the indicated node.  The
//               shortest paths will be listed first.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
find_all_paths_down_to(Node *dnode) const {
  NodePathCollection col;
  nassertr(!is_empty(), col);
  nassertr(verify_connectivity(), col);
  nassertr(dnode != (Node *)NULL, col);
  FindApproxPath approx_path;
  approx_path.add_match_many(0);
  approx_path.add_match_pointer(dnode, 0);
  find_matches(col, approx_path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_all_matches
//       Access: Published
//  Description: Returns the complete set of all NodePaths that begin
//               with this NodePath and can be extended by
//               path.  The shortest paths will be listed
//               first.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePath::
find_all_matches(const string &path) const {
  NodePathCollection col;
  nassertr(!is_empty(), col);
  nassertr(verify_connectivity(), col);
  find_matches(col, path, -1);
  return col;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_singular_transform
//       Access: Published
//  Description: Scans the subgraph beginning at the bottom node of
//               the scene graph, looking for a node with a singular
//               matrix transform above it.  Returns the first such
//               node encountered, or an empty NodePath if no singular
//               transforms exist in the scene graph.
//
//               This is a handy function for tracking down mysterious
//               "tried to invert a singular matrix" errors, which are
//               almost always caused by zero-scale transform matrices
//               in the scene graph.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
find_singular_transform() const {
  nassertr(!is_empty(), NodePath::fail());

  if (has_mat()) {
    LMatrix4f mat;
    if (!mat.invert_from(get_mat())) {
      // Here's a singular matrix!
      return *this;
    }
  }

  int num_children = get_num_children();
  for (int i = 0; i < num_children; i++) {
    NodePath result = get_child(i).find_singular_transform();
    if (!result.is_empty()) {
      return result;
    }
  }

  return NodePath::not_found();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::share_with
//       Access: Published
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
  typedef plist<ArcComponent *> Comps;
  Comps other_comps;
  Comps this_comps;

  // First, we have to reverse the lists of this path and the other
  // path, so we can compare their initial subsets.
  ArcComponent *comp = other._head;
  while (comp != (ArcComponent *)NULL) {
    other_comps.push_front(comp);
    comp = comp->get_next();
  }

  comp = _head;
  while (comp != (ArcComponent *)NULL) {
    this_comps.push_front(comp);
    comp = comp->get_next();
  }

  // Now determine how many components this and the other path have in
  // common.
  int in_common = 0;
  Comps::const_iterator oi = other_comps.begin();
  Comps::const_iterator ti = this_comps.begin();

  // The first components in both chains should be the topnodes.
  if (oi != other_comps.end() &&
      ti != this_comps.end() &&
      (*oi)->get_node() == (*ti)->get_node()) {
    ++oi;
    ++ti;
    ++in_common;
  }
  // The remaining components are arcs.
  while (oi != other_comps.end() &&
         ti != this_comps.end() &&
         (*oi)->get_arc() == (*ti)->get_arc()) {
    ++oi;
    ++ti;
    ++in_common;
  }

  if (in_common > 0) {
    ArcComponent *dest;
    if (oi == other_comps.end()) {
      dest = other._head;
    } else {
      dest = (*oi);
    }

    if (ti == this_comps.end()) {
      _head = dest;
    } else {
      (*ti)->set_next(dest);
    }
  }

  return in_common;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::verify_connectivity
//       Access: Published
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
  nassertr(comp->has_arc(), false);

  NodeRelation *darc = comp->get_arc();
  Node *parent = darc->get_parent();
  Node *child = darc->get_child();
  if (parent == (Node *)NULL || child == (Node *)NULL) {
    return false;
  }

  comp = comp->get_next();
  nassertr(comp != (const ArcComponent *)NULL, false);
  while (comp->has_arc()) {
    NodeRelation *darc = comp->get_arc();
    Node *next_parent = darc->get_parent();
    Node *next_child = darc->get_child();
    if (next_parent == (Node *)NULL || next_child == (Node *)NULL) {
      return false;
    }
    if (next_child != parent) {
      return false;
    }

    parent = next_parent;
    child = next_child;
    comp = comp->get_next();
    nassertr(comp != (const ArcComponent *)NULL, false);
  }

  // Finally, verify the top node.
  nassertr(comp->is_top_node(), false);
  if (parent != comp->get_node()) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::amputate_badness
//       Access: Published
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

  NodeRelation *darc = comp->get_arc();
  Node *parent = darc->get_parent();
  Node *child = darc->get_child();
  if (parent == (Node *)NULL || child == (Node *)NULL) {
    // Eek!  The bottom arc is broken!
    _head = (ArcComponent *)NULL;
    return false;
  }

  ArcComponent *prev = comp;
  comp = comp->get_next();
  nassertr(comp != (const ArcComponent *)NULL, false);
  while (comp->has_arc()) {
    NodeRelation *darc = comp->get_arc();
    Node *next_parent = darc->get_parent();
    Node *next_child = darc->get_child();

    if (next_parent == (Node *)NULL || next_child == (Node *)NULL) {
      prev->set_next(new ArcComponent(parent));
      return false;
    }
    if (next_child != parent) {
      prev->set_next(new ArcComponent(parent));
      return false;
    }

    parent = next_parent;
    child = next_child;
    prev = comp;
    comp = comp->get_next();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::repair_connectivity
//       Access: Published
//  Description: When a NodePath has been inadvertantly disconnected,
//               this attempts to repair the damage by finding a path
//               from the bottom of the top NodePath to the top of the
//               still-connected part of this NodePath.  It returns
//               true if the connection can be found, false if not (in
//               which case the NodePath is unchanged).
////////////////////////////////////////////////////////////////////
bool NodePath::
repair_connectivity(const NodePath &top) {
  nassertr(top.verify_connectivity(), false);

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
//       Access: Published
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
reparent_to(const NodePath &other, int sort) {
  nassertv(other.verify_connectivity());
  nassertv_always(has_arcs());
  nassertv_always(!other.is_empty());

  NodeRelation *darc = arc();
  darc->change_parent(other.node(), sort);

  // Move our head pointer to the bottom of the new chain.  This will
  // update our own path, as well as all paths that share the same
  // head pointer (i.e. all paths derived from this one).
  _head->set_next(other._head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::wrt_reparent_to
//       Access: Published
//  Description: This functions identically to reparent_to(), except
//               the transform above this node is also adjusted so
//               that the node remains in the same place in world
//               coordinates, even if it is reparented into a
//               different coordinate system.
////////////////////////////////////////////////////////////////////
void NodePath::
wrt_reparent_to(const NodePath &other, int sort) {
  nassertv(other.verify_connectivity());
  nassertv_always(has_arcs());
  nassertv_always(!other.is_empty());

  LMatrix4f mat = get_mat(other);
  set_mat(mat);

  reparent_to(other, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::instance_to
//       Access: Published
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
instance_to(const NodePath &other, int sort) const {
  nassertr(verify_connectivity(), NodePath::fail());
  nassertr(!is_empty(), NodePath::fail());
  nassertr(!other.is_empty(), NodePath::fail());

  Node *bottom_node = node();
  NodeRelation *darc =
    NodeRelation::create_typed_arc(_graph_type, other.node(),
                                   bottom_node, sort);
  nassertr(darc != (NodeRelation *)NULL, NodePath::fail());
  nassertr(darc->is_exact_type(_graph_type), NodePath::fail());

  if (has_arcs()) {
    // Copy the transitions from this one's bottom arc, so the
    // instance will inherit the same local state by default.
    darc->copy_transitions_from(arc());
  }

  NodePath instance(*this);
  instance._head = new ArcComponent(darc, other._head);
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::copy_to
//       Access: Published
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
copy_to(const NodePath &other, int sort) const {
  nassertr(verify_connectivity(), NodePath::fail());
  nassertr(!is_empty(), NodePath::fail());
  nassertr(!other.is_empty(), NodePath::fail());

  Node *source_node = node();
  PT_Node copy_node = source_node->copy_subgraph(_graph_type);
  nassertr(copy_node != (Node *)NULL, NodePath::fail());

  NodeRelation *darc =
    NodeRelation::create_typed_arc(_graph_type, other.node(),
                                   copy_node, sort);
  nassertr(darc != (NodeRelation *)NULL, NodePath::fail());
  nassertr(darc->is_exact_type(_graph_type), NodePath::fail());

  if (has_arcs()) {
    // Copy the transitions from this one's bottom arc, so the
    // duplicate will inherit the same local state by default.
    darc->copy_transitions_from(arc());
  }

  NodePath instance(*this);
  instance._head = new ArcComponent(darc, other._head);
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::attach_new_node
//       Access: Published
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
attach_new_node(Node *dnode, int sort) const {
  nassertr(verify_connectivity(), NodePath::fail());
  nassertr(!is_empty(), NodePath(_graph_type));
  nassertr(dnode != (Node *)NULL, NodePath(_graph_type));

  NodePath path(dnode, _graph_type);
  return path.instance_to(*this, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::remove_node
//       Access: Published
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
//               After the node is removed, the NodePath will have
//               been cleared.
////////////////////////////////////////////////////////////////////
void NodePath::
remove_node() {
  nassertv(_error_type != ET_not_found);
  if (!has_arcs() || !arc()->is_attached()) {
    // If we have no arcs (maybe we were already removed), or if the
    // bottom arc has been disconnected (maybe a parent was removed),
    // quietly do nothing except to ensure the NodePath is clear.
    (*this) = NodePath::removed();
    return;
  }

  PT_NodeRelation darc = arc();
  PT_Node dnode = node();

  // First, disconnect the arc from the scene graph.  This will orphan
  // our bottom node.
  remove_arc(darc);

  // Set the chain to stop here, so that any NodePaths sharing this
  // one will now begin at this "deleted" node.
  ArcComponent here(dnode.p());
  (*_head) = here;

  // Now remove our own chain reference.  If there were no other
  // sharing NodePaths, this will also delete the complete chain,
  // including the above newly-changed _head node, and all of the
  // nodes at this point and below.

  // If there *are* some sharing NodePaths, they will keep this node
  // from being actually destructed (although it has been removed from
  // the scene graph), and they will now believe they are rooted at
  // that destructed node, cut off from the rest of the world.
  (*this) = NodePath::removed();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::as_string
//       Access: Published
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

  switch (_error_type) {
  case ET_not_found:
    return "**not found**";
  case ET_removed:
    return "**removed**";
  case ET_fail:
    return "**error**";
  default:
    break;
  }

  if (is_empty()) {
    // An empty path always returns an empty string.
    return string();
  }

  // In the normal case, we have at least one node.  We must walk to
  // the end of the list, then back up start_at_node times, and start
  // formatting names from there.
  string result;
  r_as_string(_head, result, start_at_node);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::write_transitions
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Reports some statistics about the scene graph
//               properties at and below this node.
////////////////////////////////////////////////////////////////////
void NodePath::
analyze() const {
  nassertv_always(!is_empty());
  SceneGraphAnalyzer sga(_graph_type);
  sga.add_node(node());
  sga.write(nout);
}


////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_light
//       Access: Published
//  Description: Lightly flattens out the hierarchy below this node by
//               applying transforms, colors, and texture matrices
//               from the arcs onto the vertices, but does not remove
//               any nodes.
//
//               This can result in improved rendering performance
//               because there will be fewer transforms in the
//               resulting scene graph, but the number of nodes will
//               remain the same.
//
//               Particularly, any NodePaths that reference nodes
//               within this hierarchy will not be damaged.  However,
//               since this operation will remove transforms from the
//               scene graph, it may be dangerous to apply to arcs
//               where you expect to dynamically modify the transform,
//               or where you expect the geometry to remain in a
//               particular local coordinate system.
//
//               The return value is always 0, since flatten_light
//               does not remove any arcs.
////////////////////////////////////////////////////////////////////
int NodePath::
flatten_light() {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  gr.apply_transitions(arc());

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_medium
//       Access: Published
//  Description: A more thorough flattening than flatten_light(), this
//               first applies all the transforms, colors, and texture
//               matrices from the arcs onto the vertices, and then
//               removes unneeded grouping nodes--nodes that have
//               exactly one child, for instance, but have no special
//               properties in themselves.
//
//               This results in improved perforamance over
//               flatten_light() because the number of nodes in the
//               scene graph is reduced.
//
//               If max_children is specified, it represents the
//               maximum number of children a node is allowed to have
//               and still be flattened.  Normally, this is 1; we
//               don't typically want to flatten a node that has
//               multiple children.  However, sometimes this may be
//               desirable; set this parameter to control the limit.
//               If this is set to -1, there is no limit.
//
//               The return value is the number of arcs removed.
////////////////////////////////////////////////////////////////////
int NodePath::
flatten_medium(int max_children) {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  gr.set_max_children(max_children);
  gr.apply_transitions(arc());
  int num_removed = gr.flatten(node(), false);

  if (sgmanip_cat.is_debug()) {
    sgmanip_cat.debug()
      << "flatten_medium() removed " << num_removed << " arcs.\n";
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::flatten_strong
//       Access: Published
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
flatten_strong(int max_children) {
  nassertr(!is_empty(), 0);
  SceneGraphReducer gr(_graph_type);
  gr.set_max_children(max_children);
  gr.apply_transitions(arc());
  int num_removed = gr.flatten(node(), true);

  if (sgmanip_cat.is_debug()) {
    sgmanip_cat.debug()
      << "flatten_strong() removed " << num_removed << " arcs.\n";
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::write_bam_file
//       Access: Published
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
    if (bam_file.write_object(node())) {
      okflag = true;
    }
    bam_file.close();
  }

  return okflag;
}


////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos(const LVecBase3f &pos) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  mat.set_row(3, pos);
  set_mat(mat);
}

void NodePath::
set_x(float x) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  mat(3, 0) = x;
  set_mat(mat);
}

void NodePath::
set_y(float y) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  mat(3, 1) = y;
  set_mat(mat);
}

void NodePath::
set_z(float z) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  mat(3, 2) = z;
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_pos
//       Access: Published
//  Description: Retrieves the translation component of the transform.
////////////////////////////////////////////////////////////////////
LPoint3f NodePath::
get_pos() const {
  nassertr(has_arcs(), LPoint3f(0.0, 0.0, 0.0));
  LMatrix4f mat = get_mat();
  return mat.get_row3(3);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_hpr(const LVecBase3f &hpr) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_h(float h) {
  nassertv(has_arcs());
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
//       Access: Published
//  Description: Retrieves the rotation component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_hpr() const {
  nassertr(has_arcs(), LVecBase3f(0.0, 0.0, 0.0));
  LMatrix4f mat = get_mat();
  LVecBase3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hpr
//       Access: Published
//  Description: Retrieves the rotation component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_hpr(float roll) const {
  nassertr(has_arcs(), LVecBase3f(0.0, 0.0, 0.0));
  LMatrix4f mat = get_mat();
  LVecBase3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos, roll);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_scale(const LVecBase3f &sv3) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  compose_matrix(mat, sv3, hpr, pos);
  set_mat(mat);
}

void NodePath::
set_sx(float sx) {
  nassertv(has_arcs());
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
  LMatrix4f mat = get_mat();
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[2] = sz;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Published
//  Description: Retrieves the scale component of the transform.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_scale() const {
  nassertr(has_arcs(), LVecBase3f(1.0, 1.0, 1.0));
  LMatrix4f mat = get_mat();

  // We decompose the scale directly instead of calling
  // decompose_matrix(), since we don't care about the hpr and don't
  // need to go through all the work of unrolling the axes.

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);


  // Now return the lengths of these axes as the scale.
  return LVecBase3f(length(x), length(y), length(z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color_scale
//       Access: Published
//  Description: Sets the color scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_color_scale(const LVecBase4f &sv4) {
  nassertv_always(has_arcs());

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting color scale on data graph arc.\n"
      << "(This is meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  NodeRelation *darc = _head->get_arc();
  if (sv4[0] != 1.0f || sv4[1] != 1.0f || sv4[2] != 1.0f) {
    LMatrix4f mat = LMatrix4f::scale_mat(sv4[0], sv4[1], sv4[2]);
    darc->set_transition(new ColorMatrixTransition(mat));
  }
  else {
    darc->clear_transition(ColorMatrixTransition::get_class_type());
  }

  if (sv4[3] != 1) {
    darc->set_transition(new AlphaTransformTransition(0.0f, sv4[3]));
  }
  else {
    darc->clear_transition(AlphaTransformTransition::get_class_type());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_color_scale
//       Access: Published
//  Description: Returns the complete transform vector that has been
//               applied to the bottom arc, or the all 1's if no
//               scale has been applied
////////////////////////////////////////////////////////////////////
LVecBase4f NodePath::
get_color_scale() const {
  nassertr(has_arcs(), LVecBase4f(1.0f,1.0f,1.0f,1.0f));

  LVecBase4f scale;

  NodeRelation *darc = arc();
  const ColorMatrixTransition *ct;
  if (!get_transition_into(ct, darc)) {
    // No relative transform.
    scale[0] = 1.0f;
    scale[1] = 1.0f;
    scale[2] = 1.0f;
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
  if (!get_transition_into(att, darc)) {
    scale[3] = 1.0f;
  }
  else {
    scale[3] = att->get_scale();
  }

  return scale;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr(const LVecBase3f &pos, const LVecBase3f &hpr) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat();
  LVecBase3f scale, old_hpr, old_pos;
  decompose_matrix(mat, scale, old_hpr, old_pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr_scale(const LVecBase3f &pos, const LVecBase3f &hpr,
                  const LVecBase3f &scale) {
  nassertv(has_arcs());
  LMatrix4f mat;
  compose_matrix(mat, scale, hpr, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_mat
//       Access: Published
//  Description: Directly sets an arbitrary 4x4 transform matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
set_mat(const LMatrix4f &mat) {
  nassertv_always(has_arcs());

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting transform on data graph arc.\n"
      << "(This is probably meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  arc()->set_transition(new TransformTransition(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Published
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.  See
//               also look_at_preserve_scale().
////////////////////////////////////////////////////////////////////
void NodePath::
look_at(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::look_at(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

  LPoint3f pos = get_pos();

  LMatrix4f mat;
  ::heads_up(mat, point - pos, up);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at_preserve_scale
//       Access: Published
//  Description: Functions like look_at(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, look_at() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
look_at_preserve_scale(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.

  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);
  ::look_at(mat, point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up_preserve_scale
//       Access: Published
//  Description: Functions like heads_up(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, heads_up() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up_preserve_scale(const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  // The lengths of the axes defines the scale.
  LPoint3f pos;
  mat.get_row3(pos,3);

  ::heads_up(mat, point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Published
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos(const NodePath &other, const LVecBase3f &pos) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  mat.set_row(3, pos);
  set_mat(other, mat);
}

void NodePath::
set_x(const NodePath &other, float x) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  mat(3, 0) = x;
  set_mat(other, mat);
}

void NodePath::
set_y(const NodePath &other, float y) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  mat(3, 1) = y;
  set_mat(other, mat);
}

void NodePath::
set_z(const NodePath &other, float z) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  mat(3, 2) = z;
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Published
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_hpr(const NodePath &other, const LVecBase3f &hpr) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_h(const NodePath &other, float h) {
  nassertv(has_arcs());
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
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, pos;
  decompose_matrix(mat, scale, old_hpr, pos);
  old_hpr[2] = r;
  compose_matrix(mat, scale, old_hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hpr
//       Access: Published
//  Description: Returns the relative orientation of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_hpr(const NodePath &other) const {
  LMatrix4f mat = get_mat(other);
  LVector3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hpr
//       Access: Published
//  Description: Returns the relative orientation of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
LVecBase3f NodePath::
get_hpr(const NodePath &other, float roll) const {
  LMatrix4f mat = get_mat(other);
  LVector3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos, roll);
  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Published
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_scale(const NodePath &other, const LVecBase3f &scale) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

void NodePath::
set_sx(const NodePath &other, float sx) {
  nassertv(has_arcs());
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
  LMatrix4f mat = get_mat(other);
  LVecBase3f old_scale, hpr, pos;
  decompose_matrix(mat, old_scale, hpr, pos);
  old_scale[2] = sz;
  compose_matrix(mat, old_scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Published
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
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // Now return the lengths of these axes as the scale.
  return LVecBase3f(length(x), length(y), length(z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Published
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr(const NodePath &other, const LVecBase3f &pos,
            const LVecBase3f &hpr) {
  nassertv(has_arcs());
  LMatrix4f mat = get_mat(other);
  LVecBase3f scale, old_hpr, old_pos;
  decompose_matrix(mat, scale, old_hpr, old_pos);
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Published
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
void NodePath::
set_pos_hpr_scale(const NodePath &other,
                  const LVecBase3f &pos, const LVecBase3f &hpr,
                  const LVecBase3f &scale) {
  nassertv(has_arcs());
  LMatrix4f mat;
  compose_matrix(mat, scale, hpr, pos);
  set_mat(other, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the coordinate
//               space of the bottom node, relative to the other
//               path's bottom node's coordinate space.
////////////////////////////////////////////////////////////////////
LMatrix4f NodePath::
get_mat(const NodePath &other) const {
  nassertr(_error_type == ET_ok && other._error_type == ET_ok,
           LMatrix4f::ident_mat());
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());

  if (is_empty() && other.is_empty()) {
    return LMatrix4f::ident_mat();

  } else if (is_empty()) {
    wrt(NULL, other.node(), other.begin(), other.end(),
        ntw, _graph_type);

  } else if (other.is_empty()) {
    wrt(node(), begin(), end(), (Node *)NULL, ntw, _graph_type);

  } else {
    wrt(node(), begin(), end(),
        other.node(), other.begin(), other.end(),
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
//     Function: NodePath::set_mat
//       Access: Published
//  Description: Converts the indicated matrix from the other's
//               coordinate space to the local coordinate space, and
//               applies it to the arc.
////////////////////////////////////////////////////////////////////
void NodePath::
set_mat(const NodePath &other, const LMatrix4f &mat) {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(has_arcs());

#ifndef NDEBUG
  if (_graph_type == DataRelation::get_class_type()) {
    sgmanip_cat.warning()
      << "Setting transform on data graph arc.\n"
      << "(This is probably meaningless.  Did you mean to do this to the bottom node?)\n";
  }
#endif

  NodeRelation *darc = arc();

  // First, we perform a wrt to the node's parent, to get the
  // conversion matrix.
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  ForwardIterator from = begin();
  ++from;

  if (other.is_empty()) {
    wrt(NULL, darc->get_parent(), from, end(), ntw, _graph_type);
  } else {
    wrt(other.node(), other.begin(), other.end(),
        darc->get_parent(), from, end(),
        ntw, _graph_type);
  }

  LMatrix4f new_mat,*new_mat_ptr;
  const TransformTransition *tt;

  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    new_mat_ptr = (LMatrix4f*)&mat;
  } else {
    new_mat.multiply(mat,tt->get_matrix());
    new_mat_ptr = &new_mat;
  }

  darc->set_transition(new TransformTransition(*new_mat_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_relative_point
//       Access: Published
//  Description: Given that the indicated point is in the coordinate
//               system of the other node, returns the same point in
//               this node's coordinate system.
////////////////////////////////////////////////////////////////////
LPoint3f NodePath::
get_relative_point(const NodePath &other, const LVecBase3f &point) {
  LPoint3f rel_point = LPoint3f(point) * other.get_mat(*this);
  return rel_point;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Published
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space, which
//               is relative to the other NodePath.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
void NodePath::
look_at(const NodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

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
//       Access: Published
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up(const NodePath &other, const LPoint3f &point, const LVector3f &up) {
  nassertv(has_arcs());

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
//     Function: NodePath::look_at_preserve_scale
//       Access: Published
//  Description: Functions like look_at(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, look_at() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
look_at_preserve_scale(const NodePath &other, const LPoint3f &point,
                       const LVector3f &up) {
  nassertv(has_arcs());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.
  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);

  NodePath parent(*this);
  parent.shorten(1);
  LPoint3f rel_point = point * other.get_mat(parent);

  ::look_at(mat, rel_point - pos, up);

  // Now reapply the scale and position.

  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up_preserve_scale
//       Access: Published
//  Description: Functions like heads_up(), but preforms additional
//               work to preserve any scales that may already be
//               present on the node.  Normally, heads_up() blows away
//               the scale because scale and rotation are represented
//               in the same part of the matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
heads_up_preserve_scale(const NodePath &other, const LPoint3f &point,
                        const LVector3f &up) {
  nassertv(has_arcs());

  LMatrix4f mat = get_mat();

  // Extract the axes from the matrix.
  LVector3f x, y, z;
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  // The lengths of the axes defines the scale.
  float scale_0 = length(x);
  float scale_1 = length(y);
  float scale_2 = length(z);

  LPoint3f pos;
  mat.get_row3(pos,3);

  NodePath parent(*this);
  parent.shorten(1);
  LPoint3f rel_point = point * other.get_mat(parent);

  ::heads_up(mat, rel_point - pos, up);

  // Now reapply the scale and position.
  mat.get_row3(x,0);
  mat.get_row3(y,1);
  mat.get_row3(z,2);

  x *= scale_0;
  y *= scale_1;
  z *= scale_2;

  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);
  mat.set_row(3, pos);
  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color
//       Access: Published
//  Description: Sets the color transition for a render relation
////////////////////////////////////////////////////////////////////
void NodePath::
set_color(const Colorf &color, int priority) {
  nassertv_always(has_arcs());

  ColorTransition *col_trans = new ColorTransition(color);
  arc()->set_transition(col_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the geometry color.  This is normally the
//               default, but it may be useful to use this to
//               contradict set_color() at a higher node level (or,
//               with a priority, to override a set_color() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void NodePath::
set_color_off(int priority) {
  nassertv_always(has_arcs());

  ColorTransition *col_trans =
    new ColorTransition(ColorTransition::off());
  arc()->set_transition(col_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_color
//       Access: Published
//  Description: Returns the color that has been assigned to the arc,
//               or black if no color has been assigned.
////////////////////////////////////////////////////////////////////
Colorf NodePath::
get_color() const {
  nassertr(has_arcs(), false);

  const ColorTransition *ct;
  if (get_transition_into(ct, arc())) {
    if (ct->is_on() && ct->is_real()) {
      return ct->get_color();
    }
  }

  sgmanip_cat.warning()
    << "get_color() called on " << *this << " which has no color set.\n";

  return Colorf(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_bin
//       Access: Published
//  Description: Assigns the geometry at this level and below to the
//               named rendering bin.  It is the user's responsibility
//               to ensure that such a bin already exists, either via
//               the cull-bin Configrc variable, or by explicitly
//               creating a GeomBin of the appropriate type at
//               runtime.
//
//               There are two default bins created when Panda is
//               started: "default" and "fixed".  Normally, all
//               geometry is assigned to "default" unless specified
//               otherwise.  This bin renders opaque geometry in
//               state-sorted order, followed by transparent geometry
//               sorted back-to-front.  If any geometry is assigned to
//               "fixed", this will be rendered following all the
//               geometry in "default", in the order specified by
//               draw_order for each piece of geometry so assigned.
//
//               The draw_order parameter is meaningful only for
//               GeomBinFixed type bins, e.g. "fixed".  Other kinds of
//               bins ignore it.
////////////////////////////////////////////////////////////////////
void NodePath::
set_bin(const string &bin_name, int draw_order, int priority) {
  nassertv_always(has_arcs());

  GeomBinTransition *bin_trans = new GeomBinTransition(bin_name, draw_order);
  arc()->set_transition(bin_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_bin_name
//       Access: Published
//  Description: Returns the name of the bin that this particular arc
//               was assigned to via set_bin(), or the empty string if
//               no bin was assigned.  See set_bin() and has_bin().
////////////////////////////////////////////////////////////////////
string NodePath::
get_bin_name() const {
  nassertr(has_arcs(), string());

  const GeomBinTransition *bt;
  if (get_transition_into(bt, arc())) {
    if (bt->is_on()) {
      return bt->get_bin();
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_bin_draw_order
//       Access: Published
//  Description: Returns the drawing order associated with the bin
//               that this particular arc was assigned to via
//               set_bin(), or 0 if no bin was assigned.  See
//               set_bin() and has_bin().
////////////////////////////////////////////////////////////////////
int NodePath::
get_bin_draw_order() const {
  nassertr(has_arcs(), 0);

  const GeomBinTransition *bt;
  if (get_transition_into(bt, arc())) {
    if (bt->is_on()) {
      return bt->get_draw_order();
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_texture
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated texture.
////////////////////////////////////////////////////////////////////
void NodePath::
set_texture(Texture *tex, int priority) {
  nassertv_always(has_arcs());
  nassertv(tex != NULL);

  TextureTransition *tex_trans = new TextureTransition(tex);
  arc()->set_transition(tex_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_texture_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using no texture.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_texture() at a higher node level (or, with a
//               priority, to override a set_texture() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void NodePath::
set_texture_off(int priority) {
  nassertv_always(has_arcs());

  TextureTransition *tex_trans =
    new TextureTransition(TextureTransition::off());
  arc()->set_transition(tex_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_texture
//       Access: Published
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

  const TextureTransition *tt;
  if (get_transition_into(tt, arc())) {
    return tt->is_on();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_texture_off
//       Access: Published
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

  const TextureTransition *tt;
  if (get_transition_into(tt, arc())) {
    return tt->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_texture
//       Access: Published
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

  const TextureTransition *tt;
  if (get_transition_into(tt, arc())) {
    if (tt->is_on()) {
      return tt->get_texture();
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_material
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated material.
//
//               This operation copies the given material pointer.  If
//               the material structure is changed later, it must be
//               reapplied via another call to set_material().
////////////////////////////////////////////////////////////////////
void NodePath::
set_material(Material *mat, int priority) {
  nassertv_always(has_arcs());
  nassertv(mat != NULL);

  // We create a temporary Material pointer, a copy of the one we are
  // given, to allow the user to monkey with the material and set it
  // again later, with the desired effect.  If we stored the user's
  // pointer directly, it would be bad if the user later modified the
  // values within the Material.
  PT(Material) temp = new Material(*mat);
  const Material *mp = MaterialPool::get_material(temp);

  MaterialTransition *mat_trans = new MaterialTransition(mp);
  arc()->set_transition(mat_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_material
//       Access: Published
//  Description: Returns true if a material has been applied to this
//               particular arc via set_material(), false otherwise.
////////////////////////////////////////////////////////////////////
bool NodePath::
has_material() const {
  nassertr(has_arcs(), false);

  const MaterialTransition *mt;
  if (get_transition_into(mt, arc())) {
    return mt->is_on();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_material
//       Access: Published
//  Description: Returns the material that has been set on this
//               particular arc, or NULL if no material has been set.
//               This is not necessarily the material that will be
//               applied to the geometry at or below this level, as
//               another material at a higher or lower level may
//               override.
//
//               This function returns a copy of the given material,
//               to allow changes, if desired.  Once changes are made,
//               they should be reapplied via set_material().
////////////////////////////////////////////////////////////////////
PT(Material) NodePath::
get_material() const {
  nassertr(has_arcs(), NULL);

  const MaterialTransition *mt;
  if (get_transition_into(mt, arc())) {
    if (mt->is_on()) {
      return new Material(*mt->get_material());
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fog
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using the indicated fog.
////////////////////////////////////////////////////////////////////
void NodePath::
set_fog(Fog *fog, int priority) {
  nassertv_always(has_arcs());
  nassertv(fog != NULL);

  FogTransition *fog_trans = new FogTransition(fog);
  arc()->set_transition(fog_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_fog_off
//       Access: Published
//  Description: Sets the geometry at this level and below to render
//               using no fog.  This is normally the default, but
//               it may be useful to use this to contradict
//               set_fog() at a higher node level (or, with a
//               priority, to override a set_fog() at a lower
//               level).
////////////////////////////////////////////////////////////////////
void NodePath::
set_fog_off(int priority) {
  nassertv_always(has_arcs());

  FogTransition *fog_trans =
    new FogTransition(FogTransition::off());
  arc()->set_transition(fog_trans, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_fog
//       Access: Published
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

  const FogTransition *tt;
  if (get_transition_into(tt, arc())) {
    return tt->is_on();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_fog_off
//       Access: Published
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

  const FogTransition *tt;
  if (get_transition_into(tt, arc())) {
    return tt->is_off();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_fog
//       Access: Published
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

  const FogTransition *tt;
  if (get_transition_into(tt, arc())) {
    if (tt->is_on()) {
      return tt->get_fog();
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_render_mode_wireframe
//       Access: Published
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in wireframe mode.
////////////////////////////////////////////////////////////////////
void NodePath::
set_render_mode_wireframe(int priority) {
  nassertv_always(has_arcs());

  arc()->set_transition(new RenderModeTransition(RenderModeProperty::M_wireframe), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_render_mode_filled
//       Access: Published
//  Description: Sets up the geometry at this level and below (unless
//               overridden) to render in filled (i.e. not wireframe)
//               mode.
////////////////////////////////////////////////////////////////////
void NodePath::
set_render_mode_filled(int priority) {
  nassertv_always(has_arcs());

  RenderModeTransition *rmt =
    new RenderModeTransition(RenderModeProperty::M_filled);
  arc()->set_transition(rmt, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_two_sided
//       Access: Published
//  Description: Specifically sets or disables two-sided rendering
//               mode on this particular arc.  If no other arcs
//               override, this will cause backfacing polygons to be
//               drawn (in two-sided mode, true) or culled (in
//               one-sided mode, false).
////////////////////////////////////////////////////////////////////
void NodePath::
set_two_sided(bool two_sided, int priority) {
  nassertv_always(has_arcs());

  CullFaceProperty::Mode mode =
    two_sided ?
    CullFaceProperty::M_cull_none :
    CullFaceProperty::M_cull_clockwise;

  CullFaceTransition *cft = new CullFaceTransition(mode);
  arc()->set_transition(cft, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_two_sided
//       Access: Published
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

  const CullFaceTransition *cft;
  if (get_transition_into(cft, arc())) {
    return (cft->get_mode() == CullFaceProperty::M_cull_none);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::do_billboard_axis
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to heads_up().
//               However, it does lose both translate and scale
//               components of the matrix.
////////////////////////////////////////////////////////////////////
void NodePath::
do_billboard_axis(const NodePath &camera, float offset) {
  nassertv(has_arcs());

  NodePath parent(*this);
  parent.shorten(1);
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up();
  LVector3f rel_pos = -rel_mat.get_row3(3);

  LMatrix4f mat;
  ::heads_up(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::do_billboard_point_eye
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to look_at(),
//               although the point_eye billboard effect cannot be
//               achieved using the ordinary look_at() call.
////////////////////////////////////////////////////////////////////
void NodePath::
do_billboard_point_eye(const NodePath &camera, float offset) {
  nassertv(has_arcs());

  NodePath parent(*this);
  parent.shorten(1);
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up() * rel_mat;
  LVector3f rel_pos = LVector3f::forward() * rel_mat;

  LMatrix4f mat;
  ::look_at(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::do_billboard_point_world
//       Access: Published
//  Description: Performs a billboard-type rotate to the indicated
//               camera node, one time only, and leaves the object
//               rotated.  This is similar in principle to look_at().
////////////////////////////////////////////////////////////////////
void NodePath::
do_billboard_point_world(const NodePath &camera, float offset) {
  nassertv(has_arcs());

  NodePath parent(*this);
  parent.shorten(1);
  LMatrix4f rel_mat = camera.get_mat(parent);

  LVector3f up = LVector3f::up();
  LVector3f rel_pos = -rel_mat.get_row3(3);

  LMatrix4f mat;
  ::look_at(mat, rel_pos, up);

  // Also slide the geometry towards the camera according to the
  // offset factor.
  if (offset != 0.0f) {
    LVector3f translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    mat.set_row(3, translate);
  }

  set_mat(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_transparency
//       Access: Published
//  Description: Specifically sets or disables transparent rendering
//               mode on this particular arc.  If no other arcs
//               override, this will cause items with a non-1 value
//               for alpha color to be rendered partially transparent.
////////////////////////////////////////////////////////////////////
void NodePath::
set_transparency(bool transparency, int priority) {
  nassertv_always(has_arcs());

  TransparencyProperty::Mode mode =
    transparency ?
    TransparencyProperty::M_alpha :
    TransparencyProperty::M_none;

  TransparencyTransition *tt = new TransparencyTransition(mode);
  arc()->set_transition(tt, priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_transparency
//       Access: Published
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

  const TransparencyTransition *tt;
  if (get_transition_into(tt, arc())) {
    return (tt->get_mode() != TransparencyProperty::M_none);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hidden_ancestor
//       Access: Published
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
    return NodePath::not_found();
  }

  if (arc()->has_transition(PruneTransition::get_class_type())) {
    return *this;
  }

  NodePath next(*this);
  next.shorten();
  return next.get_hidden_ancestor();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_stashed_ancestor
//       Access: Published
//  Description: Returns a NodePath indicating the lowest arc above
//               this node which has been set to 'stash'.  Calling
//               unstash() on the NodePath returned by this function
//               should make the node visible again (unless there is
//               another arc further up that also has been 'stashed'.
//
//               This function returns an empty NodePath if no
//               ancestors have been 'stashed'.
////////////////////////////////////////////////////////////////////
NodePath NodePath::
get_stashed_ancestor() const {
  if (!has_arcs()) {
    return NodePath::not_found();
  }

  if (arc()->get_graph_type() != _graph_type) {
    return *this;
  }

  NodePath next(*this);
  next.shorten();
  return next.get_stashed_ancestor();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::prepare_scene
//       Access: Published
//  Description: Walks through the scene graph beginning at the bottom
//               node, and does whatever initialization is required to
//               render the scene properly with the indicated GSG.  It
//               is not strictly necessary to call this, since the GSG
//               will initialize itself when the scene is rendered,
//               but this may take some of the overhead away from that
//               process.
//
//               If force_retained_mode is true, retained mode is set
//               on the geometry encountered, regardless of the
//               setting of the retained-mode Config variable.
//               Otherwise, retained mode is set only if the
//               retained-mode Config variable is true.
////////////////////////////////////////////////////////////////////
void NodePath::
prepare_scene(GraphicsStateGuardianBase *gsg, bool force_retained_mode) {
  nassertv_always(!is_empty());

  // Use the ScenePrepareVisitor and fire off a traversal of the scene
  // beginning at the bottom node.  The ScenePrepareVisitor (defined
  // above) will call prepare() on each texture it finds in the scene
  // graph at this point and below.
  ScenePrepareVisitor visitor;
  visitor._gsg = gsg;
  visitor._retained_mode = retained_mode || force_retained_mode;

  NodeAttributeWrapper initial(TextureTransition::get_class_type());
  df_traverse(node(), visitor, initial, NullLevelState(),
              RenderRelation::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show_bounds
//       Access: Published
//  Description: Causes the bounding volume of the bottom node and all
//               of its descendants (that is, the bounding volume
//               associated with the the bottom arc) to be rendered,
//               if possible.  The rendering method is less than
//               optimal; this is intended primarily for debugging.
////////////////////////////////////////////////////////////////////
void NodePath::
show_bounds() {
  nassertv_always(has_arcs());
  arc()->set_transition(new DrawBoundsTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide_bounds
//       Access: Published
//  Description: Stops the rendering of the bounding volume begun with
//               show_bounds().
////////////////////////////////////////////////////////////////////
void NodePath::
hide_bounds() {
  nassertv_always(has_arcs());
  arc()->clear_transition(DrawBoundsTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_bounds
//       Access: Published
//  Description: Returns a newly-allocated bounding volume containing
//               the bottom node and all of its descendants.  This is
//               the bounding volume on the bottom arc, converted to
//               the local coordinate space of the node.
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) NodePath::
get_bounds() const {
  nassertr(has_arcs(), new BoundingSphere);

  NodeRelation *darc = arc();
  PT(BoundingVolume) bv = darc->get_bound().make_copy();
  if (bv->is_of_type(GeometricBoundingVolume::get_class_type()) &&
      darc->has_transition(TransformTransition::get_class_type())) {

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
//       Access: Published
//  Description: Writes a description of the bounding volume
//               containing the bottom node and all of its descendants
//               to the indicated output stream.
////////////////////////////////////////////////////////////////////
void NodePath::
write_bounds(ostream &out) const {
  get_bounds()->write(out);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::calc_tight_bounds
//       Access: Published
//  Description: Calculates the minimum and maximum vertices of all
//               Geoms at this NodePath's bottom node and below.  This
//               is a tight bounding box; it will generally be tighter
//               than the bounding volume returned by get_bounds()
//               (but it is more expensive to compute).
//
//               The return value is true if any points are within the
//               bounding volume, or false if none are.
////////////////////////////////////////////////////////////////////
bool NodePath::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point) {
  min_point.set(0.0, 0.0, 0.0);
  max_point.set(0.0, 0.0, 0.0);
  nassertr_always(!is_empty(), false);

  bool found_any = false;
  r_calc_tight_bounds(node(), min_point, max_point, found_any, LMatrix4f::ident_mat());

  return found_any;
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
  if (other->is_top_node()) {
    return (other->get_node() == node());
  }

  if (!r_extend_by(other->get_next())) {
    return false;
  }

  if (!extend_by(other->get_arc())) {
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

  if (comp->get_next() == (const ArcComponent *)NULL) {
    // Here's the end of the chain.
    result = format_node_name(comp->get_node());

    return 1;

  } else {
    int nodes_before = r_as_string(comp->get_next(), result, skip_nodes);
    if (skip_nodes <= nodes_before) {
      if (skip_nodes != nodes_before) {
        // This is not the first node, so format a slash between the
        // previous node and this node.

        NodeRelation *darc = comp->get_arc();
        if (darc->get_child() == (Node *)NULL &&
            darc->get_parent() == (Node *)NULL) {
          // Here's a deleted arc.  Indicate this.
          result += "/(...deleted...)";
        } else {
          if (darc->get_parent() != comp->get_next()->get_node() &&
              comp->get_next()->get_node() != (Node *)NULL) {
            // It's not a deleted arc, but something's broken.
            result += "/.../" + format_node_name(darc->get_parent());
          }
          result += format_arc_name(darc);
          result += format_node_name(darc->get_child());
        }
      }
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
  nassertv(comp->has_arc());

  if (!comp->is_top_arc()) {
    r_write_transitions(comp->get_next(), out, indent_level);
  }

  NodeRelation *darc = comp->get_arc();
  indent(out, indent_level) << *darc << "\n";
  darc->write_transitions(out, indent_level + 2);
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
  nassertv(comp->has_arc());

  if (!comp->is_top_arc()) {
    r_get_net_transitions(comp->get_next(), trans);
  }

  AllTransitionsWrapper arc_trans;
  arc_trans.extract_from(comp->get_arc());
  trans.compose_in_place(arc_trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::format_node_name
//       Access: Private
//  Description: Formats one component of the as_string string: a
//               node, either unnamed as a type, or as a name.
////////////////////////////////////////////////////////////////////
string NodePath::
format_node_name(Node *dnode) const {
  if (dnode == (Node *)NULL) {
    return "(NULL)";
  }
  string name;
  if (dnode->is_of_type(NamedNode::get_class_type())) {
    name = DCAST(NamedNode, dnode)->get_name();
  }

  if (name.empty()) {
    // No name.  If the type isn't one of the trivial types (Node or
    // NamedNode), use the type name instead, since it's likely to be
    // more unique.
    if (!dnode->is_of_type(Node::get_class_type()) &&
        !dnode->is_of_type(NamedNode::get_class_type())) {
      return "-" + dnode->get_type().get_name();
    }
  }

  // Use the node's name.
  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::format_arc_name
//       Access: Private
//  Description: Formats the "name" for the arc in as_string().
//               Normally, this is simply "/", but for certain arcs
//               (as for stashed nodes, for instance), it might
//               contain other characters.
////////////////////////////////////////////////////////////////////
string NodePath::
format_arc_name(NodeRelation *arc) const {
  string result = "/";

  if (arc->get_graph_type() == NodeRelation::get_stashed_type()) {
    result += "@@";

  } else if (arc->get_graph_type() != _graph_type) {
    result += "@@(" + arc->get_graph_type().get_name() + ")";
  }

  return result;
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
    } else {
      entry.consider_node(result, next_level, max_matches, _graph_type);
    }

    if (max_matches > 0 && result.get_num_paths() >= max_matches) {
      return;
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
  Node *bottom_node = node();
  nassertv(bottom_node != (Node *)NULL);
  indent(out, indent_level) << *bottom_node << "\n";

  int num_children = bottom_node->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = bottom_node->get_child(_graph_type, i);

    NodePath next(*this);
    next.extend_by(child_arc);
    next.r_list_descendants(out, indent_level + 2);
  }

  if (_graph_type != NodeRelation::get_stashed_type()) {
    // Also report the number of stashed nodes at this level.
    int num_stashed = bottom_node->get_num_children(NodeRelation::get_stashed_type());
    if (num_stashed != 0) {
      indent(out, indent_level) << "(" << num_stashed << " stashed)\n";
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
  Node *bottom_node = node();
  nassertv(bottom_node != (Node *)NULL);

  out << "\n+";
  indent(out, indent_level + 1) << *bottom_node << "\n\n";

  int num_children = bottom_node->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = bottom_node->get_child(_graph_type, i);
    NodePath next(*this);
    next.extend_by(child_arc);

    indent(out, indent_level + 2) << *child_arc << ":\n";
    child_arc->write_transitions(out, indent_level + 2);
    next.r_list_transitions(out, indent_level + 2);
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

  Node *dnode = arc->get_child();

  int num_children = dnode->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = dnode->get_child(_graph_type, i);
    r_adjust_all_priorities(child_arc, adjustment);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_clear_wrt_cache
//       Access: Private
//  Description: The recursive implementation of
//               clear_wrt_cache().  This walks through the
//               subgraph defined by the indicated arc and below.
////////////////////////////////////////////////////////////////////
void NodePath::
r_clear_wrt_cache(NodeRelation *arc) {
  arc->clear_wrt_cache();

  Node *dnode = arc->get_child();

  int num_children = dnode->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = dnode->get_child(_graph_type, i);
    r_clear_wrt_cache(child_arc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::r_calc_tight_bounds
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void NodePath::
r_calc_tight_bounds(Node *dnode, LPoint3f &min_point, LPoint3f &max_point,
                    bool &found_any, const LMatrix4f &transform) {
  if (dnode->is_of_type(GeomNode::get_class_type())) {
    GeomNode *gnode = DCAST(GeomNode, dnode);
    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      dDrawable *drawable = gnode->get_geom(i);
      if (drawable->is_of_type(Geom::get_class_type())) {
        Geom *geom = DCAST(Geom, drawable);

        Geom::VertexIterator vi = geom->make_vertex_iterator();
        int num_prims = geom->get_num_prims();

        for (int p = 0; p < num_prims; p++) {
          int length = geom->get_length(p);
          for (int v = 0; v < length; v++) {
            Vertexf vertex = geom->get_next_vertex(vi) * transform;

            if (found_any) {
              min_point.set(min(min_point[0], vertex[0]),
                            min(min_point[1], vertex[1]),
                            min(min_point[2], vertex[2]));
              max_point.set(max(max_point[0], vertex[0]),
                            max(max_point[1], vertex[1]),
                            max(max_point[2], vertex[2]));
            } else {
              min_point = vertex;
              max_point = vertex;
              found_any = true;
            }
          }
        }
      }
    }
  }

  // Now consider children.

  int num_children = dnode->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *child_arc = dnode->get_child(_graph_type, i);

    const TransformTransition *tt;
    if (get_transition_into(tt, child_arc)) {
      // This arc has a transform; apply it.
      r_calc_tight_bounds(child_arc->get_child(), min_point, max_point, 
                          found_any, tt->get_matrix() * transform);
    } else {
      // This arc doesn't have a transform.
      r_calc_tight_bounds(child_arc->get_child(), min_point, max_point, 
                          found_any, transform);
      
    }
  }
}
