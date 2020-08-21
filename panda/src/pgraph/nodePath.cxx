/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePath.cxx
 * @author drose
 * @date 2002-02-25

 */

#include "nodePath.h"
#include "nodePathCollection.h"
#include "findApproxPath.h"
#include "findApproxLevelEntry.h"
#include "internalNameCollection.h"
#include "config_pgraph.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "materialAttrib.h"
#include "materialCollection.h"
#include "lightAttrib.h"
#include "clipPlaneAttrib.h"
#include "occluderEffect.h"
#include "polylightEffect.h"
#include "fogAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "alphaTestAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "depthOffsetAttrib.h"
#include "shaderAttrib.h"
#include "billboardEffect.h"
#include "compassEffect.h"
#include "showBoundsEffect.h"
#include "transparencyAttrib.h"
#include "antialiasAttrib.h"
#include "audioVolumeAttrib.h"
#include "texProjectorEffect.h"
#include "scissorEffect.h"
#include "texturePool.h"
#include "planeNode.h"
#include "occluderNode.h"
#include "lensNode.h"
#include "materialPool.h"
#include "look_at.h"
#include "plist.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "sceneGraphReducer.h"
#include "textureCollection.h"
#include "textureStageCollection.h"
#include "globPattern.h"
#include "shader.h"
#include "shaderInput.h"
#include "config_gobj.h"
#include "bamFile.h"
#include "preparedGraphicsObjects.h"
#include "dcast.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "modelNode.h"
#include "bam.h"
#include "bamWriter.h"
#include "datagramBuffer.h"
#include "weakNodePath.h"

using std::max;
using std::move;
using std::ostream;
using std::ostringstream;
using std::string;

// stack seems to overflow on Intel C++ at 7000.  If we need more than 7000,
// need to increase stack size.
int NodePath::_max_search_depth = 7000;
TypeHandle NodePath::_type_handle;

PStatCollector NodePath::_get_transform_pcollector("*:NodePath:get_transform");
PStatCollector NodePath::_verify_complete_pcollector("*:NodePath:verify_complete");

/**
 * Constructs a NodePath with the indicated parent NodePath and child node;
 * the child node must be a stashed or unstashed child of the parent.
 */
NodePath::
NodePath(const NodePath &parent, PandaNode *child_node,
         Thread *current_thread) :
  _error_type(ET_fail)
{
  nassertv(child_node != nullptr);
  int pipeline_stage = current_thread->get_pipeline_stage();

  if (parent.is_empty()) {
    // Special case: constructing a NodePath at the root.
    _head = PandaNode::get_top_component(child_node, true,
                                         pipeline_stage, current_thread);

  } else {
    _head = PandaNode::get_component(parent._head, child_node, pipeline_stage,
                                     current_thread);
  }
  nassertv(_head != nullptr);

  if (_head != nullptr) {
    _error_type = ET_ok;
  }
  _backup_key = 0;
}

/**
 * Returns true if the NodePath is valid (not empty), or false if it contains
 * no nodes.
 */
NodePath::
operator bool () const {
  return !is_empty();
}

/**
 * Returns the number of nodes in the path.
 */
int NodePath::
get_num_nodes(Thread *current_thread) const {
  if (is_empty()) {
    return 0;
  }
  int pipeline_stage = current_thread->get_pipeline_stage();
  return _head->get_length(pipeline_stage, current_thread);
}

/**
 * Returns the nth node of the path, where 0 is the referenced (bottom) node
 * and get_num_nodes() - 1 is the top node.  This requires iterating through
 * the path.
 *
 * Also see node(), which is a convenience function to return the same thing
 * as get_node(0) (since the bottom node is the most important node in the
 * NodePath, and is the one most frequently referenced).
 *
 * Note that this function returns the same thing as
 * get_ancestor(index).node().
 */
PandaNode *NodePath::
get_node(int index, Thread *current_thread) const {
  nassertr(index >= 0 && index < get_num_nodes(), nullptr);

  int pipeline_stage = current_thread->get_pipeline_stage();

  NodePathComponent *comp = _head;
  while (index > 0) {
    // If this assertion fails, the index was out of range; the component's
    // length must have been invalid.
    nassertr(comp != nullptr, nullptr);
    comp = comp->get_next(pipeline_stage, current_thread);
    index--;
  }

  // If this assertion fails, the index was out of range; the component's
  // length must have been invalid.
  nassertr(comp != nullptr, nullptr);
  return comp->get_node();
}

/**
 * Returns the nth ancestor of the path, where 0 is the NodePath itself and
 * get_num_nodes() - 1 is get_top(). This requires iterating through the path.
 *
 * Also see get_node(), which returns the same thing as a PandaNode pointer,
 * not a NodePath.
 */
NodePath NodePath::
get_ancestor(int index, Thread *current_thread) const {
  nassertr(index >= 0 && index < get_num_nodes(), NodePath::fail());

  int pipeline_stage = current_thread->get_pipeline_stage();

  NodePathComponent *comp = _head;
  while (index > 0) {
    // If this assertion fails, the index was out of range; the component's
    // length must have been invalid.
    nassertr(comp != nullptr, NodePath::fail());
    comp = comp->get_next(pipeline_stage, current_thread);
    index--;
  }

  // If this assertion fails, the index was out of range; the component's
  // length must have been invalid.
  nassertr(comp != nullptr, NodePath::fail());

  NodePath result;
  result._head = comp;
  return result;
}

/**
 * Returns a singleton NodePath that represents the top of the path, or empty
 * NodePath if this path is empty.
 */
NodePath NodePath::
get_top(Thread *current_thread) const {
  if (is_empty()) {
    return *this;
  }

  int pipeline_stage = current_thread->get_pipeline_stage();

  NodePathComponent *comp = _head;
  while (!comp->is_top_node(pipeline_stage, current_thread)) {
    comp = comp->get_next(pipeline_stage, current_thread);
    nassertr(comp != nullptr, NodePath::fail());
  }

  NodePath top;
  top._head = comp;
  return top;
}


/**
 * Returns the set of all child nodes of the referenced node.
 */
NodePathCollection NodePath::
get_children(Thread *current_thread) const {
  NodePathCollection result;
  nassertr_always(!is_empty(), result);

  PandaNode *bottom_node = node();

  int pipeline_stage = current_thread->get_pipeline_stage();

  PandaNode::Children cr = bottom_node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    NodePath child;
    child._head = PandaNode::get_component(_head, cr.get_child(i),
                                           pipeline_stage, current_thread);
    result.add_path(child);
  }

  return result;
}

/**
 * Returns the set of all child nodes of the referenced node that have been
 * stashed.  These children are not normally visible on the node, and do not
 * appear in the list returned by get_children().
 */
NodePathCollection NodePath::
get_stashed_children(Thread *current_thread) const {
  NodePathCollection result;
  nassertr_always(!is_empty(), result);

  PandaNode *bottom_node = node();

  int pipeline_stage = current_thread->get_pipeline_stage();

  int num_stashed = bottom_node->get_num_stashed();
  for (int i = 0; i < num_stashed; i++) {
    NodePath stashed;
    stashed._head = PandaNode::get_component(_head, bottom_node->get_stashed(i),
                                             pipeline_stage, current_thread);
    result.add_path(stashed);
  }

  return result;
}

/**
 * Returns the sort value of the referenced node within its parent; that is,
 * the sort number passed on the last reparenting operation for this node.
 * This will control the position of the node within its parent's list of
 * children.
 */
int NodePath::
get_sort(Thread *current_thread) const {
  if (!has_parent()) {
    return 0;
  }

  int pipeline_stage = current_thread->get_pipeline_stage();

  PandaNode *parent = _head->get_next(pipeline_stage, current_thread)->get_node();
  PandaNode *child = node();
  nassertr(parent != nullptr && child != nullptr, 0);
  int child_index = parent->find_child(child);
  if (child_index != -1) {
    return parent->get_child_sort(child_index);
  }

  child_index = parent->find_stashed(child);
  if (child_index != -1) {
    return parent->get_stashed_sort(child_index);
  }

  nassertr(false, 0);
  return 0;
}

/**
 * Searches for a node below the referenced node that matches the indicated
 * string.  Returns the shortest match found, if any, or an empty NodePath if
 * no match can be found.
 */
NodePath NodePath::
find(const string &path) const {
  nassertr_always(!is_empty(), fail());

  NodePathCollection col;
  find_matches(col, path, 1);

  if (col.is_empty()) {
    return NodePath::not_found();
  }

  return col.get_path(0);
}

/**
 * Searches for the indicated node below this node and returns the shortest
 * NodePath that connects them.
 */
NodePath NodePath::
find_path_to(PandaNode *node) const {
  nassertr_always(!is_empty(), fail());
  nassertr(node != nullptr, fail());

  NodePathCollection col;
  FindApproxPath approx_path;
  approx_path.add_match_many(0);
  approx_path.add_match_pointer(node, 0);
  find_matches(col, approx_path, 1);

  if (col.is_empty()) {
    return NodePath::not_found();
  }

  return col.get_path(0);
}

/**
 * Returns the complete set of all NodePaths that begin with this NodePath and
 * can be extended by path.  The shortest paths will be listed first.
 */
NodePathCollection NodePath::
find_all_matches(const string &path) const {
  NodePathCollection col;
  nassertr_always(!is_empty(), col);
  nassertr(verify_complete(), col);
  find_matches(col, path, -1);
  return col;
}

/**
 * Returns the set of all NodePaths that extend from this NodePath down to the
 * indicated node.  The shortest paths will be listed first.
 */
NodePathCollection NodePath::
find_all_paths_to(PandaNode *node) const {
  NodePathCollection col;
  nassertr_always(!is_empty(), col);
  nassertr(verify_complete(), col);
  nassertr(node != nullptr, col);
  FindApproxPath approx_path;
  approx_path.add_match_many(0);
  approx_path.add_match_pointer(node, 0);
  find_matches(col, approx_path, -1);
  return col;
}

/**
 * Removes the referenced node of the NodePath from its current parent and
 * attaches it to the referenced node of the indicated NodePath.
 *
 * If the destination NodePath is empty, this is the same thing as
 * detach_node().
 *
 * If the referenced node is already a child of the indicated NodePath (via
 * some other instance), this operation fails and leaves the NodePath
 * detached.
 */
void NodePath::
reparent_to(const NodePath &other, int sort, Thread *current_thread) {
  nassertv(verify_complete());
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv(other._error_type == ET_ok);

  // Reparenting implicitly resets the delta vector.
  node()->reset_prev_transform();

  int pipeline_stage = current_thread->get_pipeline_stage();
  bool reparented = PandaNode::reparent(other._head, _head, sort, false,
                                        pipeline_stage, current_thread);
  nassertv(reparented);
}

/**
 * Similar to reparent_to(), but the node is added to its new parent's stashed
 * list, so that the result is equivalent to calling reparent_to() immediately
 * followed by stash().
 */
void NodePath::
stash_to(const NodePath &other, int sort, Thread *current_thread) {
  nassertv(verify_complete());
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv(other._error_type == ET_ok);

  // Reparenting implicitly resets the delta vector.
  node()->reset_prev_transform();

  int pipeline_stage = current_thread->get_pipeline_stage();
  bool reparented = PandaNode::reparent(other._head, _head, sort, true,
                                        pipeline_stage, current_thread);
  nassertv(reparented);
}

/**
 * This functions identically to reparent_to(), except the transform on this
 * node is also adjusted so that the node remains in the same place in world
 * coordinates, even if it is reparented into a different coordinate system.
 */
void NodePath::
wrt_reparent_to(const NodePath &other, int sort, Thread *current_thread) {
  nassertv(verify_complete(current_thread));
  nassertv(other.verify_complete(current_thread));
  nassertv_always(!is_empty());
  nassertv(other._error_type == ET_ok);

  if (get_transform(current_thread) == get_prev_transform(current_thread)) {
    set_transform(get_transform(other, current_thread), current_thread);
    node()->reset_prev_transform(current_thread);
  } else {
    set_transform(get_transform(other, current_thread), current_thread);
    set_prev_transform(get_prev_transform(other, current_thread), current_thread);
  }

  reparent_to(other, sort, current_thread);
}

/**
 * Adds the referenced node of the NodePath as a child of the referenced node
 * of the indicated other NodePath.  Any other parent-child relations of the
 * node are unchanged; in particular, the node is not removed from its
 * existing parent, if any.
 *
 * If the node already had an existing parent, this method will create a new
 * instance of the node within the scene graph.
 *
 * This does not change the NodePath itself, but does return a new NodePath
 * that reflects the new instance node.
 *
 * If the destination NodePath is empty, this creates a new instance which is
 * not yet parented to any node.  A new instance of this sort cannot easily be
 * differentiated from other similar instances, but it is nevertheless a
 * different instance and it will return a different get_id() value.
 *
 * If the referenced node is already a child of the indicated NodePath,
 * returns that already-existing instance, unstashing it first if necessary.
 */
NodePath NodePath::
instance_to(const NodePath &other, int sort, Thread *current_thread) const {
  nassertr(verify_complete(), NodePath::fail());
  nassertr(other.verify_complete(), NodePath::fail());
  nassertr_always(!is_empty(), NodePath::fail());
  nassertr(other._error_type == ET_ok, NodePath::fail());

  NodePath new_instance;

  // First, we'll attach to NULL, to guarantee we get a brand new instance.
  int pipeline_stage = current_thread->get_pipeline_stage();
  new_instance._head = PandaNode::attach(nullptr, node(), sort, pipeline_stage,
                                         current_thread);

  // Now, we'll reparent the new instance to the target node.
  bool reparented = PandaNode::reparent(other._head, new_instance._head,
                                        sort, false, pipeline_stage,
                                        current_thread);
  if (!reparented) {
    // Hmm, couldn't reparent.  Either making this instance would create a
    // cycle, or it was already a child of that node.  If it was already a
    // child, return that existing NodePath instead.
    NodePath orig(other, node(), current_thread);
    if (!orig.is_empty()) {
      if (orig.is_stashed()) {
        orig.unstash();
      }
      return orig;
    }

    // Nope, it must be a cycle.
    nassertr(reparented, new_instance);
  }

  // instance_to() doesn't reset the velocity delta, unlike most of the other
  // reparenting operations.  The reasoning is that instance_to() is not
  // necessarily a reparenting operation, since it doesn't change the original
  // instance.

  return new_instance;
}

/**
 * Behaves like instance_to(), but implicitly creates a new node to instance
 * the geometry under, and returns a NodePath to that new node.  This allows
 * the programmer to set a unique state and/or transform on this instance.
 */
NodePath NodePath::
instance_under_node(const NodePath &other, const string &name, int sort,
                    Thread *current_thread) const {
  NodePath new_node = other.attach_new_node(name, sort, current_thread);
  NodePath instance = instance_to(new_node, 0, current_thread);
  if (instance.is_empty()) {
    new_node.remove_node(current_thread);
    return instance;
  }
  return new_node;
}

/**
 * Functions like instance_to(), except a deep copy is made of the referenced
 * node and all of its descendents, which is then parented to the indicated
 * node.  A NodePath to the newly created copy is returned.
 */
NodePath NodePath::
copy_to(const NodePath &other, int sort, Thread *current_thread) const {
  nassertr(verify_complete(current_thread), fail());
  nassertr(other.verify_complete(current_thread), fail());
  nassertr_always(!is_empty(), fail());
  nassertr(other._error_type == ET_ok, fail());

  PandaNode *source_node = node();
  PT(PandaNode) copy_node = source_node->copy_subgraph(current_thread);
  nassertr(copy_node != nullptr, fail());

  copy_node->reset_prev_transform(current_thread);

  return other.attach_new_node(copy_node, sort, current_thread);
}

/**
 * Attaches a new node, with or without existing parents, to the scene graph
 * below the referenced node of this NodePath.  This is the preferred way to
 * add nodes to the graph.
 *
 * If the node was already a child of the parent, this returns a NodePath to
 * the existing child.
 *
 * This does *not* automatically extend the current NodePath to reflect the
 * attachment; however, a NodePath that does reflect this extension is
 * returned.
 */
NodePath NodePath::
attach_new_node(PandaNode *node, int sort, Thread *current_thread) const {
  nassertr(verify_complete(current_thread), NodePath::fail());
  nassertr(_error_type == ET_ok, NodePath::fail());
  nassertr(node != nullptr, NodePath::fail());

  NodePath new_path(*this);
  int pipeline_stage = current_thread->get_pipeline_stage();
  new_path._head = PandaNode::attach(_head, node, sort, pipeline_stage,
                                     current_thread);
  return new_path;
}

/**
 * Disconnects the referenced node from the scene graph.  This will also
 * delete the node if there are no other pointers to it.
 *
 * Normally, this should be called only when you are really done with the
 * node.  If you want to remove a node from the scene graph but keep it around
 * for later, you should probably use detach_node() instead.
 *
 * In practice, the only difference between remove_node() and detach_node() is
 * that remove_node() also resets the NodePath to empty, which will cause the
 * node to be deleted immediately if there are no other references.  On the
 * other hand, detach_node() leaves the NodePath referencing the node, which
 * will keep at least one reference to the node for as long as the NodePath
 * exists.
 */
void NodePath::
remove_node(Thread *current_thread) {
  nassertv(_error_type != ET_not_found);

  // If we have no parents, remove_node() is just a do-nothing operation; if
  // we have no nodes, maybe we were already removed.  In either case, quietly
  // do nothing except to ensure the NodePath is clear.
  if (!is_empty() && !is_singleton(current_thread)) {
    node()->reset_prev_transform(current_thread);
    int pipeline_stage = current_thread->get_pipeline_stage();
    PandaNode::detach(_head, pipeline_stage, current_thread);
  }

  if (is_empty() || _head->has_key()) {
    // Preserve the key we had on the node before we removed it.
    int key = get_key();
    (*this) = NodePath::removed();
    _backup_key = key;

  } else {
    // We didn't have a key; just clear the NodePath.
    (*this) = NodePath::removed();
  }
}

/**
 * Disconnects the referenced node from its parent, but does not immediately
 * delete it.  The NodePath retains a pointer to the node, and becomes a
 * singleton NodePath.
 *
 * This should be called to detach a node from the scene graph, with the
 * option of reattaching it later to the same parent or to a different parent.
 *
 * In practice, the only difference between remove_node() and detach_node() is
 * that remove_node() also resets the NodePath to empty, which will cause the
 * node to be deleted immediately if there are no other references.  On the
 * other hand, detach_node() leaves the NodePath referencing the node, which
 * will keep at least one reference to the node for as long as the NodePath
 * exists.
 */
void NodePath::
detach_node(Thread *current_thread) {
  nassertv(_error_type != ET_not_found);
  if (!is_empty() && !is_singleton()) {
    node()->reset_prev_transform();
    int pipeline_stage = current_thread->get_pipeline_stage();
    PandaNode::detach(_head, pipeline_stage, current_thread);
  }
}

/**
 * Lists the hierarchy at and above the referenced node.
 */
int NodePath::
reverse_ls(ostream &out, int indent_level) const {
  if (is_empty()) {
    out << "(empty)\n";
    return 0;
  } else if (has_parent()) {
    indent_level = get_parent().reverse_ls(out, indent_level);
  }
  node()->write(out, indent_level);
  return indent_level + 2;
}

/**
 * Writes a sensible description of the NodePath to the indicated output
 * stream.
 */
void NodePath::
output(ostream &out) const {
  switch (_error_type) {
  case ET_not_found:
    out << "**not found**";
    return;
  case ET_removed:
    out << "**removed**";
    return;
  case ET_fail:
    out << "**error**";
    return;
  default:
    break;
  }

  if (_head == nullptr) {
    out << "(empty)";
  } else {
    _head->output(out);
  }
}

/**
 * Returns the complete state object set on this node.
 */
const RenderState *NodePath::
get_state(Thread *current_thread) const {
  // This method is declared non-inline to avoid a compiler bug in gcc-3.4 and
  // gcc-4.0.
  nassertr_always(!is_empty(), RenderState::make_empty());
  return node()->get_state(current_thread);
}

/**
 * Returns the state changes that must be made to transition to the render
 * state of this node from the render state of the other node.
 */
CPT(RenderState) NodePath::
get_state(const NodePath &other, Thread *current_thread) const {
  nassertr(_error_type == ET_ok && other._error_type == ET_ok, RenderState::make_empty());

  if (other.is_empty()) {
    return get_net_state(current_thread);
  }
  if (is_empty()) {
    return other.get_net_state(current_thread)->invert_compose(RenderState::make_empty());
  }

#if defined(_DEBUG) || (defined(HAVE_THREADS) && defined(SIMPLE_THREADS))
  nassertr(verify_complete(current_thread), RenderState::make_empty());
  nassertr(other.verify_complete(current_thread), RenderState::make_empty());
#endif

  int a_count, b_count;
  if (find_common_ancestor(*this, other, a_count, b_count, current_thread) == nullptr) {
    if (allow_unrelated_wrt) {
      pgraph_cat.debug()
        << *this << " is not related to " << other << "\n";
    } else {
      pgraph_cat.error()
        << *this << " is not related to " << other << "\n";
      nassert_raise("unrelated nodes");
      return RenderState::make_empty();
    }
  }

  CPT(RenderState) a_state = r_get_partial_state(_head, a_count, current_thread);
  CPT(RenderState) b_state = r_get_partial_state(other._head, b_count, current_thread);
  return b_state->invert_compose(a_state);
}

/**
 * Sets the state object on this node, relative to the other node.  This
 * computes a new state object that will have the indicated value when seen
 * from the other node.
 */
void NodePath::
set_state(const NodePath &other, const RenderState *state,
          Thread *current_thread) {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  // First, we perform a wrt to the parent, to get the conversion.
  CPT(RenderState) rel_state;
  if (has_parent()) {
    rel_state = other.get_state(get_parent(current_thread), current_thread);
  } else {
    rel_state = other.get_state(NodePath(), current_thread);
  }

  CPT(RenderState) new_state = rel_state->compose(state);
  set_state(new_state, current_thread);
}

/**
 * Returns the complete transform object set on this node.
 */
const TransformState *NodePath::
get_transform(Thread *current_thread) const {
  // This method is declared non-inline to avoid a compiler bug in gcc-3.4 and
  // gcc-4.0.
  nassertr_always(!is_empty(), TransformState::make_identity());
  return node()->get_transform(current_thread);
}

/**
 * Returns the relative transform to this node from the other node; i.e.  the
 * transformation of this node as seen from the other node.
 */
CPT(TransformState) NodePath::
get_transform(const NodePath &other, Thread *current_thread) const {
  nassertr(_error_type == ET_ok && other._error_type == ET_ok, TransformState::make_identity());
  PStatTimer timer(_get_transform_pcollector);

  if (other.is_empty()) {
    return get_net_transform(current_thread);
  }
  if (is_empty()) {
    return other.get_net_transform(current_thread)->invert_compose(TransformState::make_identity());
  }

#if defined(_DEBUG) || (defined(HAVE_THREADS) && defined(SIMPLE_THREADS))
  nassertr(verify_complete(current_thread), TransformState::make_identity());
  nassertr(other.verify_complete(current_thread), TransformState::make_identity());
#endif

  int a_count, b_count;
  if (find_common_ancestor(*this, other, a_count, b_count, current_thread) == nullptr) {
    if (allow_unrelated_wrt) {
      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << *this << " is not related to " << other << "\n";
      }
    } else {
      pgraph_cat.error()
        << *this << " is not related to " << other << "\n";
      nassert_raise("unrelated nodes");
      return TransformState::make_identity();
    }
  }

  CPT(TransformState) a_transform, b_transform;

  a_transform = r_get_partial_transform(_head, a_count, current_thread);
  if (a_transform != nullptr) {
    b_transform = r_get_partial_transform(other._head, b_count, current_thread);
  }
  if (b_transform == nullptr) {
    // If either path involved a node with a net_transform RenderEffect
    // applied, we have to go all the way up to the root to get the right
    // answer.
    a_transform = r_get_net_transform(_head, current_thread);
    b_transform = r_get_net_transform(other._head, current_thread);
  }

  return b_transform->invert_compose(a_transform);
}

/**
 * Sets the transform object on this node, relative to the other node.  This
 * computes a new transform object that will have the indicated value when
 * seen from the other node.
 */
void NodePath::
set_transform(const NodePath &other, const TransformState *transform,
              Thread *current_thread) {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  // First, we perform a wrt to the parent, to get the conversion.
  CPT(TransformState) rel_trans;
  if (has_parent()) {
    rel_trans = other.get_transform(get_parent(current_thread), current_thread);
  } else {
    rel_trans = other.get_transform(NodePath(), current_thread);
  }

  CPT(TransformState) new_trans = rel_trans->compose(transform);
  set_transform(new_trans, current_thread);
}

/**
 * Returns the transform that has been set as this node's "previous" position.
 * See set_prev_transform().
 */
const TransformState *NodePath::
get_prev_transform(Thread *current_thread) const {
  // This method is declared non-inline to avoid a compiler bug in gcc-3.4 and
  // gcc-4.0.
  nassertr_always(!is_empty(), TransformState::make_identity());
  return node()->get_prev_transform(current_thread);
}

/**
 * Returns the relative "previous" transform to this node from the other node;
 * i.e.  the position of this node in the previous frame, as seen by the other
 * node in the previous frame.
 */
CPT(TransformState) NodePath::
get_prev_transform(const NodePath &other, Thread *current_thread) const {
  nassertr(_error_type == ET_ok && other._error_type == ET_ok, TransformState::make_identity());

  if (other.is_empty()) {
    return get_net_prev_transform(current_thread);
  }
  if (is_empty()) {
    return other.get_net_prev_transform(current_thread)->invert_compose(TransformState::make_identity());
  }

#if defined(_DEBUG) || (defined(HAVE_THREADS) && defined(SIMPLE_THREADS))
  nassertr(verify_complete(current_thread), TransformState::make_identity());
  nassertr(other.verify_complete(current_thread), TransformState::make_identity());
#endif

  int a_count, b_count;
  if (find_common_ancestor(*this, other, a_count, b_count, current_thread) == nullptr) {
    if (allow_unrelated_wrt) {
      pgraph_cat.debug()
        << *this << " is not related to " << other << "\n";
    } else {
      pgraph_cat.error()
        << *this << " is not related to " << other << "\n";
      nassert_raise("unrelated nodes");
      return TransformState::make_identity();
    }
  }

  CPT(TransformState) a_prev_transform = r_get_partial_prev_transform(_head, a_count, current_thread);
  CPT(TransformState) b_prev_transform = r_get_partial_prev_transform(other._head, b_count, current_thread);
  return b_prev_transform->invert_compose(a_prev_transform);
}

/**
 * Sets the "previous" transform object on this node, relative to the other
 * node.  This computes a new transform object that will have the indicated
 * value when seen from the other node.
 */
void NodePath::
set_prev_transform(const NodePath &other, const TransformState *transform,
                   Thread *current_thread) {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  // First, we perform a wrt to the parent, to get the conversion.
  CPT(TransformState) rel_trans;
  if (has_parent(current_thread)) {
    rel_trans = other.get_prev_transform(get_parent(current_thread), current_thread);
  } else {
    rel_trans = other.get_prev_transform(NodePath(), current_thread);
  }

  CPT(TransformState) new_trans = rel_trans->compose(transform);
  set_prev_transform(new_trans, current_thread);
}

/**
 * Sets the translation component of the transform, leaving rotation and scale
 * untouched.  This also resets the node's "previous" position, so that the
 * collision system will see the node as having suddenly appeared in the new
 * position, without passing any points in between.  See Also:
 * NodePath::set_fluid_pos
 */
void NodePath::
set_pos(const LVecBase3 &pos) {
  nassertv_always(!is_empty());
  set_transform(get_transform()->set_pos(pos));
  node()->reset_prev_transform();
}

void NodePath::
set_x(PN_stdfloat x) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[0] = x;
  set_pos(pos);
}

void NodePath::
set_y(PN_stdfloat y) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[1] = y;
  set_pos(pos);
}

void NodePath::
set_z(PN_stdfloat z) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[2] = z;
  set_pos(pos);
}

/**
 * Sets the translation component, without changing the "previous" position,
 * so that the collision system will see the node as moving fluidly from its
 * previous position to its new position.  See Also: NodePath::set_pos
 */
void NodePath::
set_fluid_pos(const LVecBase3 &pos) {
  nassertv_always(!is_empty());
  set_transform(get_transform()->set_pos(pos));
}

void NodePath::
set_fluid_x(PN_stdfloat x) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[0] = x;
  set_fluid_pos(pos);
}

void NodePath::
set_fluid_y(PN_stdfloat y) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[1] = y;
  set_fluid_pos(pos);
}

void NodePath::
set_fluid_z(PN_stdfloat z) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos();
  pos[2] = z;
  set_fluid_pos(pos);
}

/**
 * Retrieves the translation component of the transform.
 */
LPoint3 NodePath::
get_pos() const {
  nassertr_always(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  return get_transform()->get_pos();
}

/**
 * Returns the delta vector from this node's position in the previous frame
 * (according to set_prev_transform(), typically set via the use of
 * set_fluid_pos()) and its position in the current frame.  This is the vector
 * used to determine collisions.  Generally, if the node was last repositioned
 * via set_pos(), the delta will be zero; if it was adjusted via
 * set_fluid_pos(), the delta will represent the change from the previous
 * frame's position.
 */
LVector3 NodePath::
get_pos_delta() const {
  nassertr_always(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  return get_transform()->get_pos() - get_prev_transform()->get_pos();
}

/**
 * Sets the rotation component of the transform, leaving translation and scale
 * untouched.
 */
void NodePath::
set_hpr(const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_hpr());
  set_transform(transform->set_hpr(hpr));
}

void NodePath::
set_h(PN_stdfloat h) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_hpr());
  LVecBase3 hpr = transform->get_hpr();
  hpr[0] = h;
  set_transform(transform->set_hpr(hpr));
}

void NodePath::
set_p(PN_stdfloat p) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_hpr());
  LVecBase3 hpr = transform->get_hpr();
  hpr[1] = p;
  set_transform(transform->set_hpr(hpr));
}

void NodePath::
set_r(PN_stdfloat r) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  nassertv(transform->has_hpr());
  LVecBase3 hpr = transform->get_hpr();
  hpr[2] = r;
  set_transform(transform->set_hpr(hpr));
}

/**
 * Retrieves the rotation component of the transform.
 */
LVecBase3 NodePath::
get_hpr() const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform();
  nassertr(transform->has_hpr(), LVecBase3(0.0f, 0.0f, 0.0f));
  return transform->get_hpr();
}

/**
 * Sets the rotation component of the transform, leaving translation and scale
 * untouched.
 */
void NodePath::
set_quat(const LQuaternion &quat) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  set_transform(transform->set_quat(quat));
}

/**
 * Retrieves the rotation component of the transform.
 */
LQuaternion NodePath::
get_quat() const {
  nassertr_always(!is_empty(), LQuaternion::ident_quat());
  CPT(TransformState) transform = get_transform();
  return transform->get_quat();
}

/**
 * Sets the scale component of the transform, leaving translation and rotation
 * untouched.
 */
void NodePath::
set_scale(const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  set_transform(transform->set_scale(scale));
}

void NodePath::
set_sx(PN_stdfloat sx) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 scale = transform->get_scale();
  scale[0] = sx;
  set_transform(transform->set_scale(scale));
}

void NodePath::
set_sy(PN_stdfloat sy) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 scale = transform->get_scale();
  scale[1] = sy;
  set_transform(transform->set_scale(scale));
}

void NodePath::
set_sz(PN_stdfloat sz) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 scale = transform->get_scale();
  scale[2] = sz;
  set_transform(transform->set_scale(scale));
}

/**
 * Retrieves the scale component of the transform.
 */
LVecBase3 NodePath::
get_scale() const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform();
  return transform->get_scale();
}

/**
 * Sets the shear component of the transform, leaving translation and rotation
 * untouched.
 */
void NodePath::
set_shear(const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  set_transform(transform->set_shear(shear));
}

void NodePath::
set_shxy(PN_stdfloat shxy) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 shear = transform->get_shear();
  shear[0] = shxy;
  set_transform(transform->set_shear(shear));
}

void NodePath::
set_shxz(PN_stdfloat shxz) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 shear = transform->get_shear();
  shear[1] = shxz;
  set_transform(transform->set_shear(shear));
}

void NodePath::
set_shyz(PN_stdfloat shyz) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  LVecBase3 shear = transform->get_shear();
  shear[2] = shyz;
  set_transform(transform->set_shear(shear));
}

/**
 * Retrieves the shear component of the transform.
 */
LVecBase3 NodePath::
get_shear() const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform();
  return transform->get_shear();
}

/**
 * Sets the translation and rotation component of the transform, leaving scale
 * untouched.
 */
void NodePath::
set_pos_hpr(const LVecBase3 &pos, const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  transform = TransformState::make_pos_hpr_scale_shear
    (pos, hpr, transform->get_scale(), transform->get_shear());
  set_transform(transform);
  node()->reset_prev_transform();
}

/**
 * Sets the translation and rotation component of the transform, leaving scale
 * untouched.
 */
void NodePath::
set_pos_quat(const LVecBase3 &pos, const LQuaternion &quat) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  transform = TransformState::make_pos_quat_scale_shear
    (pos, quat, transform->get_scale(), transform->get_shear());
  set_transform(transform);
  node()->reset_prev_transform();
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.
 */
void NodePath::
set_hpr_scale(const LVecBase3 &hpr, const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  transform = TransformState::make_pos_hpr_scale_shear
    (transform->get_pos(), hpr, scale, transform->get_shear());
  set_transform(transform);
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.
 */
void NodePath::
set_quat_scale(const LQuaternion &quat, const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform();
  transform = TransformState::make_pos_quat_scale_shear
    (transform->get_pos(), quat, scale, transform->get_shear());
  set_transform(transform);
}

/**
 * Replaces the translation, rotation, and scale components, implicitly
 * setting shear to 0.
 */
void NodePath::
set_pos_hpr_scale(const LVecBase3 &pos, const LVecBase3 &hpr,
                  const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_pos_hpr_scale
                (pos, hpr, scale));
  node()->reset_prev_transform();
}

/**
 * Replaces the translation, rotation, and scale components, implicitly
 * setting shear to 0.
 */
void NodePath::
set_pos_quat_scale(const LVecBase3 &pos, const LQuaternion &quat,
                   const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_pos_quat_scale
                (pos, quat, scale));
  node()->reset_prev_transform();
}

/**
 * Completely replaces the transform with new translation, rotation, scale,
 * and shear components.
 */
void NodePath::
set_pos_hpr_scale_shear(const LVecBase3 &pos, const LVecBase3 &hpr,
                        const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_pos_hpr_scale_shear
                (pos, hpr, scale, shear));
  node()->reset_prev_transform();
}

/**
 * Completely replaces the transform with new translation, rotation, scale,
 * and shear components.
 */
void NodePath::
set_pos_quat_scale_shear(const LVecBase3 &pos, const LQuaternion &quat,
                         const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_pos_quat_scale_shear
                (pos, quat, scale, shear));
  node()->reset_prev_transform();
}

/**
 * Directly sets an arbitrary 4x4 transform matrix.
 */
void NodePath::
set_mat(const LMatrix4 &mat) {
  nassertv_always(!is_empty());
  set_transform(TransformState::make_mat(mat));
  node()->reset_prev_transform();
}

/**
 * Sets the hpr on this NodePath so that it rotates to face the indicated
 * point in space.
 */
void NodePath::
look_at(const LPoint3 &point, const LVector3 &up) {
  nassertv_always(!is_empty());

  LPoint3 pos = get_pos();

  LQuaternion quat;
  ::look_at(quat, point - pos, up);
  set_quat(quat);
}

/**
 * Behaves like look_at(), but with a strong preference to keeping the up
 * vector oriented in the indicated "up" direction.
 */
void NodePath::
heads_up(const LPoint3 &point, const LVector3 &up) {
  nassertv_always(!is_empty());

  LPoint3 pos = get_pos();

  LQuaternion quat;
  ::heads_up(quat, point - pos, up);
  set_quat(quat);
}

/**
 * Sets the translation component of the transform, relative to the other
 * node.
 */
void NodePath::
set_pos(const NodePath &other, const LVecBase3 &pos) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) orig_transform = get_transform();
  if (orig_transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_hpr = orig_transform->get_hpr();
    const LVecBase3 &orig_scale = orig_transform->get_scale();
    const LVecBase3 &orig_shear = orig_transform->get_shear();

    set_transform(other, rel_transform->set_pos(pos));
    set_pos_hpr_scale_shear(get_transform()->get_pos(), orig_hpr, orig_scale, orig_shear);

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_pos(pos));
  }
  node()->reset_prev_transform();
}

void NodePath::
set_x(const NodePath &other, PN_stdfloat x) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[0] = x;
  set_pos(other, pos);
}

void NodePath::
set_y(const NodePath &other, PN_stdfloat y) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[1] = y;
  set_pos(other, pos);
}

void NodePath::
set_z(const NodePath &other, PN_stdfloat z) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[2] = z;
  set_pos(other, pos);
}

/**
 * Sets the translation component of the transform, relative to the other
 * node.
 */
void NodePath::
set_fluid_pos(const NodePath &other, const LVecBase3 &pos) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) orig_transform = get_transform();
  if (orig_transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_hpr = orig_transform->get_hpr();
    const LVecBase3 &orig_scale = orig_transform->get_scale();
    const LVecBase3 &orig_shear = orig_transform->get_shear();

    // Use the relative set_transform() to compute the relative pos, and then
    // reset all of the other components back to the way they were.
    set_transform(other, rel_transform->set_pos(pos));
    set_transform(TransformState::make_pos_hpr_scale_shear
                  (get_transform()->get_pos(), orig_hpr, orig_scale, orig_shear));

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_pos(pos));
  }
}

void NodePath::
set_fluid_x(const NodePath &other, PN_stdfloat x) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[0] = x;
  set_fluid_pos(other, pos);
}

void NodePath::
set_fluid_y(const NodePath &other, PN_stdfloat y) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[1] = y;
  set_fluid_pos(other, pos);
}

void NodePath::
set_fluid_z(const NodePath &other, PN_stdfloat z) {
  nassertv_always(!is_empty());
  LPoint3 pos = get_pos(other);
  pos[2] = z;
  set_fluid_pos(other, pos);
}

/**
 * Returns the relative position of the referenced node as seen from the other
 * node.
 */
LPoint3 NodePath::
get_pos(const NodePath &other) const {
  nassertr_always(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  return get_transform(other)->get_pos();
}

/**
 * Returns the delta vector from this node's position in the previous frame
 * (according to set_prev_transform(), typically set via the use of
 * set_fluid_pos()) and its position in the current frame, as seen in the
 * indicated node's coordinate space.  This is the vector used to determine
 * collisions.  Generally, if the node was last repositioned via set_pos(),
 * the delta will be zero; if it was adjusted via set_fluid_pos(), the delta
 * will represent the change from the previous frame's position.
 */
LVector3 NodePath::
get_pos_delta(const NodePath &other) const {
  nassertr_always(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  return get_transform(other)->get_pos() - get_prev_transform(other)->get_pos();
}

/**
 * Sets the rotation component of the transform, relative to the other node.
 */
void NodePath::
set_hpr(const NodePath &other, const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);
  nassertv(rel_transform->has_hpr());

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_pos = transform->get_pos();
    const LVecBase3 &orig_scale = transform->get_scale();
    const LVecBase3 &orig_shear = transform->get_shear();

    set_transform(other, rel_transform->set_hpr(hpr));
    transform = get_transform();
    if (transform->has_components()) {
      set_transform(TransformState::make_pos_hpr_scale_shear
                    (orig_pos, transform->get_hpr(), orig_scale, orig_shear));
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_hpr(hpr));
  }
}

void NodePath::
set_h(const NodePath &other, PN_stdfloat h) {
  nassertv_always(!is_empty());
  LVecBase3 hpr = get_hpr(other);
  hpr[0] = h;
  set_hpr(other, hpr);
}

void NodePath::
set_p(const NodePath &other, PN_stdfloat p) {
  nassertv_always(!is_empty());
  LVecBase3 hpr = get_hpr(other);
  hpr[1] = p;
  set_hpr(other, hpr);
}

void NodePath::
set_r(const NodePath &other, PN_stdfloat r) {
  nassertv_always(!is_empty());
  LVecBase3 hpr = get_hpr(other);
  hpr[2] = r;
  set_hpr(other, hpr);
}

/**
 * Returns the relative orientation of the bottom node as seen from the other
 * node.
 */
LVecBase3 NodePath::
get_hpr(const NodePath &other) const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform(other);
  nassertr(transform->has_hpr(), LVecBase3(0.0f, 0.0f, 0.0f));
  return transform->get_hpr();
}

/**
 * Sets the rotation component of the transform, relative to the other node.
 */
void NodePath::
set_quat(const NodePath &other, const LQuaternion &quat) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_pos = transform->get_pos();
    const LVecBase3 &orig_scale = transform->get_scale();
    const LVecBase3 &orig_shear = transform->get_shear();

    set_transform(other, rel_transform->set_quat(quat));
    transform = get_transform();
    if (transform->has_components()) {
      set_transform(TransformState::make_pos_quat_scale_shear
                    (orig_pos, transform->get_quat(), orig_scale, orig_shear));
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_quat(quat));
  }
}

/**
 * Returns the relative orientation of the bottom node as seen from the other
 * node.
 */
LQuaternion NodePath::
get_quat(const NodePath &other) const {
  nassertr_always(!is_empty(), LQuaternion::ident_quat());
  CPT(TransformState) transform = get_transform(other);
  return transform->get_quat();
}

/**
 * Sets the scale component of the transform, relative to the other node.
 */
void NodePath::
set_scale(const NodePath &other, const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_pos = transform->get_pos();
    const LVecBase3 &orig_hpr = transform->get_hpr();
    const LVecBase3 &orig_shear = transform->get_shear();

    set_transform(other, rel_transform->set_scale(scale));
    transform = get_transform();
    if (transform->has_components()) {
      set_transform(TransformState::make_pos_hpr_scale_shear
                    (orig_pos, orig_hpr, transform->get_scale(), orig_shear));
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_scale(scale));
  }
}

void NodePath::
set_sx(const NodePath &other, PN_stdfloat sx) {
  nassertv_always(!is_empty());
  LVecBase3 scale = get_scale(other);
  scale[0] = sx;
  set_scale(other, scale);
}

void NodePath::
set_sy(const NodePath &other, PN_stdfloat sy) {
  nassertv_always(!is_empty());
  LVecBase3 scale = get_scale(other);
  scale[1] = sy;
  set_scale(other, scale);
}

void NodePath::
set_sz(const NodePath &other, PN_stdfloat sz) {
  nassertv_always(!is_empty());
  LVecBase3 scale = get_scale(other);
  scale[2] = sz;
  set_scale(other, scale);
}

/**
 * Returns the relative scale of the bottom node as seen from the other node.
 */
LVecBase3 NodePath::
get_scale(const NodePath &other) const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform(other);
  return transform->get_scale();
}

/**
 * Sets the shear component of the transform, relative to the other node.
 */
void NodePath::
set_shear(const NodePath &other, const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other three components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_pos = transform->get_pos();
    const LVecBase3 &orig_hpr = transform->get_hpr();
    const LVecBase3 &orig_scale = transform->get_scale();

    set_transform(other, rel_transform->set_shear(shear));
    transform = get_transform();
    if (transform->has_components()) {
      set_transform(TransformState::make_pos_hpr_scale_shear
                    (orig_pos, orig_hpr, orig_scale, transform->get_shear()));
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, rel_transform->set_shear(shear));
  }
}

void NodePath::
set_shxy(const NodePath &other, PN_stdfloat shxy) {
  nassertv_always(!is_empty());
  LVecBase3 shear = get_shear(other);
  shear[0] = shxy;
  set_shear(other, shear);
}

void NodePath::
set_shxz(const NodePath &other, PN_stdfloat shxz) {
  nassertv_always(!is_empty());
  LVecBase3 shear = get_shear(other);
  shear[1] = shxz;
  set_shear(other, shear);
}

void NodePath::
set_shyz(const NodePath &other, PN_stdfloat shyz) {
  nassertv_always(!is_empty());
  LVecBase3 shear = get_shear(other);
  shear[2] = shyz;
  set_shear(other, shear);
}

/**
 * Returns the relative shear of the bottom node as seen from the other node.
 */
LVecBase3 NodePath::
get_shear(const NodePath &other) const {
  nassertr_always(!is_empty(), LVecBase3(0.0f, 0.0f, 0.0f));
  CPT(TransformState) transform = get_transform(other);
  return transform->get_shear();
}

/**
 * Sets the translation and rotation component of the transform, relative to
 * the other node.
 */
void NodePath::
set_pos_hpr(const NodePath &other, const LVecBase3 &pos,
            const LVecBase3 &hpr) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other two components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_scale = transform->get_scale();
    const LVecBase3 &orig_shear = transform->get_shear();

    set_transform(other, TransformState::make_pos_hpr_scale_shear
                  (pos, hpr, rel_transform->get_scale(), rel_transform->get_shear()));
    transform = get_transform();
    if (transform->has_components()) {
      set_pos_hpr_scale_shear(transform->get_pos(), transform->get_hpr(),
                              orig_scale, orig_shear);
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, TransformState::make_pos_hpr_scale_shear
                  (pos, hpr, rel_transform->get_scale(), rel_transform->get_shear()));
    node()->reset_prev_transform();
  }
}

/**
 * Sets the translation and rotation component of the transform, relative to
 * the other node.
 */
void NodePath::
set_pos_quat(const NodePath &other, const LVecBase3 &pos,
             const LQuaternion &quat) {
  nassertv_always(!is_empty());
  CPT(TransformState) rel_transform = get_transform(other);

  CPT(TransformState) transform = get_transform();
  if (transform->has_components()) {
    // If we had a componentwise transform before we started, we should be
    // careful to preserve the other two components.  We wouldn't need to do
    // this, except for the possibility of numerical error or decompose
    // ambiguity.
    const LVecBase3 &orig_scale = transform->get_scale();
    const LVecBase3 &orig_shear = transform->get_shear();

    set_transform(other, TransformState::make_pos_quat_scale_shear
                  (pos, quat, rel_transform->get_scale(), rel_transform->get_shear()));
    transform = get_transform();
    if (transform->has_components()) {
      set_pos_quat_scale_shear(transform->get_pos(), transform->get_quat(),
                               orig_scale, orig_shear);
    }

  } else {
    // If we didn't have a componentwise transform already, never mind.
    set_transform(other, TransformState::make_pos_quat_scale_shear
                  (pos, quat, rel_transform->get_scale(), rel_transform->get_shear()));
    node()->reset_prev_transform();
  }
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.  This, or set_pos_hpr_scale, is the preferred way to
 * update a transform when both hpr and scale are to be changed.
 */
void NodePath::
set_hpr_scale(const NodePath &other, const LVecBase3 &hpr, const LVecBase3 &scale) {
  // We don't bother trying very hard to preserve pos across this operation,
  // unlike the work we do above to preserve hpr or scale, since it generally
  // doesn't matter that much if pos is off by a few thousandths.
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  transform = TransformState::make_pos_hpr_scale_shear
    (transform->get_pos(), hpr, scale, transform->get_shear());
  set_transform(other, transform);
}

/**
 * Sets the rotation and scale components of the transform, leaving
 * translation untouched.  This, or set_pos_quat_scale, is the preferred way
 * to update a transform when both quat and scale are to be changed.
 */
void NodePath::
set_quat_scale(const NodePath &other, const LQuaternion &quat,
               const LVecBase3 &scale) {
  // We don't bother trying very hard to preserve pos across this operation,
  // unlike the work we do above to preserve quat or scale, since it generally
  // doesn't matter that much if pos is off by a few thousandths.
  nassertv_always(!is_empty());
  CPT(TransformState) transform = get_transform(other);
  transform = TransformState::make_pos_quat_scale_shear
    (transform->get_pos(), quat, scale, transform->get_shear());
  set_transform(other, transform);
}

/**
 * Completely replaces the transform with new translation, rotation, and scale
 * components, relative to the other node, implicitly setting shear to 0.
 */
void NodePath::
set_pos_hpr_scale(const NodePath &other,
                  const LVecBase3 &pos, const LVecBase3 &hpr,
                  const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_pos_hpr_scale
                (pos, hpr, scale));
  node()->reset_prev_transform();
}

/**
 * Completely replaces the transform with new translation, rotation, and scale
 * components, relative to the other node, implicitly setting shear to 0.
 */
void NodePath::
set_pos_quat_scale(const NodePath &other,
                   const LVecBase3 &pos, const LQuaternion &quat,
                   const LVecBase3 &scale) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_pos_quat_scale
                (pos, quat, scale));
  node()->reset_prev_transform();
}

/**
 * Completely replaces the transform with new translation, rotation, scale,
 * and shear components, relative to the other node.
 */
void NodePath::
set_pos_hpr_scale_shear(const NodePath &other,
                        const LVecBase3 &pos, const LVecBase3 &hpr,
                        const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_pos_hpr_scale_shear
                (pos, hpr, scale, shear));
  node()->reset_prev_transform();
}

/**
 * Completely replaces the transform with new translation, rotation, scale,
 * and shear components, relative to the other node.
 */
void NodePath::
set_pos_quat_scale_shear(const NodePath &other,
                         const LVecBase3 &pos, const LQuaternion &quat,
                         const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_pos_quat_scale_shear
                (pos, quat, scale, shear));
  node()->reset_prev_transform();
}

/**
 * Returns the matrix that describes the coordinate space of the bottom node,
 * relative to the other path's bottom node's coordinate space.
 */
LMatrix4 NodePath::
get_mat(const NodePath &other) const {
  CPT(TransformState) transform = get_transform(other);
  // We can't safely return a reference to the matrix, because we can't assume
  // the transform won't go away when the function returns.  If the transform
  // was partially modified by, say, a CompassEffect, it won't be stored in
  // the cache, and thus we might have the only reference to it.
  return transform->get_mat();
}

/**
 * Converts the indicated matrix from the other's coordinate space to the
 * local coordinate space, and applies it to the node.
 */
void NodePath::
set_mat(const NodePath &other, const LMatrix4 &mat) {
  nassertv_always(!is_empty());
  set_transform(other, TransformState::make_mat(mat));
  node()->reset_prev_transform();
}

/**
 * Given that the indicated point is in the coordinate system of the other
 * node, returns the same point in this node's coordinate system.
 */
LPoint3 NodePath::
get_relative_point(const NodePath &other, const LVecBase3 &point) const {
  CPT(TransformState) transform = other.get_transform(*this);
  LPoint3 rel_point = LPoint3(point) * transform->get_mat();
  return rel_point;
}

/**
 * Given that the indicated vector is in the coordinate system of the other
 * node, returns the same vector in this node's coordinate system.
 */
LVector3 NodePath::
get_relative_vector(const NodePath &other, const LVecBase3 &vec) const {
  CPT(TransformState) transform = other.get_transform(*this);
  LVector3 rel_vector = LVector3(vec) * transform->get_mat();
  return rel_vector;
}

/**
 * Sets the transform on this NodePath so that it rotates to face the
 * indicated point in space, which is relative to the other NodePath.
 */
void NodePath::
look_at(const NodePath &other, const LPoint3 &point, const LVector3 &up) {
  nassertv_always(!is_empty());

  CPT(TransformState) transform = other.get_transform(get_parent());
  LPoint3 rel_point = point * transform->get_mat();

  LPoint3 pos = get_pos();

  LQuaternion quat;
  ::look_at(quat, rel_point - pos, up);
  set_quat(quat);
}

/**
 * Behaves like look_at(), but with a strong preference to keeping the up
 * vector oriented in the indicated "up" direction.
 */
void NodePath::
heads_up(const NodePath &other, const LPoint3 &point, const LVector3 &up) {
  nassertv_always(!is_empty());

  CPT(TransformState) transform = other.get_transform(get_parent());
  LPoint3 rel_point = point * transform->get_mat();

  LPoint3 pos = get_pos();

  LQuaternion quat;
  ::heads_up(quat, rel_point - pos, up);
  set_quat(quat);
}


/**
 * Applies a scene-graph color to the referenced node.  This color will apply
 * to all geometry at this level and below (that does not specify a new color
 * or a set_color_off()).
 */
void NodePath::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a,
          int priority) {
  set_color(LColor(r, g, b, a), priority);
}

/**
 * Applies a scene-graph color to the referenced node.  This color will apply
 * to all geometry at this level and below (that does not specify a new color
 * or a set_color_off()).
 */
void NodePath::
set_color(const LColor &color, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ColorAttrib::make_flat(color), priority);
}

/**
 * Sets the geometry at this level and below to render using the geometry
 * color.  This is normally the default, but it may be useful to use this to
 * contradict set_color() at a higher node level (or, with a priority, to
 * override a set_color() at a lower level).
 */
void NodePath::
set_color_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ColorAttrib::make_vertex(), priority);
}

/**
 * Completely removes any color adjustment from the node.  This allows the
 * natural color of the geometry, or whatever color transitions might be
 * otherwise affecting the geometry, to show instead.
 */
void NodePath::
clear_color() {
  nassertv_always(!is_empty());
  node()->clear_attrib(ColorAttrib::get_class_slot());
}

/**
 * Returns true if a color has been applied to the given node, false
 * otherwise.
 */
bool NodePath::
has_color() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(ColorAttrib::get_class_slot());
}

/**
 * Returns the color that has been assigned to the node, or black if no color
 * has been assigned.
 */
LColor NodePath::
get_color() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(ColorAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ColorAttrib *ca = DCAST(ColorAttrib, attrib);
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      return ca->get_color();
    }
  }

  pgraph_cat.warning()
    << "get_color() called on " << *this << " which has no color set.\n";

  return LColor(1.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * Returns true if a color scale has been applied to the referenced node,
 * false otherwise.  It is still possible that color at this node might have
 * been scaled by an ancestor node.
 */
bool NodePath::
has_color_scale() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(ColorScaleAttrib::get_class_slot());
}

/**
 * Completely removes any color scale from the referenced node.  This is
 * preferable to simply setting the color scale to identity, as it also
 * removes the overhead associated with having a color scale at all.
 */
void NodePath::
clear_color_scale() {
  nassertv_always(!is_empty());
  node()->clear_attrib(ColorScaleAttrib::get_class_slot());
}

/**
 * multiplies the color scale component of the transform, with previous color
 * scale leaving translation and rotation untouched.
 */
void NodePath::
compose_color_scale(const LVecBase4 &scale, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ColorScaleAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ColorScaleAttrib::get_class_slot()));
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);

    // Modify the existing ColorScaleAttrib by multiplying with the indicated
    // colorScale.
    LVecBase4 prev_color_scale = csa->get_scale();
    LVecBase4 new_color_scale(prev_color_scale[0]*scale[0],
                               prev_color_scale[1]*scale[1],
                               prev_color_scale[2]*scale[2],
                               prev_color_scale[3]*scale[3]);
    node()->set_attrib(csa->set_scale(new_color_scale), priority);

  } else {
    // Create a new ColorScaleAttrib for this node.
    node()->set_attrib(ColorScaleAttrib::make(scale), priority);
  }
}

/**
 * Sets the color scale component of the transform, leaving translation and
 * rotation untouched.
 */
void NodePath::
set_color_scale(const LVecBase4 &scale, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ColorScaleAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ColorScaleAttrib::get_class_slot()));
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);

    // Modify the existing ColorScaleAttrib to add the indicated colorScale.
    node()->set_attrib(csa->set_scale(scale), priority);

  } else {
    // Create a new ColorScaleAttrib for this node.
    node()->set_attrib(ColorScaleAttrib::make(scale), priority);
  }
}

/**
 * Disables any color scale attribute inherited from above.  This is not the
 * same thing as clear_color_scale(), which undoes any previous
 * set_color_scale() operation on this node; rather, this actively disables
 * any set_color_scale() that might be inherited from a parent node.  This
 * also disables set_alpha_scale() at the same time.
 *
 * It is legal to specify a new color scale on the same node with a subsequent
 * call to set_color_scale() or set_alpha_scale(); this new scale will apply
 * to lower geometry.
 */
void NodePath::
set_color_scale_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ColorScaleAttrib::make_off(), priority);
}

/**
 * Sets the alpha scale component of the transform without (much) affecting
 * the color scale.  Note that any priority specified will also apply to the
 * color scale.
 */
void NodePath::
set_alpha_scale(PN_stdfloat scale, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ColorScaleAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ColorScaleAttrib::get_class_slot()));
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);

    // Modify the existing ColorScaleAttrib to add the indicated colorScale.
    const LVecBase4 &sc = csa->get_scale();
    node()->set_attrib(csa->set_scale(LVecBase4(sc[0], sc[1], sc[2], scale)), priority);

  } else {
    // Create a new ColorScaleAttrib for this node.
    node()->set_attrib(ColorScaleAttrib::make(LVecBase4(1.0f, 1.0f, 1.0f, scale)), priority);
  }
}

/**
 * Scales all the color components of the object by the same amount, darkening
 * the object, without (much) affecting alpha.  Note that any priority
 * specified will also apply to the alpha scale.
 */
void NodePath::
set_all_color_scale(PN_stdfloat scale, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ColorScaleAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ColorScaleAttrib::get_class_slot()));
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);

    // Modify the existing ColorScaleAttrib to add the indicated colorScale.
    const LVecBase4 &sc = csa->get_scale();
    node()->set_attrib(csa->set_scale(LVecBase4(scale, scale, scale, sc[3])), priority);

  } else {
    // Create a new ColorScaleAttrib for this node.
    node()->set_attrib(ColorScaleAttrib::make(LVecBase4(scale, scale, scale, 1.0f)), priority);
  }
}

/**
 * Returns the complete color scale vector that has been applied to this node
 * via a previous call to set_color_scale() and/or set_alpha_scale(), or all
 * 1's (identity) if no scale has been applied to this particular node.
 */
const LVecBase4 &NodePath::
get_color_scale() const {
  static const LVecBase4 ident_scale(1.0f, 1.0f, 1.0f, 1.0f);
  nassertr_always(!is_empty(), ident_scale);
  const RenderAttrib *attrib =
    node()->get_attrib(ColorScaleAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);
    return csa->get_scale();
  }

  return ident_scale;
}

/**
 * Adds the indicated Light or PolylightNode to the list of lights that
 * illuminate geometry at this node and below.  The light itself should be
 * parented into the scene graph elsewhere, to represent the light's position
 * in space; but until set_light() is called it will illuminate no geometry.
 */
void NodePath::
set_light(const NodePath &light, int priority) {
  nassertv_always(!is_empty());
  if (!light.is_empty()) {
    Light *light_obj = light.node()->as_light();
    if (light_obj != nullptr) {
      // It's an actual Light object.
      const RenderAttrib *attrib =
        node()->get_attrib(LightAttrib::get_class_slot());
      if (attrib != nullptr) {
        priority = max(priority,
                       node()->get_state()->get_override(LightAttrib::get_class_slot()));
        const LightAttrib *la = DCAST(LightAttrib, attrib);

        // Modify the existing LightAttrib to add the indicated light.
        node()->set_attrib(la->add_on_light(light), priority);

      } else {
        // Create a new LightAttrib for this node.
        CPT(LightAttrib) la = DCAST(LightAttrib, LightAttrib::make());
        node()->set_attrib(la->add_on_light(light), priority);
      }
      return;

    } else if (light.node()->is_of_type(PolylightNode::get_class_type())) {
      // It's a Polylight object.
      if (priority != 0) {
        // PolylightEffects can't have a priority, since they're just an
        // effect to be applied immediately.
        pgraph_cat.warning()
          << "Ignoring priority on set_light(" << light << ")\n";
      }

      const RenderEffect *effect =
        node()->get_effect(PolylightEffect::get_class_type());
      if (effect != nullptr) {
        const PolylightEffect *ple = DCAST(PolylightEffect, effect);

        // Modify the existing PolylightEffect to add the indicated light.
        node()->set_effect(ple->add_light(light));

      } else {
        // Create a new PolylightEffect for this node.
        CPT(PolylightEffect) ple = DCAST(PolylightEffect, PolylightEffect::make());
        node()->set_effect(ple->add_light(light));
      }
      return;
    }
  }
  nassert_raise("Not a Light object.");
}

/**
 * Sets the geometry at this level and below to render using no lights at all.
 * This is different from not specifying a light; rather, this specifically
 * contradicts set_light() at a higher node level (or, with a priority,
 * overrides a set_light() at a lower level).
 *
 * If no lights are in effect on a particular piece of geometry, that geometry
 * is rendered with lighting disabled.
 */
void NodePath::
set_light_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(LightAttrib::make_all_off(), priority);
  node()->clear_effect(PolylightEffect::get_class_type());
}

/**
 * Sets the geometry at this level and below to render without using the
 * indicated Light.  This is different from not specifying the Light; rather,
 * this specifically contradicts set_light() at a higher node level (or, with
 * a priority, overrides a set_light() at a lower level).
 *
 * This interface does not support PolylightNodes, which cannot be turned off
 * at a lower level.
 */
void NodePath::
set_light_off(const NodePath &light, int priority) {
  nassertv_always(!is_empty());

  if (!light.is_empty()) {
    Light *light_obj = light.node()->as_light();
    if (light_obj != nullptr) {
      const RenderAttrib *attrib =
        node()->get_attrib(LightAttrib::get_class_slot());
      if (attrib != nullptr) {
        priority = max(priority,
                       node()->get_state()->get_override(LightAttrib::get_class_slot()));
        const LightAttrib *la = DCAST(LightAttrib, attrib);

        // Modify the existing LightAttrib to add the indicated light to the
        // "off" list.  This also, incidentally, removes it from the "on" list
        // if it is there.
        node()->set_attrib(la->add_off_light(light), priority);

      } else {
        // Create a new LightAttrib for this node that turns off the indicated
        // light.
        CPT(LightAttrib) la = DCAST(LightAttrib, LightAttrib::make());
        node()->set_attrib(la->add_off_light(light), priority);
      }
      return;
    }
  }
  nassert_raise("Not a Light object.");
}

/**
 * Completely removes any lighting operations that may have been set via
 * set_light() or set_light_off() from this particular node.
 */
void NodePath::
clear_light() {
  nassertv_always(!is_empty());
  node()->clear_attrib(LightAttrib::get_class_slot());
  node()->clear_effect(PolylightEffect::get_class_type());
}

/**
 * Removes any reference to the indicated Light or PolylightNode from the
 * NodePath.
 */
void NodePath::
clear_light(const NodePath &light) {
  nassertv_always(!is_empty());

  if (!light.is_empty()) {
    Light *light_obj = light.node()->as_light();
    if (light_obj != nullptr) {
      const RenderAttrib *attrib =
        node()->get_attrib(LightAttrib::get_class_slot());
      if (attrib != nullptr) {
        CPT(LightAttrib) la = DCAST(LightAttrib, attrib);
        la = DCAST(LightAttrib, la->remove_on_light(light));
        la = DCAST(LightAttrib, la->remove_off_light(light));

        if (la->is_identity()) {
          node()->clear_attrib(LightAttrib::get_class_slot());

        } else {
          int priority = node()->get_state()->get_override(LightAttrib::get_class_slot());
          node()->set_attrib(la, priority);
        }
      }
      return;

    } else if (light.node()->is_of_type(PolylightNode::get_class_type())) {
      const RenderEffect *effect =
        node()->get_effect(PolylightEffect::get_class_type());
      if (effect != nullptr) {
        CPT(PolylightEffect) ple = DCAST(PolylightEffect, effect);
        ple = DCAST(PolylightEffect, ple->remove_light(light));
        node()->set_effect(ple);
      }
      return;
    }
  }
  nassert_raise("Not a Light object.");
}

/**
 * Returns true if the indicated Light or PolylightNode has been specifically
 * enabled on this particular node.  This means that someone called
 * set_light() on this node with the indicated light.
 */
bool NodePath::
has_light(const NodePath &light) const {
  nassertr_always(!is_empty(), false);

  if (!light.is_empty()) {
    Light *light_obj = light.node()->as_light();
    if (light_obj != nullptr) {
      const RenderAttrib *attrib =
        node()->get_attrib(LightAttrib::get_class_slot());
      if (attrib != nullptr) {
        const LightAttrib *la = DCAST(LightAttrib, attrib);
        return la->has_on_light(light);
      }
      return false;

    } else if (light.node()->is_of_type(PolylightNode::get_class_type())) {
      const RenderEffect *effect =
        node()->get_effect(PolylightEffect::get_class_type());
      if (effect != nullptr) {
        const PolylightEffect *ple = DCAST(PolylightEffect, effect);
        return ple->has_light(light);
      }
      return false;
    }
  }
  nassert_raise("Not a Light object.");
  return false;
}

/**
 * Returns true if all Lights have been specifically disabled on this
 * particular node.  This means that someone called set_light_off() on this
 * node with no parameters.
 */
bool NodePath::
has_light_off() const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(LightAttrib::get_class_slot());
  if (attrib != nullptr) {
    const LightAttrib *la = DCAST(LightAttrib, attrib);
    return la->has_all_off();
  }

  return false;
}

/**
 * Returns true if the indicated Light has been specifically disabled on this
 * particular node.  This means that someone called set_light_off() on this
 * node with the indicated light.
 *
 * This interface does not support PolylightNodes, which cannot be turned off
 * at a lower level.
 */
bool NodePath::
has_light_off(const NodePath &light) const {
  nassertr_always(!is_empty(), false);
  if (!light.is_empty()) {
    Light *light_obj = light.node()->as_light();
    if (light_obj != nullptr) {
      const RenderAttrib *attrib =
        node()->get_attrib(LightAttrib::get_class_slot());
      if (attrib != nullptr) {
        const LightAttrib *la = DCAST(LightAttrib, attrib);
        return la->has_off_light(light);
      }
    }
  }
  nassert_raise("Not a Light object.");
  return false;
}

/**
 * Adds the indicated clipping plane to the list of planes that apply to
 * geometry at this node and below.  The clipping plane itself, a PlaneNode,
 * should be parented into the scene graph elsewhere, to represent the plane's
 * position in space; but until set_clip_plane() is called it will clip no
 * geometry.
 */
void NodePath::
set_clip_plane(const NodePath &clip_plane, int priority) {
  nassertv_always(!is_empty());
  if (!clip_plane.is_empty() && clip_plane.node()->is_of_type(PlaneNode::get_class_type())) {
    const RenderAttrib *attrib =
      node()->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (attrib != nullptr) {
      priority = max(priority,
                     node()->get_state()->get_override(ClipPlaneAttrib::get_class_slot()));
      const ClipPlaneAttrib *la = DCAST(ClipPlaneAttrib, attrib);

      // Modify the existing ClipPlaneAttrib to add the indicated clip_plane.
      node()->set_attrib(la->add_on_plane(clip_plane), priority);

    } else {
      // Create a new ClipPlaneAttrib for this node.
      CPT(ClipPlaneAttrib) la = DCAST(ClipPlaneAttrib, ClipPlaneAttrib::make());
      node()->set_attrib(la->add_on_plane(clip_plane), priority);
    }
    return;
  }
  nassert_raise("Not a PlaneNode object.");
}

/**
 * Sets the geometry at this level and below to render using no clip_planes at
 * all.  This is different from not specifying a clip_plane; rather, this
 * specifically contradicts set_clip_plane() at a higher node level (or, with
 * a priority, overrides a set_clip_plane() at a lower level).
 *
 * If no clip_planes are in effect on a particular piece of geometry, that
 * geometry is rendered without being clipped (other than by the viewing
 * frustum).
 */
void NodePath::
set_clip_plane_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(ClipPlaneAttrib::make_all_off(), priority);
}

/**
 * Sets the geometry at this level and below to render without being clipped
 * by the indicated PlaneNode.  This is different from not specifying the
 * PlaneNode; rather, this specifically contradicts set_clip_plane() at a
 * higher node level (or, with a priority, overrides a set_clip_plane() at a
 * lower level).
 */
void NodePath::
set_clip_plane_off(const NodePath &clip_plane, int priority) {
  nassertv_always(!is_empty());

  if (!clip_plane.is_empty() && clip_plane.node()->is_of_type(PlaneNode::get_class_type())) {
    const RenderAttrib *attrib =
      node()->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (attrib != nullptr) {
      priority = max(priority,
                     node()->get_state()->get_override(ClipPlaneAttrib::get_class_slot()));
      const ClipPlaneAttrib *la = DCAST(ClipPlaneAttrib, attrib);

      // Modify the existing ClipPlaneAttrib to add the indicated clip_plane
      // to the "off" list.  This also, incidentally, removes it from the "on"
      // list if it is there.
      node()->set_attrib(la->add_off_plane(clip_plane), priority);

    } else {
      // Create a new ClipPlaneAttrib for this node that turns off the
      // indicated clip_plane.
      CPT(ClipPlaneAttrib) la = DCAST(ClipPlaneAttrib, ClipPlaneAttrib::make());
      node()->set_attrib(la->add_off_plane(clip_plane), priority);
    }
    return;
  }
  nassert_raise("Not a PlaneNode object.");
}

/**
 * Completely removes any clip planes that may have been set via
 * set_clip_plane() or set_clip_plane_off() from this particular node.
 */
void NodePath::
clear_clip_plane() {
  nassertv_always(!is_empty());
  node()->clear_attrib(ClipPlaneAttrib::get_class_slot());
}

/**
 * Removes any reference to the indicated clipping plane from the NodePath.
 */
void NodePath::
clear_clip_plane(const NodePath &clip_plane) {
  nassertv_always(!is_empty());

  if (!clip_plane.is_empty() && clip_plane.node()->is_of_type(PlaneNode::get_class_type())) {
    const RenderAttrib *attrib =
      node()->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (attrib != nullptr) {
      CPT(ClipPlaneAttrib) la = DCAST(ClipPlaneAttrib, attrib);
      la = DCAST(ClipPlaneAttrib, la->remove_on_plane(clip_plane));
      la = DCAST(ClipPlaneAttrib, la->remove_off_plane(clip_plane));

      if (la->is_identity()) {
        node()->clear_attrib(ClipPlaneAttrib::get_class_slot());

      } else {
        int priority = node()->get_state()->get_override(ClipPlaneAttrib::get_class_slot());
        node()->set_attrib(la, priority);
      }
    }
    return;
  }
  nassert_raise("Not a PlaneNode object.");
}

/**
 * Returns true if the indicated clipping plane has been specifically applied
 * to this particular node.  This means that someone called set_clip_plane()
 * on this node with the indicated clip_plane.
 */
bool NodePath::
has_clip_plane(const NodePath &clip_plane) const {
  nassertr_always(!is_empty(), false);

  if (!clip_plane.is_empty() && clip_plane.node()->is_of_type(PlaneNode::get_class_type())) {
    const RenderAttrib *attrib =
      node()->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (attrib != nullptr) {
      const ClipPlaneAttrib *la = DCAST(ClipPlaneAttrib, attrib);
      return la->has_on_plane(clip_plane);
    }
    return false;
  }
  nassert_raise("Not a PlaneNode object.");
  return false;
}

/**
 * Returns true if all clipping planes have been specifically disabled on this
 * particular node.  This means that someone called set_clip_plane_off() on
 * this node with no parameters.
 */
bool NodePath::
has_clip_plane_off() const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(ClipPlaneAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ClipPlaneAttrib *la = DCAST(ClipPlaneAttrib, attrib);
    return la->has_all_off();
  }

  return false;
}

/**
 * Returns true if the indicated clipping plane has been specifically disabled
 * on this particular node.  This means that someone called
 * set_clip_plane_off() on this node with the indicated clip_plane.
 */
bool NodePath::
has_clip_plane_off(const NodePath &clip_plane) const {
  nassertr_always(!is_empty(), false);
  if (!clip_plane.is_empty() && clip_plane.node()->is_of_type(PlaneNode::get_class_type())) {
    const RenderAttrib *attrib =
      node()->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (attrib != nullptr) {
      const ClipPlaneAttrib *la = DCAST(ClipPlaneAttrib, attrib);
      return la->has_off_plane(clip_plane);
    }
  }
  nassert_raise("Not a PlaneNode object.");
  return false;
}

/**
 * Adds the indicated occluder to the list of occluders that apply to geometry
 * at this node and below.  The occluder itself, an OccluderNode, should be
 * parented into the scene graph elsewhere, to represent the occluder's
 * position in space; but until set_occluder() is called it will clip no
 * geometry.
 */
void NodePath::
set_occluder(const NodePath &occluder) {
  nassertv_always(!is_empty());
  if (!occluder.is_empty() && occluder.node()->is_of_type(OccluderNode::get_class_type())) {
    const RenderEffect *effect =
      node()->get_effect(OccluderEffect::get_class_type());
    if (effect != nullptr) {
      const OccluderEffect *la = DCAST(OccluderEffect, effect);

      // Modify the existing OccluderEffect to add the indicated occluder.
      node()->set_effect(la->add_on_occluder(occluder));

    } else {
      // Create a new OccluderEffect for this node.
      CPT(OccluderEffect) la = DCAST(OccluderEffect, OccluderEffect::make());
      node()->set_effect(la->add_on_occluder(occluder));
    }
    return;
  }
  nassert_raise("Not an OccluderNode object.");
}

/**
 * Completely removes any occluders that may have been set via set_occluder()
 * from this particular node.
 */
void NodePath::
clear_occluder() {
  nassertv_always(!is_empty());
  node()->clear_effect(OccluderEffect::get_class_type());
}

/**
 * Removes any reference to the indicated occluder from the NodePath.
 */
void NodePath::
clear_occluder(const NodePath &occluder) {
  nassertv_always(!is_empty());

  if (!occluder.is_empty() && occluder.node()->is_of_type(OccluderNode::get_class_type())) {
    const RenderEffect *effect =
      node()->get_effect(OccluderEffect::get_class_type());
    if (effect != nullptr) {
      CPT(OccluderEffect) la = DCAST(OccluderEffect, effect);
      la = DCAST(OccluderEffect, la->remove_on_occluder(occluder));

      if (la->is_identity()) {
        node()->clear_effect(OccluderEffect::get_class_type());

      } else {
        node()->set_effect(la);
      }
    }
    return;
  }
  nassert_raise("Not an OccluderNode object.");
}

/**
 * Returns true if the indicated occluder has been specifically applied to
 * this particular node.  This means that someone called set_occluder() on
 * this node with the indicated occluder.
 */
bool NodePath::
has_occluder(const NodePath &occluder) const {
  nassertr_always(!is_empty(), false);

  if (!occluder.is_empty() && occluder.node()->is_of_type(OccluderNode::get_class_type())) {
    const RenderEffect *effect =
      node()->get_effect(OccluderEffect::get_class_type());
    if (effect != nullptr) {
      const OccluderEffect *la = DCAST(OccluderEffect, effect);
      return la->has_on_occluder(occluder);
    }
    return false;
  }
  nassert_raise("Not an OccluderNode object.");
  return false;
}

/**
 * Sets up a scissor region on the nodes rendered at this level and below.
 * The four coordinates are understood to define a rectangle in screen space.
 * These numbers are relative to the current DisplayRegion, where (0,0) is the
 * lower-left corner of the DisplayRegion, and (1,1) is the upper-right
 * corner.
 */
void NodePath::
set_scissor(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_effect(ScissorEffect::make_screen(LVecBase4(left, right, bottom, top)));
}

/**
 * Sets up a scissor region on the nodes rendered at this level and below.
 * The two points are understood to be relative to this node.  When these
 * points are projected into screen space, they define the diagonally-opposite
 * points that determine the scissor region.
 */
void NodePath::
set_scissor(const LPoint3 &a, const LPoint3 &b) {
  set_effect(ScissorEffect::make_node(a, b));
}

/**
 * Sets up a scissor region on the nodes rendered at this level and below.
 * The four points are understood to be relative to this node.  When these
 * points are projected into screen space, they define the bounding volume of
 * the scissor region (the scissor region is the smallest onscreen rectangle
 * that encloses all four points).
 */
void NodePath::
set_scissor(const LPoint3 &a, const LPoint3 &b,
            const LPoint3 &c, const LPoint3 &d) {
  set_effect(ScissorEffect::make_node(a, b, c, d));
}

/**
 * Sets up a scissor region on the nodes rendered at this level and below.
 * The two points are understood to be relative to the indicated other node.
 * When these points are projected into screen space, they define the
 * diagonally-opposite points that determine the scissor region.
 */
void NodePath::
set_scissor(const NodePath &other, const LPoint3 &a, const LPoint3 &b) {
  set_effect(ScissorEffect::make_node(a, b, other));
}

/**
 * Sets up a scissor region on the nodes rendered at this level and below.
 * The four points are understood to be relative to the indicated other node.
 * When these points are projected into screen space, they define the bounding
 * volume of the scissor region (the scissor region is the smallest onscreen
 * rectangle that encloses all four points).
 */
void NodePath::
set_scissor(const NodePath &other,
            const LPoint3 &a, const LPoint3 &b,
            const LPoint3 &c, const LPoint3 &d) {
  set_effect(ScissorEffect::make_node(a, b, c, d, other));
}

/**
 * Removes the scissor region that was defined at this node level by a
 * previous call to set_scissor().
 */
void NodePath::
clear_scissor() {
  clear_effect(ScissorEffect::get_class_type());
}

/**
 * Returns true if a scissor region was defined at this node by a previous
 * call to set_scissor().  This does not check for scissor regions inherited
 * from a parent class.  It also does not check for the presence of a low-
 * level ScissorAttrib, which is different from the ScissorEffect added by
 * set_scissor.
 */
bool NodePath::
has_scissor() const {
  return has_effect(ScissorEffect::get_class_type());
}

/**
 * Assigns the geometry at this level and below to the named rendering bin.
 * It is the user's responsibility to ensure that such a bin already exists,
 * either via the cull-bin Configrc variable, or by explicitly creating a
 * GeomBin of the appropriate type at runtime.
 *
 * There are two default bins created when Panda is started: "default" and
 * "fixed".  Normally, all geometry is assigned to "default" unless specified
 * otherwise.  This bin renders opaque geometry in state-sorted order,
 * followed by transparent geometry sorted back-to-front.  If any geometry is
 * assigned to "fixed", this will be rendered following all the geometry in
 * "default", in the order specified by draw_order for each piece of geometry
 * so assigned.
 *
 * The draw_order parameter is meaningful only for GeomBinFixed type bins,
 * e.g.  "fixed".  Other kinds of bins ignore it.
 */
void NodePath::
set_bin(const string &bin_name, int draw_order, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(CullBinAttrib::make(bin_name, draw_order), priority);
}

/**
 * Completely removes any bin adjustment that may have been set via set_bin()
 * from this particular node.
 */
void NodePath::
clear_bin() {
  nassertv_always(!is_empty());
  node()->clear_attrib(CullBinAttrib::get_class_slot());
}

/**
 * Returns true if the node has been assigned to the a particular rendering
 * bin via set_bin(), false otherwise.
 */
bool NodePath::
has_bin() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(CullBinAttrib::get_class_slot());
}

/**
 * Returns the name of the bin that this particular node was assigned to via
 * set_bin(), or the empty string if no bin was assigned.  See set_bin() and
 * has_bin().
 */
string NodePath::
get_bin_name() const {
  nassertr_always(!is_empty(), string());
  const RenderAttrib *attrib =
    node()->get_attrib(CullBinAttrib::get_class_slot());
  if (attrib != nullptr) {
    const CullBinAttrib *ba = DCAST(CullBinAttrib, attrib);
    return ba->get_bin_name();
  }

  return string();
}

/**
 * Returns the drawing order associated with the bin that this particular node
 * was assigned to via set_bin(), or 0 if no bin was assigned.  See set_bin()
 * and has_bin().
 */
int NodePath::
get_bin_draw_order() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(CullBinAttrib::get_class_slot());
  if (attrib != nullptr) {
    const CullBinAttrib *ba = DCAST(CullBinAttrib, attrib);
    return ba->get_draw_order();
  }

  return 0;
}

/**
 * Adds the indicated texture to the list of textures that will be rendered on
 * the default texture stage.
 *
 * This is the convenience single-texture variant of this method; it is now
 * superceded by set_texture() that accepts a stage and texture.  You may use
 * this method if you just want to adjust the default stage.
 */
void NodePath::
set_texture(Texture *tex, int priority) {
  nassertv_always(!is_empty());
  PT(TextureStage) stage = TextureStage::get_default();
  set_texture(stage, tex, priority);
}

/**
 * Adds the indicated texture to the list of textures that will be rendered on
 * the indicated multitexture stage.  If there are multiple texture stages
 * specified (possibly on multiple different nodes at different levels), they
 * will all be applied to geometry together, according to the stage
 * specification set up in the TextureStage object.
 */
void NodePath::
set_texture(TextureStage *stage, Texture *tex, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *tsa = DCAST(TextureAttrib, attrib);
    int sg_priority = node()->get_state()->get_override(TextureAttrib::get_class_slot());

    // Modify the existing TextureAttrib to add the indicated texture.
    node()->set_attrib(tsa->add_on_stage(stage, tex, priority), sg_priority);

  } else {
    // Create a new TextureAttrib for this node.
    CPT(TextureAttrib) tsa = DCAST(TextureAttrib, TextureAttrib::make());
    node()->set_attrib(tsa->add_on_stage(stage, tex, priority));
  }
}

/**
 * Adds the indicated texture to the list of textures that will be rendered on
 * the default texture stage.
 *
 * The given sampler state will override the sampling settings on the texture
 * itself.  Note that this method makes a copy of the sampler settings that
 * you give; further changes to this object will not be reflected.
 *
 * This is the convenience single-texture variant of this method; it is now
 * superceded by set_texture() that accepts a stage and texture.  You may use
 * this method if you just want to adjust the default stage.
 */
void NodePath::
set_texture(Texture *tex, const SamplerState &sampler, int priority) {
  nassertv_always(!is_empty());
  PT(TextureStage) stage = TextureStage::get_default();
  set_texture(stage, tex, sampler, priority);
}

/**
 * Adds the indicated texture to the list of textures that will be rendered on
 * the indicated multitexture stage.  If there are multiple texture stages
 * specified (possibly on multiple different nodes at different levels), they
 * will all be applied to geometry together, according to the stage
 * specification set up in the TextureStage object.
 *
 * The given sampler state will override the sampling settings on the texture
 * itself.  Note that this method makes a copy of the sampler settings that
 * you give; further changes to this object will not be reflected.
 */
void NodePath::
set_texture(TextureStage *stage, Texture *tex, const SamplerState &sampler, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *tsa = DCAST(TextureAttrib, attrib);
    int sg_priority = node()->get_state()->get_override(TextureAttrib::get_class_slot());

    // Modify the existing TextureAttrib to add the indicated texture.
    node()->set_attrib(tsa->add_on_stage(stage, tex, sampler, priority), sg_priority);

  } else {
    // Create a new TextureAttrib for this node.
    CPT(TextureAttrib) tsa = DCAST(TextureAttrib, TextureAttrib::make());
    node()->set_attrib(tsa->add_on_stage(stage, tex, sampler, priority));
  }
}

/**
 * Sets the geometry at this level and below to render using no texture, on
 * any stage.  This is different from not specifying a texture; rather, this
 * specifically contradicts set_texture() at a higher node level (or, with a
 * priority, overrides a set_texture() at a lower level).
 */
void NodePath::
set_texture_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(TextureAttrib::make_all_off(), priority);
}

/**
 * Sets the geometry at this level and below to render using no texture, on
 * the indicated stage.  This is different from not specifying a texture;
 * rather, this specifically contradicts set_texture() at a higher node level
 * (or, with a priority, overrides a set_texture() at a lower level).
 */
void NodePath::
set_texture_off(TextureStage *stage, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *tsa = DCAST(TextureAttrib, attrib);
    int sg_priority = node()->get_state()->get_override(TextureAttrib::get_class_slot());

    // Modify the existing TextureAttrib to add the indicated texture to the
    // "off" list.  This also, incidentally, removes it from the "on" list if
    // it is there.
    node()->set_attrib(tsa->add_off_stage(stage, priority), sg_priority);

  } else {
    // Create a new TextureAttrib for this node that turns off the indicated
    // stage.
    CPT(TextureAttrib) tsa = DCAST(TextureAttrib, TextureAttrib::make());
    node()->set_attrib(tsa->add_off_stage(stage, priority));
  }
}

/**
 * Completely removes any texture adjustment that may have been set via
 * set_texture() or set_texture_off() from this particular node.  This allows
 * whatever textures might be otherwise affecting the geometry to show
 * instead.
 */
void NodePath::
clear_texture() {
  nassertv_always(!is_empty());
  node()->clear_attrib(TextureAttrib::get_class_slot());
}

/**
 * Removes any reference to the indicated texture stage from the NodePath.
 */
void NodePath::
clear_texture(TextureStage *stage) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    CPT(TextureAttrib) tsa = DCAST(TextureAttrib, attrib);
    tsa = DCAST(TextureAttrib, tsa->remove_on_stage(stage));
    tsa = DCAST(TextureAttrib, tsa->remove_off_stage(stage));

    if (tsa->is_identity()) {
      node()->clear_attrib(TextureAttrib::get_class_slot());

    } else {
      int priority = node()->get_state()->get_override(TextureAttrib::get_class_slot());
      node()->set_attrib(tsa, priority);
    }
  }
}

/**
 * Returns true if a texture has been applied to this particular node via
 * set_texture(), false otherwise.  This is not the same thing as asking
 * whether the geometry at this node will be rendered with texturing, as there
 * may be a texture in effect from a higher or lower level.
 */
bool NodePath::
has_texture() const {
  return get_texture() != nullptr;
}

/**
 * Returns true if texturing has been specifically enabled on this particular
 * node for the indicated stage.  This means that someone called set_texture()
 * on this node with the indicated stage name, or the stage_name is the
 * default stage_name, and someone called set_texture() on this node.
 */
bool NodePath::
has_texture(TextureStage *stage) const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->has_on_stage(stage);
  }

  return false;
}

/**
 * Returns true if texturing has been specifically disabled on this particular
 * node via set_texture_off(), false otherwise.  This is not the same thing as
 * asking whether the geometry at this node will be rendered untextured, as
 * there may be a texture in effect from a higher or lower level.
 */
bool NodePath::
has_texture_off() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->has_all_off();
  }

  return false;
}

/**
 * Returns true if texturing has been specifically disabled on this particular
 * node for the indicated stage.  This means that someone called
 * set_texture_off() on this node with the indicated stage name, or that
 * someone called set_texture_off() on this node to remove all stages.
 */
bool NodePath::
has_texture_off(TextureStage *stage) const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->has_off_stage(stage);
  }

  return false;
}

/**
 * Returns the base-level texture that has been set on this particular node,
 * or NULL if no texture has been set.  This is not necessarily the texture
 * that will be applied to the geometry at or below this level, as another
 * texture at a higher or lower level may override.
 *
 * See also find_texture().
 */
Texture *NodePath::
get_texture() const {
  nassertr_always(!is_empty(), nullptr);
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->get_texture();
  }

  return nullptr;
}

/**
 * Returns the texture that has been set on the indicated stage for this
 * particular node, or NULL if no texture has been set for this stage.
 */
Texture *NodePath::
get_texture(TextureStage *stage) const {
  nassertr_always(!is_empty(), nullptr);
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    return ta->get_on_texture(stage);
  }

  return nullptr;
}

/**
 * Recursively searches the scene graph for references to the given texture,
 * and replaces them with the new texture.
 *
 * @since 1.10.4
 */
void NodePath::
replace_texture(Texture *tex, Texture *new_tex) {
  nassertv_always(!is_empty());
  nassertv(tex != nullptr);
  nassertv(new_tex != nullptr);

  r_replace_texture(node(), tex, new_tex);
}

/**
 * Returns the sampler state that has been given for the base-level texture
 * that has been set on this particular node.  If no sampler state was given,
 * this returns the texture's default sampler settings.
 *
 * It is an error to call this if there is no base-level texture applied to
 * this particular node.
 */
const SamplerState &NodePath::
get_texture_sampler() const {
  return get_texture_sampler(TextureStage::get_default());
}

/**
 * Returns the sampler state that has been given for the indicated texture
 * stage that has been set on this particular node.  If no sampler state was
 * given, this returns the texture's default sampler settings.
 *
 * It is an error to call this if there is no texture set for this stage on
 * this particular node.
 */
const SamplerState &NodePath::
get_texture_sampler(TextureStage *stage) const {
  nassertr_always(!is_empty(), SamplerState::get_default());
  const RenderAttrib *attrib =
    node()->get_attrib(TextureAttrib::get_class_slot());
  nassertr_always(attrib != nullptr, SamplerState::get_default());

  const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
  return ta->get_on_sampler(stage);
}

/**
 *
 */
void NodePath::
set_shader(const Shader *sha, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ShaderAttrib::get_class_slot()));
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->set_shader(sha, priority));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    node()->set_attrib(sa->set_shader(sha, priority));
  }
}

/**
 *
 */
void NodePath::
set_shader_off(int priority) {
  set_shader(nullptr, priority);
}

/**
 *
 */
void NodePath::
set_shader_auto(int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ShaderAttrib::get_class_slot()));
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->set_shader_auto(priority));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    node()->set_attrib(sa->set_shader_auto(priority));
  }
}

/**
 * overloaded for auto shader customization
 */
void NodePath::
set_shader_auto(BitMask32 shader_switch, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(ShaderAttrib::get_class_slot()));
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->set_shader_auto(shader_switch, priority));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    node()->set_attrib(sa->set_shader_auto(shader_switch, priority));
  }
}
/**
 *
 */
void NodePath::
clear_shader() {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->clear_shader());
  }
}

/**
 *
 */
const Shader *NodePath::
get_shader() const {
  nassertr_always(!is_empty(), nullptr);
  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    return sa->get_shader();
  }
  return nullptr;
}

/**
 *
 */
void NodePath::
set_shader_input(const ShaderInput &inp) {
  nassertv_always(!is_empty());

  PandaNode *pnode = node();
  const RenderAttrib *attrib =
    pnode->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = (const ShaderAttrib *)attrib;
    pnode->set_attrib(sa->set_shader_input(inp));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    pnode->set_attrib(sa->set_shader_input(inp));
  }
}

/**
 *
 */
void NodePath::
set_shader_input(ShaderInput &&inp) {
  nassertv_always(!is_empty());

  PandaNode *pnode = node();
  const RenderAttrib *attrib =
    pnode->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = (const ShaderAttrib *)attrib;
    pnode->set_attrib(sa->set_shader_input(move(inp)));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    pnode->set_attrib(sa->set_shader_input(move(inp)));
  }
}

/**
 *
 */
ShaderInput NodePath::
get_shader_input(CPT_InternalName id) const {
  nassertr_always(!is_empty(), ShaderInput::get_blank());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = (const ShaderAttrib *)attrib;
    return sa->get_shader_input(id);
  }
  return ShaderInput::get_blank();
}

/**
 * Returns the geometry instance count, or 0 if disabled.  See
 * set_instance_count.
 */
int NodePath::
get_instance_count() const {
  nassertr_always(!is_empty(), 0);

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());

  if (attrib != nullptr) {
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    return sa->get_instance_count();
  }

  return 0;
}

/**
 *
 */
void NodePath::
clear_shader_input(CPT_InternalName id) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->clear_shader_input(id));
  }
}

/**
 * Sets the geometry instance count, or 0 if geometry instancing should be
 * disabled.  Do not confuse with instanceTo which only applies to animation
 * instancing.
 */
void NodePath::
set_instance_count(int instance_count) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(ShaderAttrib::get_class_slot());
  if (attrib != nullptr) {
    const ShaderAttrib *sa = DCAST(ShaderAttrib, attrib);
    node()->set_attrib(sa->set_instance_count(instance_count));
  } else {
    // Create a new ShaderAttrib for this node.
    CPT(ShaderAttrib) sa = DCAST(ShaderAttrib, ShaderAttrib::make());
    node()->set_attrib(sa->set_instance_count(instance_count));
  }
}

/**
 * Sets the texture matrix on the current node to the indicated transform for
 * the given stage.
 */
void NodePath::
set_tex_transform(TextureStage *stage, const TransformState *transform) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attrib);

    // Modify the existing TexMatrixAttrib to add the indicated stage.
    node()->set_attrib(tma->add_stage(stage, transform));

  } else {
    // Create a new TexMatrixAttrib for this node.
    node()->set_attrib(TexMatrixAttrib::make(stage, transform));
  }
}

/**
 * Removes all texture matrices from the current node.
 */
void NodePath::
clear_tex_transform() {
  nassertv_always(!is_empty());
  node()->clear_attrib(TexMatrixAttrib::get_class_slot());
}

/**
 * Removes the texture matrix on the current node for the given stage.
 */
void NodePath::
clear_tex_transform(TextureStage *stage) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    CPT(TexMatrixAttrib) tma = DCAST(TexMatrixAttrib, attrib);
    tma = DCAST(TexMatrixAttrib, tma->remove_stage(stage));

    if (tma->is_empty()) {
      node()->clear_attrib(TexMatrixAttrib::get_class_slot());

    } else {
      node()->set_attrib(tma);
    }
  }
}

/**
 * Returns true if there is an explicit texture matrix on the current node for
 * the given stage.
 */
bool NodePath::
has_tex_transform(TextureStage *stage) const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attrib);
    return tma->has_stage(stage);
  }

  return false;
}

/**
 * Returns the texture matrix on the current node for the given stage, or
 * identity transform if there is no explicit transform set for the given
 * stage.
 */
CPT(TransformState) NodePath::
get_tex_transform(TextureStage *stage) const {
  nassertr_always(!is_empty(), nullptr);

  const RenderAttrib *attrib =
    node()->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attrib);
    return tma->get_transform(stage);
  }

  return TransformState::make_identity();
}

/**
 * Sets the texture matrix on the current node to the indicated transform for
 * the given stage.
 */
void NodePath::
set_tex_transform(const NodePath &other, TextureStage *stage, const TransformState *transform) {
  nassertv(_error_type == ET_ok && other._error_type == ET_ok);
  nassertv_always(!is_empty());

  CPT(RenderState) state = get_state(other);
  const RenderAttrib *attrib =
    state->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attrib);

    // Modify the existing TexMatrixAttrib to add the indicated stage.
    state = state->add_attrib(tma->add_stage(stage, transform));

  } else {
    // Create a new TexMatrixAttrib for this node.
    state = state->add_attrib(TexMatrixAttrib::make(stage, transform));
  }

  // Now compose that with our parent's state.
  CPT(RenderState) rel_state;
  if (has_parent()) {
    rel_state = other.get_state(get_parent());
  } else {
    rel_state = other.get_state(NodePath());
  }
  CPT(RenderState) new_state = rel_state->compose(state);

  // And apply only the TexMatrixAttrib to the current node, leaving the
  // others unchanged.
  node()->set_attrib(new_state->get_attrib(TexMatrixAttrib::get_class_slot()));
}

/**
 * Returns the texture matrix on the current node for the given stage,
 * relative to the other node.
 */
CPT(TransformState) NodePath::
get_tex_transform(const NodePath &other, TextureStage *stage) const {
  nassertr(_error_type == ET_ok && other._error_type == ET_ok, TransformState::make_identity());

  CPT(RenderState) state = get_state(other);
  const RenderAttrib *attrib =
    state->get_attrib(TexMatrixAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attrib);
    return tma->get_transform(stage);
  }

  return TransformState::make_identity();
}

/**
 * Enables automatic texture coordinate generation for the indicated texture
 * stage.
 */
void NodePath::
set_tex_gen(TextureStage *stage, RenderAttrib::TexGenMode mode, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TexGenAttrib::get_class_slot());

  CPT(TexGenAttrib) tga;

  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(TextureAttrib::get_class_slot()));
    tga = DCAST(TexGenAttrib, attrib);

  } else {
    tga = DCAST(TexGenAttrib, TexGenAttrib::make());
  }

  node()->set_attrib(tga->add_stage(stage, mode), priority);
}

/**
 * Enables automatic texture coordinate generation for the indicated texture
 * stage.  This version of this method is useful when setting M_constant,
 * which requires a constant texture coordinate value.
 */
void NodePath::
set_tex_gen(TextureStage *stage, RenderAttrib::TexGenMode mode,
            const LTexCoord3 &constant_value, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TexGenAttrib::get_class_slot());

  CPT(TexGenAttrib) tga;

  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(TextureAttrib::get_class_slot()));
    tga = DCAST(TexGenAttrib, attrib);

  } else {
    tga = DCAST(TexGenAttrib, TexGenAttrib::make());
  }

  node()->set_attrib(tga->add_stage(stage, mode, constant_value), priority);
}

/**
 * Removes the texture coordinate generation mode from all texture stages on
 * this node.
 */
void NodePath::
clear_tex_gen() {
  nassertv_always(!is_empty());
  node()->clear_attrib(TexGenAttrib::get_class_slot());
}

/**
 * Disables automatic texture coordinate generation for the indicated texture
 * stage.
 */
void NodePath::
clear_tex_gen(TextureStage *stage) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(TexGenAttrib::get_class_slot());
  if (attrib != nullptr) {
    CPT(TexGenAttrib) tga = DCAST(TexGenAttrib, attrib);
    tga = DCAST(TexGenAttrib, tga->remove_stage(stage));

    if (tga->is_empty()) {
      node()->clear_attrib(TexGenAttrib::get_class_slot());

    } else {
      node()->set_attrib(tga);
    }
  }
}

/**
 * Returns true if there is a mode for automatic texture coordinate generation
 * on the current node for the given stage.
 */
bool NodePath::
has_tex_gen(TextureStage *stage) const {
  nassertr_always(!is_empty(), false);

  const RenderAttrib *attrib =
    node()->get_attrib(TexGenAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexGenAttrib *tga = DCAST(TexGenAttrib, attrib);
    return tga->has_stage(stage);
  }

  return false;
}

/**
 * Returns the texture coordinate generation mode for the given stage, or
 * M_off if there is no explicit mode set for the given stage.
 */
RenderAttrib::TexGenMode NodePath::
get_tex_gen(TextureStage *stage) const {
  nassertr_always(!is_empty(), TexGenAttrib::M_off);

  const RenderAttrib *attrib =
    node()->get_attrib(TexGenAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TexGenAttrib *tga = DCAST(TexGenAttrib, attrib);
    return tga->get_mode(stage);
  }

  return TexGenAttrib::M_off;
}

/**
 * Establishes a TexProjectorEffect on this node, which can be used to
 * establish projective texturing (but see also the
 * NodePath::project_texture() convenience function), or it can be used to
 * bind this node's texture transform to particular node's position in space,
 * allowing a LerpInterval (for instance) to adjust this node's texture
 * coordinates.
 *
 * If to is a LensNode, then the fourth parameter, lens_index, can be provided
 * to select a particular lens to apply.  Otherwise lens_index is not used.
 */
void NodePath::
set_tex_projector(TextureStage *stage, const NodePath &from, const NodePath &to,
                  int lens_index) {
  nassertv_always(!is_empty());

  const RenderEffect *effect =
    node()->get_effect(TexProjectorEffect::get_class_type());

  CPT(TexProjectorEffect) tpe;

  if (effect != nullptr) {
    tpe = DCAST(TexProjectorEffect, effect);

  } else {
    tpe = DCAST(TexProjectorEffect, TexProjectorEffect::make());
  }

  node()->set_effect(tpe->add_stage(stage, from, to, lens_index));
}

/**
 * Removes the TexProjectorEffect for the indicated stage from this node.
 */
void NodePath::
clear_tex_projector(TextureStage *stage) {
  nassertv_always(!is_empty());

  const RenderEffect *effect =
    node()->get_effect(TexProjectorEffect::get_class_type());
  if (effect != nullptr) {
    CPT(TexProjectorEffect) tpe = DCAST(TexProjectorEffect, effect);
    tpe = DCAST(TexProjectorEffect, tpe->remove_stage(stage));

    if (tpe->is_empty()) {
      node()->clear_effect(TexProjectorEffect::get_class_type());

    } else {
      node()->set_effect(tpe);
    }
  }
}

/**
 * Removes the TexProjectorEffect for all stages from this node.
 */
void NodePath::
clear_tex_projector() {
  nassertv_always(!is_empty());
  node()->clear_effect(TexProjectorEffect::get_class_type());
}

/**
 * Returns true if this node has a TexProjectorEffect for the indicated stage,
 * false otherwise.
 */
bool NodePath::
has_tex_projector(TextureStage *stage) const {
  nassertr_always(!is_empty(), false);

  const RenderEffect *effect =
    node()->get_effect(TexProjectorEffect::get_class_type());
  if (effect != nullptr) {
    const TexProjectorEffect *tpe = DCAST(TexProjectorEffect, effect);
    return tpe->has_stage(stage);
  }

  return false;
}

/**
 * Returns the "from" node associated with the TexProjectorEffect on the
 * indicated stage.  The relative transform between the "from" and the "to"
 * nodes is automatically applied to the texture transform each frame.
 */
NodePath NodePath::
get_tex_projector_from(TextureStage *stage) const {
  nassertr_always(!is_empty(), NodePath::fail());

  const RenderEffect *effect =
    node()->get_effect(TexProjectorEffect::get_class_type());
  if (effect != nullptr) {
    const TexProjectorEffect *tpe = DCAST(TexProjectorEffect, effect);
    return tpe->get_from(stage);
  }

  return NodePath::not_found();
}

/**
 * Returns the "to" node associated with the TexProjectorEffect on the
 * indicated stage.  The relative transform between the "from" and the "to"
 * nodes is automatically applied to the texture transform each frame.
 */
NodePath NodePath::
get_tex_projector_to(TextureStage *stage) const {
  nassertr_always(!is_empty(), NodePath::fail());

  const RenderEffect *effect =
    node()->get_effect(TexProjectorEffect::get_class_type());
  if (effect != nullptr) {
    const TexProjectorEffect *tpe = DCAST(TexProjectorEffect, effect);
    return tpe->get_to(stage);
  }

  return NodePath::not_found();
}

/**
 * A convenience function to enable projective texturing at this node level
 * and below, using the indicated NodePath (which should contain a LensNode)
 * as the projector.
 */
void NodePath::
project_texture(TextureStage *stage, Texture *tex, const NodePath &projector) {
  nassertv(!projector.is_empty() && projector.node()->is_of_type(LensNode::get_class_type()));
  set_texture(stage, tex);
  set_tex_gen(stage, TexGenAttrib::M_world_position);
  set_tex_projector(stage, NodePath(), projector);
}

/**
 * Returns true if there are at least some vertices at this node and below
 * that contain a reference to the indicated vertex data column name, false
 * otherwise.
 *
 * This is particularly useful for testing whether a particular model has a
 * given texture coordinate set (but see has_texcoord()).
 */
bool NodePath::
has_vertex_column(const InternalName *name) const {
  nassertr_always(!is_empty(), false);
  return r_has_vertex_column(node(), name);
}

/**
 * Returns a list of all vertex array columns stored on some geometry found at
 * this node level and below.
 */
InternalNameCollection NodePath::
find_all_vertex_columns() const {
  nassertr_always(!is_empty(), InternalNameCollection());
  InternalNames vertex_columns;
  r_find_all_vertex_columns(node(), vertex_columns);

  InternalNameCollection tc;
  InternalNames::iterator ti;
  for (ti = vertex_columns.begin(); ti != vertex_columns.end(); ++ti) {
    tc.add_name(*ti);
  }
  return tc;
}

/**
 * Returns a list of all vertex array columns stored on some geometry found at
 * this node level and below that match the indicated name (which may contain
 * wildcard characters).
 */
InternalNameCollection NodePath::
find_all_vertex_columns(const string &name) const {
  nassertr_always(!is_empty(), InternalNameCollection());
  InternalNames vertex_columns;
  r_find_all_vertex_columns(node(), vertex_columns);

  GlobPattern glob(name);

  InternalNameCollection tc;
  InternalNames::iterator ti;
  for (ti = vertex_columns.begin(); ti != vertex_columns.end(); ++ti) {
    const InternalName *name = (*ti);
    if (glob.matches(name->get_name())) {
      tc.add_name(name);
    }
  }
  return tc;
}

/**
 * Returns a list of all texture coordinate sets used by any geometry at this
 * node level and below.
 */
InternalNameCollection NodePath::
find_all_texcoords() const {
  nassertr_always(!is_empty(), InternalNameCollection());
  InternalNames vertex_columns;
  r_find_all_vertex_columns(node(), vertex_columns);

  CPT(InternalName) texcoord_name = InternalName::get_texcoord();

  InternalNameCollection tc;
  InternalNames::iterator ti;
  for (ti = vertex_columns.begin(); ti != vertex_columns.end(); ++ti) {
    if ((*ti)->get_top() == texcoord_name) {
      tc.add_name(*ti);
    }
  }
  return tc;
}

/**
 * Returns a list of all texture coordinate sets used by any geometry at this
 * node level and below that match the indicated name (which may contain
 * wildcard characters).
 */
InternalNameCollection NodePath::
find_all_texcoords(const string &name) const {
  nassertr_always(!is_empty(), InternalNameCollection());
  InternalNames vertex_columns;
  r_find_all_vertex_columns(node(), vertex_columns);

  GlobPattern glob(name);
  CPT_InternalName texcoord_name = InternalName::get_texcoord();

  InternalNameCollection tc;
  InternalNames::iterator ti;
  for (ti = vertex_columns.begin(); ti != vertex_columns.end(); ++ti) {
    const InternalName *name = (*ti);
    if (name->get_top() == texcoord_name) {
      // This is a texture coordinate name.  Figure out the basename of the
      // texture coordinates.
      int index = name->find_ancestor("texcoord");
      nassertr(index != -1, InternalNameCollection());
      string net_basename = name->get_net_basename(index - 1);

      if (glob.matches(net_basename)) {
        tc.add_name(name);
      }
    }
  }
  return tc;
}

/**
 * Returns the first texture found applied to geometry at this node or below
 * that matches the indicated name (which may contain wildcards).  Returns the
 * texture if it is found, or NULL if it is not.
 */
Texture *NodePath::
find_texture(const string &name) const {
  nassertr_always(!is_empty(), nullptr);
  GlobPattern glob(name);
  return r_find_texture(node(), get_net_state(), glob);
}

/**
 * Returns the first texture found applied to geometry at this node or below
 * that is assigned to the indicated texture stage.  Returns the texture if it
 * is found, or NULL if it is not.
 */
Texture *NodePath::
find_texture(TextureStage *stage) const {
  nassertr_always(!is_empty(), nullptr);
  return r_find_texture(node(), stage);
}

/**
 * Returns a list of a textures applied to geometry at this node and below.
 */
TextureCollection NodePath::
find_all_textures() const {
  nassertr_always(!is_empty(), TextureCollection());
  Textures textures;
  r_find_all_textures(node(), get_net_state(), textures);

  TextureCollection tc;
  Textures::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    tc.add_texture(*ti);
  }
  return tc;
}

/**
 * Returns a list of a textures applied to geometry at this node and below
 * that match the indicated name (which may contain wildcard characters).
 */
TextureCollection NodePath::
find_all_textures(const string &name) const {
  nassertr_always(!is_empty(), TextureCollection());
  Textures textures;
  r_find_all_textures(node(), get_net_state(), textures);

  GlobPattern glob(name);

  TextureCollection tc;
  Textures::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    Texture *texture = (*ti);
    if (glob.matches(texture->get_name())) {
      tc.add_texture(texture);
    }
  }
  return tc;
}

/**
 * Returns a list of a textures on geometry at this node and below that are
 * assigned to the indicated texture stage.
 */
TextureCollection NodePath::
find_all_textures(TextureStage *stage) const {
  nassertr_always(!is_empty(), TextureCollection());
  Textures textures;
  r_find_all_textures(node(), stage, textures);

  TextureCollection tc;
  Textures::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    Texture *texture = (*ti);
    tc.add_texture(texture);
  }
  return tc;
}

/**
 * Returns the first TextureStage found applied to geometry at this node or
 * below that matches the indicated name (which may contain wildcards).
 * Returns the TextureStage if it is found, or NULL if it is not.
 */
TextureStage *NodePath::
find_texture_stage(const string &name) const {
  nassertr_always(!is_empty(), nullptr);
  GlobPattern glob(name);
  return r_find_texture_stage(node(), get_net_state(), glob);
}

/**
 * Returns a list of a TextureStages applied to geometry at this node and
 * below.
 */
TextureStageCollection NodePath::
find_all_texture_stages() const {
  nassertr_always(!is_empty(), TextureStageCollection());
  TextureStages texture_stages;
  r_find_all_texture_stages(node(), get_net_state(), texture_stages);

  TextureStageCollection tc;
  TextureStages::iterator ti;
  for (ti = texture_stages.begin(); ti != texture_stages.end(); ++ti) {
    tc.add_texture_stage(*ti);
  }
  return tc;
}

/**
 * Searches through all TextureStages at this node and below.  Any
 * TextureStages that share the same name as the indicated TextureStage object
 * are replaced with this object, thus ensuring that all geometry at this node
 * and below with a particular TextureStage name is using the same
 * TextureStage object.
 */
void NodePath::
unify_texture_stages(TextureStage *stage) {
  nassertv_always(!is_empty());
  r_unify_texture_stages(node(), stage);
}

/**
 * Returns a list of a TextureStages applied to geometry at this node and
 * below that match the indicated name (which may contain wildcard
 * characters).
 */
TextureStageCollection NodePath::
find_all_texture_stages(const string &name) const {
  nassertr_always(!is_empty(), TextureStageCollection());
  TextureStages texture_stages;
  r_find_all_texture_stages(node(), get_net_state(), texture_stages);

  GlobPattern glob(name);

  TextureStageCollection tc;
  TextureStages::iterator ti;
  for (ti = texture_stages.begin(); ti != texture_stages.end(); ++ti) {
    TextureStage *texture_stage = (*ti);
    if (glob.matches(texture_stage->get_name())) {
      tc.add_texture_stage(texture_stage);
    }
  }
  return tc;
}

/**
 * Returns the first material found applied to geometry at this node or below
 * that matches the indicated name (which may contain wildcards).  Returns the
 * material if it is found, or NULL if it is not.
 */
Material *NodePath::
find_material(const string &name) const {
  nassertr_always(!is_empty(), nullptr);
  GlobPattern glob(name);
  return r_find_material(node(), get_net_state(), glob);
}

/**
 * Returns a list of a materials applied to geometry at this node and below.
 */
MaterialCollection NodePath::
find_all_materials() const {
  nassertr_always(!is_empty(), MaterialCollection());
  Materials materials;
  r_find_all_materials(node(), get_net_state(), materials);

  MaterialCollection tc;
  Materials::iterator ti;
  for (ti = materials.begin(); ti != materials.end(); ++ti) {
    tc.add_material(*ti);
  }
  return tc;
}

/**
 * Returns a list of a materials applied to geometry at this node and below
 * that match the indicated name (which may contain wildcard characters).
 */
MaterialCollection NodePath::
find_all_materials(const string &name) const {
  nassertr_always(!is_empty(), MaterialCollection());
  Materials materials;
  r_find_all_materials(node(), get_net_state(), materials);

  GlobPattern glob(name);

  MaterialCollection tc;
  Materials::iterator ti;
  for (ti = materials.begin(); ti != materials.end(); ++ti) {
    Material *material = (*ti);
    if (glob.matches(material->get_name())) {
      tc.add_material(material);
    }
  }
  return tc;
}

/**
 * Sets the geometry at this level and below to render using the indicated
 * material.
 *
 * Previously, this operation made a copy of the material structure, but
 * nowadays it assigns the pointer directly.
 */
void NodePath::
set_material(Material *mat, int priority) {
  nassertv_always(!is_empty());
  nassertv(mat != nullptr);
  node()->set_attrib(MaterialAttrib::make(mat), priority);
}

/**
 * Sets the geometry at this level and below to render using no material.
 * This is normally the default, but it may be useful to use this to
 * contradict set_material() at a higher node level (or, with a priority, to
 * override a set_material() at a lower level).
 */
void NodePath::
set_material_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(MaterialAttrib::make_off(), priority);
}

/**
 * Completely removes any material adjustment that may have been set via
 * set_material() from this particular node.
 */
void NodePath::
clear_material() {
  nassertv_always(!is_empty());
  node()->clear_attrib(MaterialAttrib::get_class_slot());
}

/**
 * Returns true if a material has been applied to this particular node via
 * set_material(), false otherwise.
 */
bool NodePath::
has_material() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(MaterialAttrib::get_class_slot());
  if (attrib != nullptr) {
    const MaterialAttrib *ma = DCAST(MaterialAttrib, attrib);
    return !ma->is_off();
  }

  return false;
}

/**
 * Returns the material that has been set on this particular node, or NULL if
 * no material has been set.  This is not necessarily the material that will
 * be applied to the geometry at or below this level, as another material at a
 * higher or lower level may override.
 *
 * See also find_material().
 */
PT(Material) NodePath::
get_material() const {
  nassertr_always(!is_empty(), nullptr);
  const RenderAttrib *attrib =
    node()->get_attrib(MaterialAttrib::get_class_slot());
  if (attrib != nullptr) {
    const MaterialAttrib *ma = DCAST(MaterialAttrib, attrib);
    return ma->get_material();
  }

  return nullptr;
}

/**
 * Recursively searches the scene graph for references to the given material,
 * and replaces them with the new material.
 */
void NodePath::
replace_material(Material *mat, Material *new_mat) {
  nassertv_always(!is_empty());
  nassertv(mat != nullptr);
  nassertv(new_mat != nullptr);

  CPT(RenderAttrib) new_attrib = MaterialAttrib::make(new_mat);
  r_replace_material(node(), mat, (const MaterialAttrib *)new_attrib.p());
}

/**
 * Sets the geometry at this level and below to render using the indicated
 * fog.
 */
void NodePath::
set_fog(Fog *fog, int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(FogAttrib::make(fog), priority);
}

/**
 * Sets the geometry at this level and below to render using no fog.  This is
 * normally the default, but it may be useful to use this to contradict
 * set_fog() at a higher node level (or, with a priority, to override a
 * set_fog() at a lower level).
 */
void NodePath::
set_fog_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(FogAttrib::make_off(), priority);
}

/**
 * Completely removes any fog adjustment that may have been set via set_fog()
 * or set_fog_off() from this particular node.  This allows whatever fogs
 * might be otherwise affecting the geometry to show instead.
 */
void NodePath::
clear_fog() {
  nassertv_always(!is_empty());
  node()->clear_attrib(FogAttrib::get_class_slot());
}

/**
 * Returns true if a fog has been applied to this particular node via
 * set_fog(), false otherwise.  This is not the same thing as asking whether
 * the geometry at this node will be rendered with fog, as there may be a fog
 * in effect from a higher or lower level.
 */
bool NodePath::
has_fog() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(FogAttrib::get_class_slot());
  if (attrib != nullptr) {
    const FogAttrib *fa = DCAST(FogAttrib, attrib);
    return !fa->is_off();
  }

  return false;
}

/**
 * Returns true if a fog has been specifically disabled on this particular
 * node via set_fog_off(), false otherwise.  This is not the same thing as
 * asking whether the geometry at this node will be rendered unfogged, as
 * there may be a fog in effect from a higher or lower level.
 */
bool NodePath::
has_fog_off() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(FogAttrib::get_class_slot());
  if (attrib != nullptr) {
    const FogAttrib *fa = DCAST(FogAttrib, attrib);
    return fa->is_off();
  }

  return false;
}

/**
 * Returns the fog that has been set on this particular node, or NULL if no
 * fog has been set.  This is not necessarily the fog that will be applied to
 * the geometry at or below this level, as another fog at a higher or lower
 * level may override.
 */
Fog *NodePath::
get_fog() const {
  nassertr_always(!is_empty(), nullptr);
  const RenderAttrib *attrib =
    node()->get_attrib(FogAttrib::get_class_slot());
  if (attrib != nullptr) {
    const FogAttrib *fa = DCAST(FogAttrib, attrib);
    return fa->get_fog();
  }

  return nullptr;
}

/**
 * Sets up the geometry at this level and below (unless overridden) to render
 * in wireframe mode.
 */
void NodePath::
set_render_mode_wireframe(int priority) {
  nassertv_always(!is_empty());
  const RenderModeAttrib *rma;
  node()->get_state()->get_attrib_def(rma);
  node()->set_attrib(RenderModeAttrib::make(RenderModeAttrib::M_wireframe, rma->get_thickness(), rma->get_perspective()), priority);
}

/**
 * Sets up the geometry at this level and below (unless overridden) to render
 * in filled (i.e.  not wireframe) mode.
 */
void NodePath::
set_render_mode_filled(int priority) {
  nassertv_always(!is_empty());
  const RenderModeAttrib *rma;
  node()->get_state()->get_attrib_def(rma);
  node()->set_attrib(RenderModeAttrib::make(RenderModeAttrib::M_filled, rma->get_thickness(), rma->get_perspective()), priority);
}

/**
 * Sets up the geometry at this level and below (unless overridden) to render
 * in filled, but overlay the wireframe on top with a fixed color.  This is
 * useful for debug visualizations.
 */
void NodePath::
set_render_mode_filled_wireframe(const LColor &wireframe_color, int priority) {
  nassertv_always(!is_empty());
  const RenderModeAttrib *rma;
  node()->get_state()->get_attrib_def(rma);
  node()->set_attrib(RenderModeAttrib::make(RenderModeAttrib::M_filled_wireframe, rma->get_thickness(), rma->get_perspective(), wireframe_color), priority);
}

/**
 * Sets up the point geometry at this level and below to render as perspective
 * sprites (that is, billboarded quads).  The thickness, as specified with
 * set_render_mode_thickness(), is the width of each point in 3-D units,
 * unless it is overridden on a per-vertex basis.  This does not affect
 * geometry other than points.
 *
 * If you want the quads to be individually textured, you should also set a
 * TexGenAttrib::M_point_sprite on the node.
 */
void NodePath::
set_render_mode_perspective(bool perspective, int priority) {
  nassertv_always(!is_empty());
  const RenderModeAttrib *rma;
  node()->get_state()->get_attrib_def(rma);
  node()->set_attrib(RenderModeAttrib::make(rma->get_mode(), rma->get_thickness(), perspective, rma->get_wireframe_color()), priority);
}

/**
 * Sets up the point geometry at this level and below to render as thick
 * points (that is, billboarded quads).  The thickness is in pixels, unless
 * set_render_mode_perspective is also true, in which case it is in 3-D units.
 *
 * If you want the quads to be individually textured, you should also set a
 * TexGenAttrib::M_point_sprite on the node.
 */
void NodePath::
set_render_mode_thickness(PN_stdfloat thickness, int priority) {
  nassertv_always(!is_empty());
  const RenderModeAttrib *rma;
  node()->get_state()->get_attrib_def(rma);
  node()->set_attrib(RenderModeAttrib::make(rma->get_mode(), thickness, rma->get_perspective(), rma->get_wireframe_color()), priority);
}

/**
 * Sets up the geometry at this level and below (unless overridden) to render
 * in the specified mode and with the indicated line and/or point thickness.
 */
void NodePath::
set_render_mode(RenderModeAttrib::Mode mode, PN_stdfloat thickness, int priority) {
  nassertv_always(!is_empty());

  node()->set_attrib(RenderModeAttrib::make(mode, thickness), priority);
}

/**
 * Completely removes any render mode adjustment that may have been set on
 * this node via set_render_mode_wireframe() or set_render_mode_filled().
 */
void NodePath::
clear_render_mode() {
  nassertv_always(!is_empty());
  node()->clear_attrib(RenderModeAttrib::get_class_slot());
}

/**
 * Returns true if a render mode has been explicitly set on this particular
 * node via set_render_mode() (or set_render_mode_wireframe() or
 * set_render_mode_filled()), false otherwise.
 */
bool NodePath::
has_render_mode() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(RenderModeAttrib::get_class_slot());
}

/**
 * Returns the render mode that has been specifically set on this node via
 * set_render_mode(), or M_unchanged if nothing has been set.
 */
RenderModeAttrib::Mode NodePath::
get_render_mode() const {
  nassertr_always(!is_empty(), RenderModeAttrib::M_unchanged);
  const RenderAttrib *attrib =
    node()->get_attrib(RenderModeAttrib::get_class_slot());
  if (attrib != nullptr) {
    const RenderModeAttrib *ta = DCAST(RenderModeAttrib, attrib);
    return ta->get_mode();
  }

  return RenderModeAttrib::M_unchanged;
}

/**
 * Returns the render mode thickness that has been specifically set on this
 * node via set_render_mode(), or 1.0 if nothing has been set.
 */
PN_stdfloat NodePath::
get_render_mode_thickness() const {
  nassertr_always(!is_empty(), 0.0f);
  const RenderAttrib *attrib =
    node()->get_attrib(RenderModeAttrib::get_class_slot());
  if (attrib != nullptr) {
    const RenderModeAttrib *ta = DCAST(RenderModeAttrib, attrib);
    return ta->get_thickness();
  }

  return 1.0f;
}

/**
 * Returns the flag that has been set on this node via
 * set_render_mode_perspective(), or false if no flag has been set.
 */
bool NodePath::
get_render_mode_perspective() const {
  nassertr_always(!is_empty(), 0.0f);
  const RenderAttrib *attrib =
    node()->get_attrib(RenderModeAttrib::get_class_slot());
  if (attrib != nullptr) {
    const RenderModeAttrib *ta = DCAST(RenderModeAttrib, attrib);
    return ta->get_perspective();
  }

  return false;
}

/**
 * Specifically sets or disables two-sided rendering mode on this particular
 * node.  If no other nodes override, this will cause backfacing polygons to
 * be drawn (in two-sided mode, true) or culled (in one-sided mode, false).
 */
void NodePath::
set_two_sided(bool two_sided, int priority) {
  nassertv_always(!is_empty());

  CullFaceAttrib::Mode mode =
    two_sided ?
    CullFaceAttrib::M_cull_none :
    CullFaceAttrib::M_cull_clockwise;

  node()->set_attrib(CullFaceAttrib::make(mode), priority);
}

/**
 * Completely removes any two-sided adjustment that may have been set on this
 * node via set_two_sided(). The geometry at this level and below will
 * subsequently be rendered either two-sided or one-sided, according to
 * whatever other nodes may have had set_two_sided() on it, or according to
 * the initial state otherwise.
 */
void NodePath::
clear_two_sided() {
  nassertv_always(!is_empty());
  node()->clear_attrib(CullFaceAttrib::get_class_slot());
}

/**
 * Returns true if a two-sided adjustment has been explicitly set on this
 * particular node via set_two_sided().  If this returns true, then
 * get_two_sided() may be called to determine which has been set.
 */
bool NodePath::
has_two_sided() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(CullFaceAttrib::get_class_slot());
}

/**
 * Returns true if two-sided rendering has been specifically set on this node
 * via set_two_sided(), or false if one-sided rendering has been specifically
 * set, or if nothing has been specifically set.  See also has_two_sided().
 * This does not necessarily imply that the geometry will or will not be
 * rendered two-sided, as there may be other nodes that override.
 */
bool NodePath::
get_two_sided() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(CullFaceAttrib::get_class_slot());
  if (attrib != nullptr) {
    const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, attrib);
    return (cfa->get_actual_mode() == CullFaceAttrib::M_cull_none);
  }

  return false;
}

/**
 * Specifically sets or disables the testing of the depth buffer on this
 * particular node.  This is normally on in the 3-d scene graph and off in the
 * 2-d scene graph; it should be on for rendering most 3-d objects properly.
 */
void NodePath::
set_depth_test(bool depth_test, int priority) {
  nassertv_always(!is_empty());

  DepthTestAttrib::PandaCompareFunc mode =
    depth_test ?
    DepthTestAttrib::M_less :
    DepthTestAttrib::M_none;

  node()->set_attrib(DepthTestAttrib::make(mode), priority);
}

/**
 * Completely removes any depth-test adjustment that may have been set on this
 * node via set_depth_test().
 */
void NodePath::
clear_depth_test() {
  nassertv_always(!is_empty());
  node()->clear_attrib(DepthTestAttrib::get_class_slot());
}

/**
 * Returns true if a depth-test adjustment has been explicitly set on this
 * particular node via set_depth_test().  If this returns true, then
 * get_depth_test() may be called to determine which has been set.
 */
bool NodePath::
has_depth_test() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(DepthTestAttrib::get_class_slot());
}

/**
 * Returns true if depth-test rendering has been specifically set on this node
 * via set_depth_test(), or false if depth-test rendering has been
 * specifically disabled.  If nothing has been specifically set, returns true.
 * See also has_depth_test().
 */
bool NodePath::
get_depth_test() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(DepthTestAttrib::get_class_slot());
  if (attrib != nullptr) {
    const DepthTestAttrib *dta = DCAST(DepthTestAttrib, attrib);
    return (dta->get_mode() != DepthTestAttrib::M_none);
  }

  return true;
}

/**
 * Specifically sets or disables the writing to the depth buffer on this
 * particular node.  This is normally on in the 3-d scene graph and off in the
 * 2-d scene graph; it should be on for rendering most 3-d objects properly.
 */
void NodePath::
set_depth_write(bool depth_write, int priority) {
  nassertv_always(!is_empty());

  DepthWriteAttrib::Mode mode =
    depth_write ?
    DepthWriteAttrib::M_on :
    DepthWriteAttrib::M_off;

  node()->set_attrib(DepthWriteAttrib::make(mode), priority);
}

/**
 * Completely removes any depth-write adjustment that may have been set on
 * this node via set_depth_write().
 */
void NodePath::
clear_depth_write() {
  nassertv_always(!is_empty());
  node()->clear_attrib(DepthWriteAttrib::get_class_slot());
}

/**
 * Returns true if a depth-write adjustment has been explicitly set on this
 * particular node via set_depth_write().  If this returns true, then
 * get_depth_write() may be called to determine which has been set.
 */
bool NodePath::
has_depth_write() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(DepthWriteAttrib::get_class_slot());
}

/**
 * Returns true if depth-write rendering has been specifically set on this
 * node via set_depth_write(), or false if depth-write rendering has been
 * specifically disabled.  If nothing has been specifically set, returns true.
 * See also has_depth_write().
 */
bool NodePath::
get_depth_write() const {
  nassertr_always(!is_empty(), false);
  const RenderAttrib *attrib =
    node()->get_attrib(DepthWriteAttrib::get_class_slot());
  if (attrib != nullptr) {
    const DepthWriteAttrib *dta = DCAST(DepthWriteAttrib, attrib);
    return (dta->get_mode() != DepthWriteAttrib::M_off);
  }

  return true;
}

/**
 * This instructs the graphics driver to apply an offset or bias to the
 * generated depth values for rendered polygons, before they are written to
 * the depth buffer.  This can be used to shift polygons forward slightly, to
 * resolve depth conflicts, or self-shadowing artifacts on thin objects.  The
 * bias is always an integer number, and each integer increment represents the
 * smallest possible increment in Z that is sufficient to completely resolve
 * two coplanar polygons.  Positive numbers are closer towards the camera.
 */
void NodePath::
set_depth_offset(int bias, int priority) {
  nassertv_always(!is_empty());

  node()->set_attrib(DepthOffsetAttrib::make(bias), priority);
}

/**
 * Completely removes any depth-offset adjustment that may have been set on
 * this node via set_depth_offset().
 */
void NodePath::
clear_depth_offset() {
  nassertv_always(!is_empty());
  node()->clear_attrib(DepthOffsetAttrib::get_class_slot());
}

/**
 * Returns true if a depth-offset adjustment has been explicitly set on this
 * particular node via set_depth_offset().  If this returns true, then
 * get_depth_offset() may be called to determine which has been set.
 */
bool NodePath::
has_depth_offset() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(DepthOffsetAttrib::get_class_slot());
}

/**
 * Returns the depth offset value if it has been specified using
 * set_depth_offset, or 0 if not.
 */
int NodePath::
get_depth_offset() const {
  nassertr_always(!is_empty(), 0);
  const RenderAttrib *attrib =
    node()->get_attrib(DepthOffsetAttrib::get_class_slot());
  if (attrib != nullptr) {
    const DepthOffsetAttrib *doa = DCAST(DepthOffsetAttrib, attrib);
    return doa->get_offset();
  }

  return 0;
}

/**
 * Performs a billboard-type rotate to the indicated camera node, one time
 * only, and leaves the object rotated.  This is similar in principle to
 * heads_up().
 */
void NodePath::
do_billboard_axis(const NodePath &camera, PN_stdfloat offset) {
  nassertv_always(!is_empty());

  CPT(TransformState) transform = camera.get_transform(get_parent());
  const LMatrix4 &rel_mat = transform->get_mat();

  LVector3 up = LVector3::up();
  LVector3 rel_pos = -rel_mat.get_row3(3);

  LQuaternion quat;
  ::heads_up(quat, rel_pos, up);
  set_quat(quat);

  // Also slide the geometry towards the camera according to the offset
  // factor.
  if (offset != 0.0f) {
    LVector3 translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    set_pos(translate);
  }
}

/**
 * Performs a billboard-type rotate to the indicated camera node, one time
 * only, and leaves the object rotated.  This is similar in principle to
 * look_at(), although the point_eye billboard effect cannot be achieved using
 * the ordinary look_at() call.
 */
void NodePath::
do_billboard_point_eye(const NodePath &camera, PN_stdfloat offset) {
  nassertv_always(!is_empty());

  CPT(TransformState) transform = camera.get_transform(get_parent());
  const LMatrix4 &rel_mat = transform->get_mat();

  LVector3 up = LVector3::up() * rel_mat;
  LVector3 rel_pos = LVector3::forward() * rel_mat;

  LQuaternion quat;
  ::look_at(quat, rel_pos, up);
  set_quat(quat);

  // Also slide the geometry towards the camera according to the offset
  // factor.
  if (offset != 0.0f) {
    LVector3 translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    set_pos(translate);
  }
}

/**
 * Performs a billboard-type rotate to the indicated camera node, one time
 * only, and leaves the object rotated.  This is similar in principle to
 * look_at().
 */
void NodePath::
do_billboard_point_world(const NodePath &camera, PN_stdfloat offset) {
  nassertv_always(!is_empty());

  CPT(TransformState) transform = camera.get_transform(get_parent());
  const LMatrix4 &rel_mat = transform->get_mat();

  LVector3 up = LVector3::up();
  LVector3 rel_pos = -rel_mat.get_row3(3);

  LQuaternion quat;
  ::look_at(quat, rel_pos, up);
  set_quat(quat);

  // Also slide the geometry towards the camera according to the offset
  // factor.
  if (offset != 0.0f) {
    LVector3 translate = rel_mat.get_row3(3);
    translate.normalize();
    translate *= offset;
    set_pos(translate);
  }
}

/**
 * Puts a billboard transition on the node such that it will rotate in two
 * dimensions around the up axis, towards a specified "camera" instead of to
 * the viewing camera.
 */
void NodePath::
set_billboard_axis(const NodePath &camera, PN_stdfloat offset) {
  nassertv_always(!is_empty());
  CPT(RenderEffect) billboard = BillboardEffect::make
    (LVector3::up(), false, true,
     offset, camera, LPoint3(0.0f, 0.0f, 0.0f));
  node()->set_effect(billboard);
}

/**
 * Puts a billboard transition on the node such that it will rotate in three
 * dimensions about the origin, keeping its up vector oriented to the top of
 * the camera, towards a specified "camera" instead of to the viewing camera.
 */
void NodePath::
set_billboard_point_eye(const NodePath &camera, PN_stdfloat offset, bool fixed_depth) {
  nassertv_always(!is_empty());
  CPT(RenderEffect) billboard = BillboardEffect::make
    (LVector3::up(), true, false,
     offset, camera, LPoint3(0.0f, 0.0f, 0.0f), fixed_depth);
  node()->set_effect(billboard);
}

/**
 * Puts a billboard transition on the node such that it will rotate in three
 * dimensions about the origin, keeping its up vector oriented to the sky,
 * towards a specified "camera" instead of to the viewing camera.
 */
void NodePath::
set_billboard_point_world(const NodePath &camera, PN_stdfloat offset) {
  nassertv_always(!is_empty());
  CPT(RenderEffect) billboard = BillboardEffect::make
    (LVector3::up(), false, false,
     offset, camera, LPoint3(0.0f, 0.0f, 0.0f));
  node()->set_effect(billboard);
}

/**
 * Removes any billboard effect from the node.
 */
void NodePath::
clear_billboard() {
  nassertv_always(!is_empty());
  node()->clear_effect(BillboardEffect::get_class_type());
}

/**
 * Returns true if there is any billboard effect on the node.
 */
bool NodePath::
has_billboard() const {
  nassertr_always(!is_empty(), false);
  return node()->has_effect(BillboardEffect::get_class_type());
}

/**
 * Puts a compass effect on the node, so that it will retain a fixed rotation
 * relative to the reference node (or render if the reference node is empty)
 * regardless of the transforms above it.
 */
void NodePath::
set_compass(const NodePath &reference) {
  nassertv_always(!is_empty());
  node()->set_effect(CompassEffect::make(reference));
}

/**
 * Removes any compass effect from the node.
 */
void NodePath::
clear_compass() {
  nassertv_always(!is_empty());
  node()->clear_effect(CompassEffect::get_class_type());
}

/**
 * Returns true if there is any compass effect on the node.
 */
bool NodePath::
has_compass() const {
  nassertr_always(!is_empty(), false);
  return node()->has_effect(CompassEffect::get_class_type());
}

/**
 * Specifically sets or disables transparent rendering mode on this particular
 * node.  If no other nodes override, this will cause items with a non-1 value
 * for alpha color to be rendered partially transparent.
 */
void NodePath::
set_transparency(TransparencyAttrib::Mode mode, int priority) {
  nassertv_always(!is_empty());

  node()->set_attrib(TransparencyAttrib::make(mode), priority);
}

/**
 * Completely removes any transparency adjustment that may have been set on
 * this node via set_transparency(). The geometry at this level and below will
 * subsequently be rendered either transparent or not, to whatever other nodes
 * may have had set_transparency() on them.
 */
void NodePath::
clear_transparency() {
  nassertv_always(!is_empty());
  node()->clear_attrib(TransparencyAttrib::get_class_slot());
}

/**
 * Returns true if a transparent-rendering adjustment has been explicitly set
 * on this particular node via set_transparency().  If this returns true, then
 * get_transparency() may be called to determine whether transparency has been
 * explicitly enabled or explicitly disabled for this node.
 */
bool NodePath::
has_transparency() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(TransparencyAttrib::get_class_slot());
}

/**
 * Returns the transparent rendering that has been specifically set on this
 * node via set_transparency(), or M_none if nontransparent rendering has been
 * specifically set, or if nothing has been specifically set.  See also
 * has_transparency().  This does not necessarily imply that the geometry will
 * or will not be rendered transparent, as there may be other nodes that
 * override.
 */
TransparencyAttrib::Mode NodePath::
get_transparency() const {
  nassertr_always(!is_empty(), TransparencyAttrib::M_none);
  const RenderAttrib *attrib =
    node()->get_attrib(TransparencyAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TransparencyAttrib *ta = DCAST(TransparencyAttrib, attrib);
    return ta->get_mode();
  }

  return TransparencyAttrib::M_none;
}

/**
 * Specifically sets or disables a logical operation on this particular node.
 * If no other nodes override, this will cause geometry to be rendered without
 * color blending but instead using the given logical operator.
 */
void NodePath::
set_logic_op(LogicOpAttrib::Operation op, int priority) {
  nassertv_always(!is_empty());

  node()->set_attrib(LogicOpAttrib::make(op), priority);
}

/**
 * Completely removes any logical operation that may have been set on this
 * node via set_logic_op(). The geometry at this level and below will
 * subsequently be rendered using standard color blending.
 */
void NodePath::
clear_logic_op() {
  nassertv_always(!is_empty());
  node()->clear_attrib(LogicOpAttrib::get_class_slot());
}

/**
 * Returns true if a logical operation has been explicitly set on this
 * particular node via set_logic_op().  If this returns true, then
 * get_logic_op() may be called to determine whether a logical operation has
 * been explicitly disabled for this node or set to particular operation.
 */
bool NodePath::
has_logic_op() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(LogicOpAttrib::get_class_slot());
}

/**
 * Returns the logical operation that has been specifically set on this node
 * via set_logic_op(), or O_none if standard color blending has been
 * specifically set, or if nothing has been specifically set.  See also
 * has_logic_op().  This does not necessarily imply that the geometry will
 * or will not be rendered with the given logical operation, as there may be
 * other nodes that override.
 */
LogicOpAttrib::Operation NodePath::
get_logic_op() const {
  nassertr_always(!is_empty(), LogicOpAttrib::O_none);
  const RenderAttrib *attrib =
    node()->get_attrib(LogicOpAttrib::get_class_slot());
  if (attrib != nullptr) {
    const LogicOpAttrib *ta = DCAST(LogicOpAttrib, attrib);
    return ta->get_operation();
  }

  return LogicOpAttrib::O_none;
}

/**
 * Specifies the antialiasing type that should be applied at this node and
 * below.  See AntialiasAttrib.
 */
void NodePath::
set_antialias(unsigned short mode, int priority) {
  nassertv_always(!is_empty());

  node()->set_attrib(AntialiasAttrib::make(mode), priority);
}

/**
 * Completely removes any antialias setting that may have been set on this
 * node via set_antialias().
 */
void NodePath::
clear_antialias() {
  nassertv_always(!is_empty());
  node()->clear_attrib(AntialiasAttrib::get_class_slot());
}

/**
 * Returns true if an antialias setting has been explicitly mode on this
 * particular node via set_antialias().  If this returns true, then
 * get_antialias() may be called to determine what the setting was.
 */
bool NodePath::
has_antialias() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(AntialiasAttrib::get_class_slot());
}

/**
 * Returns the antialias setting that has been specifically set on this node
 * via set_antialias(), or M_none if no setting has been made.
 */
unsigned short NodePath::
get_antialias() const {
  nassertr_always(!is_empty(), AntialiasAttrib::M_none);
  const RenderAttrib *attrib =
    node()->get_attrib(AntialiasAttrib::get_class_slot());
  if (attrib != nullptr) {
    const AntialiasAttrib *ta = DCAST(AntialiasAttrib, attrib);
    return ta->get_mode();
  }

  return AntialiasAttrib::M_none;
}

/**
 * Returns true if an audio volume has been applied to the referenced node,
 * false otherwise.  It is still possible that volume at this node might have
 * been scaled by an ancestor node.
 */
bool NodePath::
has_audio_volume() const {
  nassertr_always(!is_empty(), false);
  return node()->has_attrib(AudioVolumeAttrib::get_class_slot());
}

/**
 * Completely removes any audio volume from the referenced node.  This is
 * preferable to simply setting the audio volume to identity, as it also
 * removes the overhead associated with having an audio volume at all.
 */
void NodePath::
clear_audio_volume() {
  nassertv_always(!is_empty());
  node()->clear_attrib(AudioVolumeAttrib::get_class_slot());
}

/**
 * Sets the audio volume component of the transform
 */
void NodePath::
set_audio_volume(PN_stdfloat volume, int priority) {
  nassertv_always(!is_empty());

  const RenderAttrib *attrib =
    node()->get_attrib(AudioVolumeAttrib::get_class_slot());
  if (attrib != nullptr) {
    priority = max(priority,
                   node()->get_state()->get_override(AudioVolumeAttrib::get_class_slot()));
    CPT(AudioVolumeAttrib) ava = DCAST(AudioVolumeAttrib, attrib);

    // Modify the existing AudioVolumeAttrib to add the indicated volume.
    node()->set_attrib(ava->set_volume(volume), priority);

  } else {
    // Create a new AudioVolumeAttrib for this node.
    node()->set_attrib(AudioVolumeAttrib::make(volume), priority);
  }
}

/**
 * Disables any audio volume attribute inherited from above.  This is not the
 * same thing as clear_audio_volume(), which undoes any previous
 * set_audio_volume() operation on this node; rather, this actively disables
 * any set_audio_volume() that might be inherited from a parent node.
 *
 * It is legal to specify a new volume on the same node with a subsequent call
 * to set_audio_volume(); this new scale will apply to lower nodes.
 */
void NodePath::
set_audio_volume_off(int priority) {
  nassertv_always(!is_empty());
  node()->set_attrib(AudioVolumeAttrib::make_off(), priority);
}

/**
 * Returns the complete audio volume that has been applied to this node via a
 * previous call to set_audio_volume(), or 1. (identity) if no volume has been
 * applied to this particular node.
 */
PN_stdfloat NodePath::
get_audio_volume() const {
  const RenderAttrib *attrib =
    node()->get_attrib(AudioVolumeAttrib::get_class_slot());
  if (attrib != nullptr) {
    const AudioVolumeAttrib *ava = DCAST(AudioVolumeAttrib, attrib);
    return ava->get_volume();
  }

  return 1.0f;
}

/**
 * Returns the complete audio volume for this node taking highers nodes in the
 * graph into account.
 */
PN_stdfloat NodePath::
get_net_audio_volume() const {
  CPT(RenderState) net_state = get_net_state();
  const RenderAttrib *attrib = net_state->get_attrib(AudioVolumeAttrib::get_class_slot());
  if (attrib != nullptr) {
    const AudioVolumeAttrib *ava = DCAST(AudioVolumeAttrib, attrib);
    if (ava != nullptr) {
      return ava->get_volume();
    }
  }

  return 1.0f;
}

/**
 * Returns the NodePath at or above the referenced node that is hidden to the
 * indicated camera(s), or an empty NodePath if no ancestor of the referenced
 * node is hidden (and the node should be visible).
 */
NodePath NodePath::
get_hidden_ancestor(DrawMask camera_mask, Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();

  NodePathComponent *comp;
  for (comp = _head;
       comp != nullptr;
       comp = comp->get_next(pipeline_stage, current_thread)) {
    PandaNode *node = comp->get_node();
    if (node->is_overall_hidden() ||
        ((node->get_draw_show_mask() | ~node->get_draw_control_mask()) & camera_mask).is_zero()) {
      NodePath result;
      result._head = comp;
      return result;
    }
  }

  return not_found();
}

/**
 * Removes the referenced node (and the entire subgraph below this node) from
 * the scene graph in any normal sense.  The node will no longer be visible
 * and is not tested for collisions; furthermore, no normal scene graph
 * traversal will visit the node.  The node's bounding volume no longer
 * contributes to its parent's bounding volume.
 *
 * A stashed node cannot be located by a normal find() operation (although a
 * special find string can still retrieve it).
 */
void NodePath::
stash(int sort, Thread *current_thread) {
  nassertv_always(!is_singleton() && !is_empty());
  nassertv(verify_complete());

  int pipeline_stage = current_thread->get_pipeline_stage();
  bool reparented = PandaNode::reparent(_head->get_next(pipeline_stage, current_thread),
                                        _head, sort, true, pipeline_stage,
                                        current_thread);
  nassertv(reparented);
}

/**
 * Undoes the effect of a previous stash() on this node: makes the referenced
 * node (and the entire subgraph below this node) once again part of the scene
 * graph.
 */
void NodePath::
unstash(int sort, Thread *current_thread) {
  nassertv_always(!is_singleton() && !is_empty());
  nassertv(verify_complete());

  int pipeline_stage = current_thread->get_pipeline_stage();
  bool reparented = PandaNode::reparent(_head->get_next(pipeline_stage, current_thread),
                                        _head, sort, false, pipeline_stage,
                                        current_thread);
  nassertv(reparented);
}

/**
 * Unstashes this node and all stashed child nodes.
 */
void NodePath::
unstash_all(Thread *current_thread) {
  NodePathCollection stashed_descendents = find_all_matches("**/@@*");
  stashed_descendents.unstash();
  unstash(0, current_thread);
}

/**
 * Returns the NodePath at or above the referenced node that is stashed, or an
 * empty NodePath if no ancestor of the referenced node is stashed (and the
 * node should be visible).
 */
NodePath NodePath::
get_stashed_ancestor(Thread *current_thread) const {
  NodePathComponent *comp = _head;
  if (comp != nullptr) {
    int pipeline_stage = current_thread->get_pipeline_stage();
    NodePathComponent *next = comp->get_next(pipeline_stage, current_thread);

    while (next != nullptr) {
      PandaNode *node = comp->get_node();
      PandaNode *parent_node = next->get_node();

      if (parent_node->find_stashed(node) >= 0) {
        NodePath result;
        result._head = comp;
        return result;
      }

      comp = next;
      next = next->get_next(pipeline_stage, current_thread);
    }
  }

  return not_found();
}

/**
 * Returns true if the two paths are equivalent; that is, if they contain the
 * same list of nodes in the same order.
 */
bool NodePath::
operator == (const WeakNodePath &other) const {
  return (other == *this);
}

/**
 * Returns true if the two paths are not equivalent.
 */
bool NodePath::
operator != (const WeakNodePath &other) const {
  return (other != *this);
}

/**
 * Returns true if this NodePath sorts before the other one, false otherwise.
 * The sorting order of two nonequivalent NodePaths is consistent but
 * undefined, and is useful only for storing NodePaths in a sorted container
 * like an STL set.
 */
bool NodePath::
operator < (const WeakNodePath &other) const {
  return other.compare_to(*this) > 0;
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
int NodePath::
compare_to(const WeakNodePath &other) const {
  return -other.compare_to(*this);
}

/**
 * Returns true if all of the nodes described in the NodePath are connected,
 * or false otherwise.
 */
bool NodePath::
verify_complete(Thread *current_thread) const {
  if (is_empty()) {
    return true;
  }

#ifdef HAVE_THREADS
  if (Thread::is_true_threads()) {
    // In a threaded environment, we can't reliably test this, since a sub-
    // thread may be mucking with the NodePath's ancestry as we try to
    // validate it.  NodePaths are inherently not thread-safe, but generally
    // that's not an issue.
    return true;
  }
#endif  // HAVE_THREADS

  PStatTimer timer(_verify_complete_pcollector);

  const NodePathComponent *comp = _head;
  nassertr(comp != nullptr, false);

  int pipeline_stage = current_thread->get_pipeline_stage();

  PandaNode *node = comp->get_node();
  nassertr(node != nullptr, false);
  int length = comp->get_length(pipeline_stage, current_thread);

  comp = comp->get_next(pipeline_stage, current_thread);
  length--;
  while (comp != nullptr) {
    PandaNode *next_node = comp->get_node();
    nassertr(next_node != nullptr, false);

    if (node->find_parent(next_node) < 0) {
      pgraph_cat.warning()
        << *this << " is incomplete; " << *node << " is not a child of "
        << *next_node << "\n";
      return false;
    }

    if (comp->get_length(pipeline_stage, current_thread) != length) {
      pgraph_cat.warning()
        << *this << " is incomplete; length at " << *next_node
        << " indicates " << comp->get_length(pipeline_stage, current_thread)
        << " while length at " << *node << " indicates " << length << "\n";
      return false;
    }

    node = next_node;
    comp = comp->get_next(pipeline_stage, current_thread);
    length--;
  }

  return true;
}

/**
 * Walks through the scene graph beginning at the bottom node, and internally
 * adjusts any GeomVertexFormats for optimal rendering on the indicated GSG.
 * If this step is not done prior to rendering, the formats will be optimized
 * at render time instead, for a small cost.
 *
 * It is not normally necessary to do this on a model loaded directly from
 * disk, since the loader will do this by default.
 */
void NodePath::
premunge_scene(GraphicsStateGuardianBase *gsg) {
  nassertv_always(!is_empty());

  CPT(RenderState) state = RenderState::make_empty();
  if (has_parent()) {
    state = get_parent().get_net_state();
  }

  SceneGraphReducer gr(gsg);
  gr.premunge(node(), state);
}

/**
 * Walks through the scene graph beginning at the bottom node, and does
 * whatever initialization is required to render the scene properly with the
 * indicated GSG.  It is not strictly necessary to call this, since the GSG
 * will initialize itself when the scene is rendered, but this may take some
 * of the overhead away from that process.
 *
 * In particular, this will ensure that textures and vertex buffers within the
 * scene are loaded into graphics memory.
 */
void NodePath::
prepare_scene(GraphicsStateGuardianBase *gsg) {
  nassertv_always(!is_empty());

  node()->prepare_scene(gsg, get_net_state());
}

/**
 * Causes the bounding volume of the bottom node and all of its descendants
 * (that is, the bounding volume associated with the the bottom arc) to be
 * rendered, if possible.  The rendering method is less than optimal; this is
 * intended primarily for debugging.
 */
void NodePath::
show_bounds() {
  nassertv_always(!is_empty());
  node()->set_effect(ShowBoundsEffect::make(false));
}

/**
 * Similar to show_bounds(), this draws a bounding box representing the
 * "tight" bounds of this node and all of its descendants.  The bounding box
 * is recomputed every frame by reexamining all of the vertices; this is far
 * from efficient, but this is intended for debugging.
 */
void NodePath::
show_tight_bounds() {
  nassertv_always(!is_empty());
  node()->set_effect(ShowBoundsEffect::make(true));
}

/**
 * Stops the rendering of the bounding volume begun with show_bounds().
 */
void NodePath::
hide_bounds() {
  nassertv_always(!is_empty());
  node()->clear_effect(ShowBoundsEffect::get_class_type());
}

/**
 * Returns a newly-allocated bounding volume containing the bottom node and
 * all of its descendants.  This is the bounding volume on the bottom arc,
 * converted to the local coordinate space of the node.
 */
PT(BoundingVolume) NodePath::
get_bounds(Thread *current_thread) const {
  nassertr_always(!is_empty(), new BoundingSphere);
  return node()->get_bounds(current_thread)->make_copy();
}

/**
 * Forces the recomputing of all the bounding volumes at every node in the
 * subgraph beginning at this node and below.
 *
 * This should not normally need to be called, since the bounding volumes are
 * supposed to be recomputed automatically when necessary.  It may be useful
 * when debugging, to verify that the bounding volumes have not become
 * inadvertently stale; it may also be useful to force animated characters to
 * update their bounding volumes (which does not presently happen
 * automatically).
 */
void NodePath::
force_recompute_bounds() {
  nassertv_always(!is_empty());
  r_force_recompute_bounds(node());
}

/**
 * Writes a description of the bounding volume containing the bottom node and
 * all of its descendants to the indicated output stream.
 */
void NodePath::
write_bounds(ostream &out) const {
  get_bounds()->write(out);
}

/**
 * Calculates the minimum and maximum vertices of all Geoms at this NodePath's
 * bottom node and below.  This is a tight bounding box; it will generally be
 * tighter than the bounding volume returned by get_bounds() (but it is more
 * expensive to compute).
 *
 * The bounding box is computed relative to the parent node's coordinate
 * system by default.  You can optionally specify a different NodePath to
 * compute the bounds relative to.  Note that the box is always axis-aligned
 * against the given NodePath's coordinate system, so you might get a
 * differently sized box depending on which node you pass.
 *
 * The return value is true if any points are within the bounding volume, or
 * false if none are.
 */
bool NodePath::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  const NodePath &other, Thread *current_thread) const {
  min_point.set(0.0f, 0.0f, 0.0f);
  max_point.set(0.0f, 0.0f, 0.0f);
  nassertr_always(!is_empty(), false);

  CPT(TransformState) transform = TransformState::make_identity();
  if (!other.is_empty()) {
    transform = get_transform(other)->compose(get_transform()->get_inverse());
  }

  bool found_any = false;
  node()->calc_tight_bounds(min_point, max_point, found_any,
                            move(transform), current_thread);

  return found_any;
}

/**
 * Analyzes the geometry below this node and reports the number of vertices,
 * triangles, etc.  This is the same information reported by the bam-info
 * program.
 */
/*
NB: Had to remove this function to avoid circular dependency when
moving SceneGraphAnalyzer into pgraphnodes, attempting to reduce size
of pgraph.  This function is now defined as a Python extension
function instead.

void NodePath::
analyze() const {
  nassertv_always(!is_empty());
  SceneGraphAnalyzer sga;
  sga.add_node(node());

  if (sga.get_num_lod_nodes() == 0) {
    sga.write(nout);

  } else {
    nout << "At highest LOD:\n";
    SceneGraphAnalyzer sga2;
    sga2.set_lod_mode(SceneGraphAnalyzer::LM_highest);
    sga2.add_node(node());
    sga2.write(nout);

    nout << "\nAt lowest LOD:\n";
    sga2.clear();
    sga2.set_lod_mode(SceneGraphAnalyzer::LM_lowest);
    sga2.add_node(node());
    sga2.write(nout);

    nout << "\nAll nodes:\n";
    sga.write(nout);
  }
}
*/

/**
 * Lightly flattens out the hierarchy below this node by applying transforms,
 * colors, and texture matrices from the nodes onto the vertices, but does not
 * remove any nodes.
 *
 * This can result in improved rendering performance because there will be
 * fewer transforms in the resulting scene graph, but the number of nodes will
 * remain the same.
 *
 * In particular, any NodePaths that reference nodes within this hierarchy
 * will not be damaged.  However, since this operation will remove transforms
 * from the scene graph, it may be dangerous to apply to nodes where you
 * expect to dynamically modify the transform, or where you expect the
 * geometry to remain in a particular local coordinate system.
 *
 * The return value is always 0, since flatten_light does not remove any
 * nodes.
 */
int NodePath::
flatten_light() {
  nassertr_always(!is_empty(), 0);
  SceneGraphReducer gr;
  gr.apply_attribs(node());

  return 0;
}

/**
 * A more thorough flattening than flatten_light(), this first applies all the
 * transforms, colors, and texture matrices from the nodes onto the vertices,
 * and then removes unneeded grouping nodes--nodes that have exactly one
 * child, for instance, but have no special properties in themselves.
 *
 * This results in improved performance over flatten_light() because the
 * number of nodes in the scene graph is reduced.
 *
 * The return value is the number of nodes removed.
 */
int NodePath::
flatten_medium() {
  nassertr_always(!is_empty(), 0);
  SceneGraphReducer gr;
  gr.apply_attribs(node());
  int num_removed = gr.flatten(node(), 0);

  if (flatten_geoms) {
    gr.make_compatible_state(node());
    gr.collect_vertex_data(node());
    gr.unify(node(), true);
  }

  return num_removed;
}

/**
 * The strongest possible flattening.  This first applies all of the
 * transforms to the vertices, as in flatten_medium(), but then it will
 * combine sibling nodes together when possible, in addition to removing
 * unnecessary parent-child nodes.  This can result in substantially fewer
 * nodes, but any nicely-grouped hierachical bounding volumes may be lost.
 *
 * It is generally a good idea to apply this kind of flattening only to nodes
 * that will be culled largely as a single unit, like a car.  Applying this to
 * an entire scene may result in overall poorer performance because of less-
 * effective culling.
 */
int NodePath::
flatten_strong() {
  nassertr_always(!is_empty(), 0);
  SceneGraphReducer gr;
  gr.apply_attribs(node());
  int num_removed = gr.flatten(node(), ~0);

  if (flatten_geoms) {
    gr.make_compatible_state(node());
    gr.collect_vertex_data(node(), ~(SceneGraphReducer::CVD_format | SceneGraphReducer::CVD_name | SceneGraphReducer::CVD_animation_type));
    gr.unify(node(), false);
  }

  return num_removed;
}

/**
 * Removes textures from Geoms at this node and below by applying the texture
 * colors to the vertices.  This is primarily useful to simplify a low-LOD
 * model.  The texture colors are replaced by flat colors that approximate the
 * original textures.
 *
 * Only the bottommost texture on each Geom is used (if there is more than
 * one), and it is applied as if it were M_modulate, and WM_repeat, regardless
 * of its actual settings.  If the texture has a simple_ram_image, this may be
 * used if the main image isn't resident.
 *
 * After this call, there will be no texturing specified at this level and
 * below.  Of course, there might still be texturing inherited from above.
 */
void NodePath::
apply_texture_colors() {
  nassertv_always(!is_empty());
  SceneGraphReducer gr;
  gr.apply_attribs(node(), SceneGraphReducer::TT_apply_texture_color | SceneGraphReducer::TT_tex_matrix | SceneGraphReducer::TT_other);
}

/**
 * Returns the lowest ancestor of this node that contains a tag definition
 * with the indicated key, if any, or an empty NodePath if no ancestor of this
 * node contains this tag definition.  See set_tag().
 */
NodePath NodePath::
find_net_tag(const string &key) const {
  if (is_empty()) {
    return NodePath::not_found();
  }
  if (has_tag(key)) {
    return *this;
  }
  return get_parent().find_net_tag(key);
}

/**
 * Writes the contents of this node and below out to a bam file with the
 * indicated filename.  This file may then be read in again, as is, at some
 * later point.  Returns true if successful, false on some kind of error.
 */
bool NodePath::
write_bam_file(const Filename &filename) const {
  nassertr_always(!is_empty(), false);

  BamFile bam_file;

  bool okflag = false;

  if (bam_file.open_write(filename)) {
    // Tell the BamWriter which node is the root node, for making NodePaths
    // relative to when writing them out to the file.
    bam_file.get_writer()->set_root_node(node());

    if (bam_file.write_object(node())) {
      okflag = true;
    }
    bam_file.close();
  }
  return okflag;
}

/**
 * Writes the contents of this node and below out to the indicated stream.
 */
bool NodePath::
write_bam_stream(ostream &out) const {
  nassertr_always(!is_empty(), false);

  BamFile bam_file;

  bool okflag = false;

  if (bam_file.open_write(out)) {
    // Tell the BamWriter which node is the root node, for making NodePaths
    // relative to when writing them out to the file.
    bam_file.get_writer()->set_root_node(node());

    if (bam_file.write_object(node())) {
      okflag = true;
    }
    bam_file.close();
  }
  return okflag;
}

/**
 * Converts the NodePath object into a single stream of data using a
 * BamWriter, and stores that data in the indicated string.  Returns true on
 * success, false on failure.
 *
 * If the BamWriter is NULL, this behaves the same way as
 * NodePath::write_bam_stream() and PandaNode::encode_to_bam_stream(), in the
 * sense that it only writes this node and all nodes below it.
 *
 * However, if the BamWriter is not NULL, it behaves very differently.  In
 * this case, it encodes the *entire graph* of all nodes connected to the
 * NodePath, including all parent nodes and siblings.  This is necessary for
 * correct streaming of related NodePaths and restoration of instances, etc.,
 * but it does mean you must detach() a node before writing it if you want to
 * limit the nodes that get written.
 *
 * This method is used by __reduce__ to handle streaming of NodePaths to a
 * pickle file.  The BamWriter case is used by the direct.stdpy.pickle module,
 * while the saner, non-BamWriter case is used when the standard pickle module
 * calls this function.
 */
bool NodePath::
encode_to_bam_stream(vector_uchar &data, BamWriter *writer) const {
  data.clear();
  ostringstream stream;

  DatagramBuffer buffer;
  BamWriter local_writer;
  bool used_local_writer = false;
  if (writer == nullptr) {
    // Create our own writer.

    if (!buffer.write_header(_bam_header)) {
      return false;
    }
    writer = &local_writer;
    used_local_writer = true;
  }

  writer->set_target(&buffer);

  int num_nodes = get_num_nodes();
  if (used_local_writer && num_nodes > 1) {
    // In this case--no BamWriter--we only write the bottom node.
    num_nodes = 1;
  }

  // Tell the BamWriter which node is the root node, for making NodePaths
  // relative to when writing them out to the file.
  if (!is_empty()) {
    writer->set_root_node(node());
  }

  // Write an initial Datagram to represent the error type and number of
  // nodes.
  Datagram dg;
  dg.add_uint8(_error_type);
  dg.add_int32(num_nodes);

  if (!buffer.put_datagram(dg)) {
    writer->set_target(nullptr);
    return false;
  }

  // Now write the nodes, one at a time.
  for (int i = 0; i < num_nodes; ++i) {
    PandaNode *node = get_node(num_nodes - i - 1);
    nassertr(node != nullptr, false);
    if (!writer->write_object(node)) {
      writer->set_target(nullptr);
      return false;
    }
  }
  writer->set_target(nullptr);

  buffer.swap_data(data);
  return true;
}

/**
 * Reads the string created by a previous call to encode_to_bam_stream(), and
 * extracts and returns the NodePath on that string.  Returns NULL on error.
 */
NodePath NodePath::
decode_from_bam_stream(vector_uchar data, BamReader *reader) {
  NodePath result;

  DatagramBuffer buffer(move(data));

  BamReader local_reader;
  if (reader == nullptr) {
    // Create a local reader.

    string head;
    if (!buffer.read_header(head, _bam_header.size())) {
      return NodePath::fail();
    }

    if (head != _bam_header) {
      return NodePath::fail();
    }

    reader = &local_reader;
  }

  reader->set_source(&buffer);

  // One initial datagram to encode the error type, and the number of nodes.
  Datagram dg;
  if (!buffer.get_datagram(dg)) {
    return NodePath::fail();
  }

  DatagramIterator dgi(dg);
  ErrorType error_type = (ErrorType)dgi.get_uint8();
  int num_nodes = dgi.get_int32();
  if (num_nodes == 0) {
    // An empty NodePath.
    result._error_type = error_type;

  } else {
    // A real NodePath.  Ignore error_type.
    for (int i = 0; i < num_nodes; ++i) {
      TypedWritable *object = reader->read_object();

      if (object == nullptr ||
          !object->is_of_type(PandaNode::get_class_type())) {
        reader->set_source(nullptr);
        return NodePath::fail();
      }

      if (!reader->resolve()) {
        reader->set_source(nullptr);
        return NodePath::fail();
      }

      PandaNode *node = DCAST(PandaNode, object);
      result = NodePath(result, node);
    }
  }

  reader->set_source(nullptr);

  return result;
}

/**
 * Walks up from both NodePaths to find the first node that both have in
 * common, if any.  Fills a_count and b_count with the number of nodes below
 * the common node in each path.
 *
 * The return value is the NodePathComponent of the node they have in common,
 * or NULL if they have nothing in common.
 */
NodePathComponent *NodePath::
find_common_ancestor(const NodePath &a, const NodePath &b,
                     int &a_count, int &b_count, Thread *current_thread) {
  nassertr(!a.is_empty() && !b.is_empty(), nullptr);
  NodePathComponent *ac = a._head;
  NodePathComponent *bc = b._head;
  a_count = 0;
  b_count = 0;

  int pipeline_stage = current_thread->get_pipeline_stage();

  // Shorten up the longer one until they are the same length.
  while (ac->get_length(pipeline_stage, current_thread) > bc->get_length(pipeline_stage, current_thread)) {
    nassertr(ac != nullptr, nullptr);
    ac = ac->get_next(pipeline_stage, current_thread);
    a_count++;
  }
  while (bc->get_length(pipeline_stage, current_thread) > ac->get_length(pipeline_stage, current_thread)) {
    nassertr(bc != nullptr, nullptr);
    bc = bc->get_next(pipeline_stage, current_thread);
    b_count++;
  }

  // Now shorten them both up until we reach the same component.
  while (ac != bc) {
    // These shouldn't go to NULL unless they both go there together.
    nassertr(ac != nullptr, nullptr);
    nassertr(bc != nullptr, nullptr);
    ac = ac->get_next(pipeline_stage, current_thread);
    a_count++;
    bc = bc->get_next(pipeline_stage, current_thread);
    b_count++;
  }

  return ac;
}

/**
 * Recursively determines the net state changes to the indicated component
 * node from the root of the graph.
 */
CPT(RenderState) NodePath::
r_get_net_state(NodePathComponent *comp, Thread *current_thread) const {
  if (comp == nullptr) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state(current_thread);
    int pipeline_stage = current_thread->get_pipeline_stage();
    return r_get_net_state(comp->get_next(pipeline_stage, current_thread), current_thread)->compose(state);
  }
}

/**
 * Recursively determines the net state changes to the indicated component
 * node from the nth node above it.  If n exceeds the length of the path, this
 * returns the net transform from the root of the graph.
 */
CPT(RenderState) NodePath::
r_get_partial_state(NodePathComponent *comp, int n,
                    Thread *current_thread) const {
  if (n == 0 || comp == nullptr) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state(current_thread);
    int pipeline_stage = current_thread->get_pipeline_stage();
    return r_get_partial_state(comp->get_next(pipeline_stage, current_thread), n - 1, current_thread)->compose(state);
  }
}

/**
 * Recursively determines the net transform to the indicated component node
 * from the root of the graph.
 */
CPT(TransformState) NodePath::
r_get_net_transform(NodePathComponent *comp, Thread *current_thread) const {
  if (comp == nullptr) {
    return TransformState::make_identity();
  } else {
    PandaNode *node = comp->get_node();
    int pipeline_stage = current_thread->get_pipeline_stage();
    CPT(TransformState) net_transform = r_get_net_transform(comp->get_next(pipeline_stage, current_thread), current_thread);

    PandaNode::CDReader node_cdata(node->_cycler, current_thread);
    if (!node_cdata->_effects->has_adjust_transform()) {
      if (node_cdata->_transform->is_identity()) {
        return net_transform;
      } else {
        return net_transform->compose(node_cdata->_transform);
      }
    } else {
      CPT(TransformState) transform = node_cdata->_transform.p();
      node_cdata->_effects->adjust_transform(net_transform, transform, node);
      return net_transform->compose(transform);
    }
  }
}

/**
 * Recursively determines the net transform to the indicated component node
 * from the nth node above it.  If n exceeds the length of the path, this
 * returns the net transform from the root of the graph.
 *
 * If any node in the path had a net_transform effect applied, returns NULL--
 * in this case the partial transform cannot be easily determined.
 */
CPT(TransformState) NodePath::
r_get_partial_transform(NodePathComponent *comp, int n,
                        Thread *current_thread) const {
  if (n == 0 || comp == nullptr) {
    return TransformState::make_identity();
  } else {
    PandaNode *node = comp->get_node();
    PandaNode::CDReader node_cdata(node->_cycler, current_thread);
    if (node_cdata->_effects->has_adjust_transform()) {
      return nullptr;
    }
    int pipeline_stage = current_thread->get_pipeline_stage();
    CPT(TransformState) partial = r_get_partial_transform(comp->get_next(pipeline_stage, current_thread), n - 1, current_thread);
    if (partial == nullptr) {
      return nullptr;
    }
    if (node_cdata->_transform->is_identity()) {
      return partial;
    } else {
      return partial->compose(node_cdata->_transform);
    }
  }
}

/**
 * Recursively determines the net "previous" transform to the indicated
 * component node from the root of the graph.
 */
CPT(TransformState) NodePath::
r_get_net_prev_transform(NodePathComponent *comp, Thread *current_thread) const {
  if (comp == nullptr) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_prev_transform(current_thread);
    int pipeline_stage = current_thread->get_pipeline_stage();
    return r_get_net_prev_transform(comp->get_next(pipeline_stage, current_thread), current_thread)->compose(transform);
  }
}

/**
 * Recursively determines the net "previous" transform to the indicated
 * component node from the nth node above it.  If n exceeds the length of the
 * path, this returns the net previous transform from the root of the graph.
 */
CPT(TransformState) NodePath::
r_get_partial_prev_transform(NodePathComponent *comp, int n, Thread *current_thread) const {
  if (n == 0 || comp == nullptr) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_prev_transform(current_thread);
    int pipeline_stage = current_thread->get_pipeline_stage();
    return r_get_partial_prev_transform(comp->get_next(pipeline_stage, current_thread), n - 1, current_thread)->compose(transform);
  }
}

/**
 * Finds up to max_matches matches against the given path string from this
 * node and deeper.  The max_matches count indicates the maximum number of
 * matches to return, or -1 not to limit the number returned.
 */
void NodePath::
find_matches(NodePathCollection &result, const string &path,
             int max_matches) const {
  if (is_empty()) {
    pgraph_cat.warning()
      << "Attempt to extend an empty NodePath by '" << path
      << "'.\n";
    return;
  }
  FindApproxPath approx_path;
  if (approx_path.add_string(path)) {
    find_matches(result, approx_path, max_matches);
  }
}

/**
 * Finds up to max_matches matches against the given approx_path from this
 * node and deeper.  The max_matches count indicates the maximum number of
 * matches to return, or -1 not to limit the number returned.
 */
void NodePath::
find_matches(NodePathCollection &result, FindApproxPath &approx_path,
             int max_matches) const {
  if (is_empty()) {
    pgraph_cat.warning()
      << "Attempt to extend an empty NodePath by: " << approx_path << ".\n";
    return;
  }

  // We start with just one entry on the level.
  FindApproxLevelEntry *level =
    new FindApproxLevelEntry(WorkingNodePath(*this), approx_path);
  nassertv(level->_node_path.is_valid());

  find_matches(result, level, max_matches);
}

/**
 * The fundamental implementation of find_matches(), given a starting level (a
 * linked list of FindApproxLevelEntry objects).
 */
void NodePath::
find_matches(NodePathCollection &result, FindApproxLevelEntry *level,
             int max_matches) const {

  int num_levels_remaining = _max_search_depth;

  FindApproxLevelEntry *deleted_entries = nullptr;

  while (num_levels_remaining > 0 && level != nullptr) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "find_matches pass: " << result << ", "
        << max_matches << ", " << num_levels_remaining << "\n";
      level->write_level(pgraph_cat.spam(false), 4);
    }

    num_levels_remaining--;

    FindApproxLevelEntry *next_level = nullptr;

    // For each node in the current level, build up the set of possible
    // matches in the next level.
    FindApproxLevelEntry *entry = level;
    while (entry != nullptr) {
      if (entry->consider_node(result, next_level, max_matches, 0)) {
        // If we found the requisite number of matches, we can stop.  Delete
        // all remaining entries and return immediately.

        while (entry != nullptr) {
          FindApproxLevelEntry *next = entry->_next;
          delete entry;
          entry = next;
        }
        while (next_level != nullptr) {
          FindApproxLevelEntry *next = next_level->_next;
          delete next_level;
          next_level = next;
        }
        while (deleted_entries != nullptr) {
          FindApproxLevelEntry *next = deleted_entries->_next;
          delete deleted_entries;
          deleted_entries = next;
        }
        return;
      }

      // Move the entry to the delete chain so we can delete it before we
      // return from this method.  (We can't delete it immediately, because
      // there might be WorkingNodePaths in the next_level that reference the
      // WorkingNodePath object within the entry.)
      FindApproxLevelEntry *next = entry->_next;
      entry->_next = deleted_entries;
      deleted_entries = entry;

      entry = next;
    }

    // Make sure the remaining entries from this level are added to the delete
    // chain.
    while (entry != nullptr) {
      FindApproxLevelEntry *next = entry->_next;
      entry->_next = deleted_entries;
      deleted_entries = entry;

      entry = next;
    }

    level = next_level;
  }

  // Now it's safe to delete all entries on the delete chain.
  while (deleted_entries != nullptr) {
    FindApproxLevelEntry *next = deleted_entries->_next;
    delete deleted_entries;
    deleted_entries = next;
  }
}

/**
 * The recursive implementation of clear_model_nodes().  This walks through
 * the subgraph defined by the indicated node and below.
 */
int NodePath::
r_clear_model_nodes(PandaNode *node) {
  int count = 0;

  if (node->is_of_type(ModelNode::get_class_type())) {
    ModelNode *mnode;
    DCAST_INTO_R(mnode, node, count);
    mnode->set_preserve_transform(ModelNode::PT_drop_node);
    ++count;
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    count += r_clear_model_nodes(cr.get_child(i));
  }

  return count;
}

/**
 * The recursive implementation of adjust_all_priorities().  This walks
 * through the subgraph defined by the indicated node and below.
 */
void NodePath::
r_adjust_all_priorities(PandaNode *node, int adjustment) {
  node->set_state(node->get_state()->adjust_all_priorities(adjustment));
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      gnode->set_geom_state(i, gnode->get_geom_state(i)->adjust_all_priorities(adjustment));
    }
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_adjust_all_priorities(cr.get_child(i), adjustment);
  }
}

/**
 *
 */
void NodePath::
r_force_recompute_bounds(PandaNode *node) {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      const Geom *geom = gnode->get_geom(i);
      geom->mark_bounds_stale();
    }
  }

  node->mark_bounds_stale();

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_force_recompute_bounds(cr.get_child(i));
  }
}

/**
 * Recursively applies the indicated collide mask to the nodes at and below
 * this node.
 */
void NodePath::
r_set_collide_mask(PandaNode *node,
                   CollideMask and_mask, CollideMask or_mask,
                   TypeHandle node_type) {
  if (node->is_of_type(node_type)) {
    CollideMask into_collide_mask = node->get_into_collide_mask();
    into_collide_mask = (into_collide_mask & and_mask) | or_mask;
    node->set_into_collide_mask(into_collide_mask);
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_set_collide_mask(cr.get_child(i), and_mask, or_mask, node_type);
  }
}

/**
 *
 */
bool NodePath::
r_has_vertex_column(PandaNode *node, const InternalName *name) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_R(gnode, node, false);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      const Geom *geom = gnode->get_geom(i);
      CPT(GeomVertexData) vdata = geom->get_vertex_data();
      if (vdata->has_column(name)) {
        return true;
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    if (r_has_vertex_column(child, name)) {
      return true;
    }
  }

  return false;
}

/**
 *
 */
void NodePath::
r_find_all_vertex_columns(PandaNode *node,
                          NodePath::InternalNames &vertex_columns) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; ++i) {
      const Geom *geom = gnode->get_geom(i);
      const GeomVertexFormat *format = geom->get_vertex_data()->get_format();
      int num_arrays = format->get_num_arrays();
      for (int j = 0; j < num_arrays; ++j) {
        const GeomVertexArrayFormat *array = format->get_array(j);
        int num_columns = array->get_num_columns();
        for (int k = 0; k < num_columns; ++k) {
          const GeomVertexColumn *column = array->get_column(k);
          vertex_columns.insert(column->get_name());
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    r_find_all_vertex_columns(child, vertex_columns);
  }
}

/**
 *
 */
Texture *NodePath::
r_find_texture(PandaNode *node, const RenderState *state,
               const GlobPattern &glob) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_R(gnode, node, nullptr);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        for (int i = 0; i < ta->get_num_on_stages(); i++) {
          Texture *texture = ta->get_on_texture(ta->get_on_stage(i));
          if (texture != nullptr) {
            if (glob.matches(texture->get_name())) {
              return texture;
            }
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());

    Texture *result = r_find_texture(child, next_state, glob);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 *
 */
void NodePath::
r_find_all_textures(PandaNode *node, const RenderState *state,
                    NodePath::Textures &textures) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        for (int i = 0; i < ta->get_num_on_stages(); i++) {
          Texture *texture = ta->get_on_texture(ta->get_on_stage(i));
          if (texture != nullptr) {
            textures.insert(texture);
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());
    r_find_all_textures(child, next_state, textures);
  }
}

/**
 *
 */
Texture * NodePath::
r_find_texture(PandaNode *node, TextureStage *stage) const {
  // Look for a TextureAttrib on the node.
  const RenderAttrib *attrib =
    node->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    if (ta->has_on_stage(stage)) {
      return ta->get_on_texture(stage);
    }
  }

  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_R(gnode, node, nullptr);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state = gnode->get_geom_state(i);

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        if (ta->has_on_stage(stage)) {
          return ta->get_on_texture(stage);
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);

    Texture *result = r_find_texture(child, stage);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 *
 */
void NodePath::
r_find_all_textures(PandaNode *node, TextureStage *stage,
                    NodePath::Textures &textures) const {
  // Look for a TextureAttrib on the node.
  const RenderAttrib *attrib =
    node->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    if (ta->has_on_stage(stage)) {
      textures.insert(ta->get_on_texture(stage));
    }
  }

  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state = gnode->get_geom_state(i);

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        if (ta->has_on_stage(stage)) {
          textures.insert(ta->get_on_texture(stage));
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    r_find_all_textures(child, stage, textures);
  }
}

/**
 * Recursively replaces references to the given texture on this section of the
 * scene graph with the given other texture.
 */
void NodePath::
r_replace_texture(PandaNode *node, Texture *tex, Texture *new_tex) {
  // Consider the state of the node itself.
  {
    CPT(RenderState) node_state = node->get_state();
    const TextureAttrib *ta;
    if (node_state->get_attrib(ta)) {
      CPT(RenderAttrib) new_ta = ta->replace_texture(tex, new_tex);
      if (new_ta != ta) {
        node->set_state(node_state->set_attrib(new_ta));
      }
    }
  }

  // If this is a GeomNode, consider the state of any of its Geoms.
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state = gnode->get_geom_state(i);

      // Look for a TextureAttrib on the state.
      const TextureAttrib *ta;
      if (geom_state->get_attrib(ta)) {
        CPT(RenderAttrib) new_ta = ta->replace_texture(tex, new_tex);
        if (new_ta != ta) {
          gnode->set_geom_state(i, geom_state->set_attrib(new_ta));
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  size_t num_children = cr.get_num_children();
  for (size_t i = 0; i < num_children; ++i) {
    PandaNode *child = cr.get_child(i);
    r_replace_texture(child, tex, new_tex);
  }
}

/**
 *
 */
TextureStage * NodePath::
r_find_texture_stage(PandaNode *node, const RenderState *state,
                     const GlobPattern &glob) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_R(gnode, node, nullptr);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        for (int i = 0; i < ta->get_num_on_stages(); i++) {
          TextureStage *texture_stage = ta->get_on_stage(i);
          if (texture_stage != nullptr) {
            if (glob.matches(texture_stage->get_name())) {
              return texture_stage;
            }
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());

    TextureStage *result = r_find_texture_stage(child, next_state, glob);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 *
 */
void NodePath::
r_find_all_texture_stages(PandaNode *node, const RenderState *state,
                          NodePath::TextureStages &texture_stages) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        for (int i = 0; i < ta->get_num_on_stages(); i++) {
          TextureStage *texture_stage = ta->get_on_stage(i);
          if (texture_stage != nullptr) {
            texture_stages.insert(texture_stage);
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());
    r_find_all_texture_stages(child, next_state, texture_stages);
  }
}

/**
 *
 */
void NodePath::
r_unify_texture_stages(PandaNode *node, TextureStage *stage) {
  // Look for a TextureAttrib on the state.
  const RenderAttrib *attrib =
    node->get_attrib(TextureAttrib::get_class_slot());
  if (attrib != nullptr) {
    const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
    CPT(RenderAttrib) new_attrib = ta->unify_texture_stages(stage);
    if (new_attrib != ta) {
      node->set_attrib(new_attrib);
    }
  }

  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) state = gnode->get_geom_state(i);

      // Look for a TextureAttrib on the state.
      const RenderAttrib *attrib =
        state->get_attrib(TextureAttrib::get_class_slot());
      if (attrib != nullptr) {
        const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
        CPT(RenderAttrib) new_attrib = ta->unify_texture_stages(stage);
        if (new_attrib != ta) {
          CPT(RenderState) new_state = state->add_attrib(new_attrib);
          gnode->set_geom_state(i, new_state);
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    r_unify_texture_stages(child, stage);
  }
}

/**
 *
 */
Material *NodePath::
r_find_material(PandaNode *node, const RenderState *state,
               const GlobPattern &glob) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_R(gnode, node, nullptr);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a MaterialAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(MaterialAttrib::get_class_slot());
      if (attrib != nullptr) {
        const MaterialAttrib *ta = DCAST(MaterialAttrib, attrib);
        if (!ta->is_off()) {
          Material *material = ta->get_material();
          if (material != nullptr) {
            if (glob.matches(material->get_name())) {
              return material;
            }
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());

    Material *result = r_find_material(child, next_state, glob);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 *
 */
void NodePath::
r_find_all_materials(PandaNode *node, const RenderState *state,
                    NodePath::Materials &materials) const {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state =
        state->compose(gnode->get_geom_state(i));

      // Look for a MaterialAttrib on the state.
      const RenderAttrib *attrib =
        geom_state->get_attrib(MaterialAttrib::get_class_slot());
      if (attrib != nullptr) {
        const MaterialAttrib *ta = DCAST(MaterialAttrib, attrib);
        if (!ta->is_off()) {
          Material *material = ta->get_material();
          if (material != nullptr) {
            materials.insert(material);
          }
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = cr.get_child(i);
    CPT(RenderState) next_state = state->compose(child->get_state());
    r_find_all_materials(child, next_state, materials);
  }
}

/**
 *
 */
void NodePath::
r_replace_material(PandaNode *node, Material *mat,
                   const MaterialAttrib *new_attrib) {
  // Consider the state of the node itself.
  {
    CPT(RenderState) node_state = node->get_state();
    const MaterialAttrib *ma;
    if (node_state->get_attrib(ma)) {
      if (mat == ma->get_material()) {
        node->set_state(node_state->set_attrib(new_attrib));
      }
    }
  }

  // If this is a GeomNode, consider the state of any of its Geoms.
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(RenderState) geom_state = gnode->get_geom_state(i);

      // Look for a MaterialAttrib on the state.
      const MaterialAttrib *ma;
      if (geom_state->get_attrib(ma)) {
        if (mat == ma->get_material()) {
          // Replace it
          gnode->set_geom_state(i, geom_state->set_attrib(new_attrib));
        }
      }
    }
  }

  // Now consider children.
  PandaNode::Children cr = node->get_children();
  size_t num_children = cr.get_num_children();
  for (size_t i = 0; i < num_children; ++i) {
    PandaNode *child = cr.get_child(i);
    r_replace_material(child, mat, new_attrib);
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void NodePath::
write_datagram(BamWriter *manager, Datagram &dg) const {
  if (is_empty()) {
    manager->write_pointer(dg, nullptr);
    return;
  }

  PandaNode *root = DCAST(PandaNode, manager->get_root_node());

  // We have no root node to measure from.
  if (root == nullptr || root == node()) {
    manager->write_pointer(dg, node());
    manager->write_pointer(dg, nullptr);
    return;
  }

  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();

  // Record the chain of nodes from the root to this node.
  pvector<PandaNode *> path;
  NodePathComponent *comp = _head;
  while (comp != nullptr) {
    PandaNode *node = comp->get_node();
    path.push_back(node);

    if (node == root) {
      break;
    }

    comp = comp->get_next(pipeline_stage, current_thread);
  }

  if (comp == nullptr) {
    // We did not encounter the root node.  Not much we can do.
    manager->write_pointer(dg, node());
    manager->write_pointer(dg, nullptr);
    return;
  }

  // Write out the nodes in reverse order, for fast reconstructing.
  for (int i = path.size() - 1; i >= 0; --i) {
    manager->write_pointer(dg, path[i]);
  }
  manager->write_pointer(dg, nullptr);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int NodePath::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;
  PT(PandaNode) node = DCAST(PandaNode, p_list[pi++]);
  if (node.is_null()) {
    // An empty NodePath.
    _head = nullptr;
    return pi;
  }

  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();

  // Take an arbitrary path to the root of the NodePath.  This probably won't
  // be ambiguous, as this is usually the root of the model or scene we are
  // currently loading.
  PT(NodePathComponent) comp = node->get_generic_component(false, pipeline_stage, current_thread);
  nassertd(!comp.is_null()) {
    while (p_list[pi++]) {}
    return pi;
  }

  // Build up the chain of NodePathComponents leading up to this node.
  while (p_list[pi] != nullptr) {
    PT(PandaNode) node = DCAST(PandaNode, p_list[pi++]);

    LightReMutexHolder holder(node->_paths_lock);

    // First, walk through the list of NodePathComponents we already have on
    // the child, looking for one that already exists, referencing the
    // indicated parent component.
    PandaNode::Paths::const_iterator it;
    for (it = node->_paths.begin(); it != node->_paths.end(); ++it) {
      if ((*it)->get_next(pipeline_stage, current_thread) == comp) {
        // If we already have such a component, use that.
        comp = (*it);
        break;
      }
    }

    if (it == node->_paths.end()) {
      // We don't already have a NodePathComponent referring to this parent-
      // child relationship.  Create a new one.  Note that we can't verify
      // that they are actually related because we may not have completed the
      // node's pointers yet, so we trust that the .bam is right.
      comp = new NodePathComponent(node, comp, pipeline_stage, current_thread);
      node->_paths.insert(comp);
    }
  }
  // One more for the final NULL node.
  ++pi;

  _head = comp;
  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new NodePath.
 */
void NodePath::
fillin(DatagramIterator &scan, BamReader *manager) {
  while(manager->read_pointer(scan)) {};
}
