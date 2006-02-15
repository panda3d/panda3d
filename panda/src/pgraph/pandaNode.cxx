// Filename: pandaNode.cxx
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandaNode.h"
#include "config_pgraph.h"
#include "nodePathComponent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"
#include "geometricBoundingVolume.h"
#include "sceneGraphReducer.h"
#include "accumulatedAttribs.h"
#include "clipPlaneAttrib.h"
#include "boundingSphere.h"

TypeHandle PandaNode::_type_handle;

//
// There are two different interfaces here for making and breaking
// parent-child connections: the fundamental PandaNode interface, via
// add_child() and remove_child() (and related functions), and the
// NodePath support interface, via attach(), detach(), and reparent().
// They both do essentially the same thing, but with slightly
// different inputs.  The PandaNode interfaces try to guess which
// NodePaths should be updated as a result of the scene graph change,
// while the NodePath interfaces already know.
//
// The NodePath support interface functions are strictly called from
// within the NodePath class, and are used to implement
// NodePath::reparent_to() and NodePath::remove_node(), etc.  The
// fundamental interface, on the other hand, is intended to be called
// directly by the user.
//
// The fundamental interface has a slightly lower overhead because it
// does not need to create a NodePathComponent chain where one does
// not already exist; however, the NodePath support interface is more
// useful when the NodePath already does exist, because it ensures
// that the particular NodePath calling it is kept appropriately
// up-to-date.
//


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const string &name) :
  Namable(name)
{
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Constructing " << (void *)this << ", " << get_name() << "\n";
  }
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
~PandaNode() {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Destructing " << (void *)this << ", " << get_name() << "\n";
  }

  // We shouldn't have any parents left by the time we destruct, or
  // there's a refcount fault somewhere.
#ifndef NDEBUG
  {
    CDLinksReader cdata(_cycler_links);
    nassertv(cdata->_up.empty());
  }
#endif  // NDEBUG

  remove_all_children();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Copy Constructor
//       Access: Protected
//  Description: Do not call the copy constructor directly; instead,
//               use make_copy() or copy_subgraph() to make a copy of
//               a node.
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const PandaNode &copy) :
  ReferenceCount(copy),
  TypedWritable(copy),
  Namable(copy)
{
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Copying " << (void *)this << ", " << get_name() << "\n";
  }
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  // Copying a node does not copy its children.

  // Copy the other node's state.
  {
    CDLightReader copy_cdata(copy._cycler_light);
    CDLightWriter cdata(_cycler_light);
    cdata->_state = copy_cdata->_state;
    cdata->_transform = copy_cdata->_transform;
    cdata->_prev_transform = copy_cdata->_prev_transform;
  }
  {
    CDHeavyReader copy_cdata(copy._cycler_heavy);
    CDHeavyWriter cdata(_cycler_heavy);
    cdata->_effects = copy_cdata->_effects;
    cdata->_tag_data = copy_cdata->_tag_data;
    cdata->_draw_mask = copy_cdata->_draw_mask;
    cdata->_into_collide_mask = copy_cdata->_into_collide_mask;
    cdata->_user_bounds = copy_cdata->_user_bounds;
    cdata->_internal_bounds = NULL;
    cdata->_internal_bounds_stale = true;
    cdata->_final_bounds = copy_cdata->_final_bounds;
    
#ifdef HAVE_PYTHON
    // Copy and increment all of the Python objects held by the other
    // node.
    cdata->_python_tag_data = copy_cdata->_python_tag_data;
    cdata->inc_py_refs();
#endif  // HAVE_PYTHON
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Copy Assignment Operator
//       Access: Private
//  Description: Do not call the copy assignment operator at all.  Use
//               make_copy() or copy_subgraph() to make a copy of a
//               node.
////////////////////////////////////////////////////////////////////
void PandaNode::
operator = (const PandaNode &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *PandaNode::
make_copy() const {
  return new PandaNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of PandaNode by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_flatten() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of PandaNode by calling the
//               xform() method, false otherwise.  For instance, it's
//               usually a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_transform() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_modify_transform
//       Access: Public, Virtual
//  Description: Returns true if it is safe to automatically adjust
//               the transform on this kind of node.  Usually, this is
//               only a bad idea if the user expects to find a
//               particular transform on the node.
//
//               ModelNodes with the preserve_transform flag set are
//               presently the only kinds of nodes that should not
//               have their transform even adjusted.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_modify_transform() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes, adding children or whatever.  For
//               instance, an LODNode should not be combined with any
//               other PandaNode, because its set of children is
//               meaningful.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_combine() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_flatten_below
//       Access: Public, Virtual
//  Description: Returns true if a flatten operation may safely
//               continue past this node, or false if nodes below this
//               node may not be molested.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_flatten_below() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::preserve_name
//       Access: Public, Virtual
//  Description: Returns true if the node's name has extrinsic meaning
//               and must be preserved across a flatten operation,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PandaNode::
preserve_name() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_unsafe_to_apply_attribs
//       Access: Public, Virtual
//  Description: Returns the union of all attributes from
//               SceneGraphReducer::AttribTypes that may not safely be
//               applied to the vertices of this node.  If this is
//               nonzero, these attributes must be dropped at this
//               node as a state change.
//
//               This is a generalization of safe_to_transform().
////////////////////////////////////////////////////////////////////
int PandaNode::
get_unsafe_to_apply_attribs() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::apply_attribs_to_vertices
//       Access: Public, Virtual
//  Description: Applies whatever attributes are specified in the
//               AccumulatedAttribs object (and by the attrib_types
//               bitmask) to the vertices on this node, if
//               appropriate.  If this node uses geom arrays like a
//               GeomNode, the supplied GeomTransformer may be used to
//               unify shared arrays across multiple different nodes.
//
//               This is a generalization of xform().
////////////////////////////////////////////////////////////////////
void PandaNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    xform(attribs._transform->get_mat());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PandaNode::
xform(const LMatrix4f &) {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::combine_with
//       Access: Public, Virtual
//  Description: Collapses this PandaNode with the other PandaNode, if
//               possible, and returns a pointer to the combined
//               PandaNode, or NULL if the two PandaNodes cannot
//               safely be combined.
//
//               The return value may be this, other, or a new
//               PandaNode altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two PandaNodes and
//               what the collapsed PandaNode should look like.
////////////////////////////////////////////////////////////////////
PandaNode *PandaNode::
combine_with(PandaNode *other) {
  // This is a little bit broken right now w.r.t. NodePaths, since any
  // NodePaths attached to the lost node will simply be disconnected.
  // This isn't the right thing to do; we should collapse those
  // NodePaths with these NodePaths instead.  To do this properly, we
  // will need to combine this functionality with that of stealing the
  // other node's children into one method.  Not too difficult, but
  // there are more pressing problems to work on right now.


  // An unadorned PandaNode always combines with any other PandaNodes by
  // yielding completely.  However, if we are actually some fancy PandaNode
  // type that derives from PandaNode but didn't redefine this function, we
  // should refuse to combine.
  if (is_exact_type(get_class_type())) {
    // No, we're an ordinary PandaNode.
    return other;

  } else if (other->is_exact_type(get_class_type())) {
    // We're not an ordinary PandaNode, but the other one is.
    return this;
  }

  // We're something other than an ordinary PandaNode.  Don't combine.
  return (PandaNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::calc_tight_bounds
//       Access: Public, Virtual
//  Description: This is used to support
//               NodePath::calc_tight_bounds().  It is not intended to
//               be called directly, and it has nothing to do with the
//               normal Panda bounding-volume computation.
//
//               If the node contains any geometry, this updates
//               min_point and max_point to enclose its bounding box.
//               found_any is to be set true if the node has any
//               geometry at all, or left alone if it has none.  This
//               method may be called over several nodes, so it may
//               enter with min_point, max_point, and found_any
//               already set.
//
//               This function is recursive, and the return value is
//               the transform after it has been modified by this
//               node's transform.
////////////////////////////////////////////////////////////////////
CPT(TransformState) PandaNode::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, bool &found_any,
                  const TransformState *transform) const {
  CPT(TransformState) next_transform = transform->compose(get_transform());

  Children cr = get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    cr.get_child(i)->calc_tight_bounds(min_point, max_point,
                                       found_any, next_transform);
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PandaNode::
has_cull_callback() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool PandaNode::
cull_callback(CullTraverser *, CullTraverserData &) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_selective_visibility
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if this kind of node has some restrictions on
//               the set of children that should be rendered.  Node
//               with this property include LODNodes, SwitchNodes, and
//               SequenceNodes.
//
//               If this function returns true,
//               get_first_visible_child() and
//               get_next_visible_child() will be called to walk
//               through the list of children during cull, instead of
//               iterating through the entire list.  This method is
//               called after cull_callback(), so cull_callback() may
//               be responsible for the decisions as to which children
//               are visible at the moment.
////////////////////////////////////////////////////////////////////
bool PandaNode::
has_selective_visibility() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_first_visible_child
//       Access: Public, Virtual
//  Description: Returns the index number of the first visible child
//               of this node, or a number >= get_num_children() if
//               there are no visible children of this node.  This is
//               called during the cull traversal, but only if
//               has_selective_visibility() has already returned true.
//               See has_selective_visibility().
////////////////////////////////////////////////////////////////////
int PandaNode::
get_first_visible_child() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_next_visible_child
//       Access: Public, Virtual
//  Description: Returns the index number of the next visible child
//               of this node following the indicated child, or a
//               number >= get_num_children() if there are no more
//               visible children of this node.  See
//               has_selective_visibility() and
//               get_first_visible_child().
////////////////////////////////////////////////////////////////////
int PandaNode::
get_next_visible_child(int n) const {
  return n + 1;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_single_child_visibility
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if this kind of node has the special property
//               that just one of its children is visible at any given
//               time, and furthermore that the particular visible
//               child can be determined without reference to any
//               external information (such as a camera).  At present,
//               only SequenceNodes and SwitchNodes fall into this
//               category.
//
//               If this function returns true, get_visible_child()
//               can be called to return the index of the
//               currently-visible child.
////////////////////////////////////////////////////////////////////
bool PandaNode::
has_single_child_visibility() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_visible_child
//       Access: Public, Virtual
//  Description: Returns the index number of the currently visible
//               child of this node.  This is only meaningful if
//               has_single_child_visibility() has returned true.
////////////////////////////////////////////////////////////////////
int PandaNode::
get_visible_child() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::copy_subgraph
//       Access: Published
//  Description: Allocates and returns a complete copy of this
//               PandaNode and the entire scene graph rooted at this
//               PandaNode.  Some data may still be shared from the
//               original (e.g. vertex index tables), but nothing that
//               will impede normal use of the PandaNode.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PandaNode::
copy_subgraph() const {
  InstanceMap inst_map;
  return r_copy_subgraph(inst_map);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::add_child
//       Access: Published
//  Description: Adds a new child to the node.  The child is added in
//               the relative position indicated by sort; if all
//               children have the same sort index, the child is added
//               at the end.
//
//               If the same child is added to a node more than once,
//               the previous instance is first removed.
////////////////////////////////////////////////////////////////////
void PandaNode::
add_child(PandaNode *child_node, int sort) {
  nassertv(child_node != (PandaNode *)NULL);
  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);

  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links) {
    CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
    CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
    
    cdata->_down.insert(DownConnection(child_node, sort));
    cdata_child->_up.insert(UpConnection(this));
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links);

  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links) {
    new_connection(this, child_node, pipeline_stage);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links);

  force_bounds_stale();
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the nth child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_child(int child_index) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);

  CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
  nassertv(child_index >= 0 && child_index < (int)cdata->_down.size());
  
  PT(PandaNode) child_node = cdata->_down[child_index].get_child();
  CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);

  cdata->_down.erase(cdata->_down.begin() + child_index);
  int num_erased = cdata_child->_up.erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node, pipeline_stage);
  force_bounds_stale(pipeline_stage);

  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the indicated child from the node.  Returns
//               true if the child was removed, false if it was not
//               already a child of the node.  This will also
//               successfully remove the child if it had been stashed.
////////////////////////////////////////////////////////////////////
bool PandaNode::
remove_child(PandaNode *child_node) {
  nassertr(child_node != (PandaNode *)NULL, false);
  
  // Make sure the child node is not destructed during the execution
  // of this method.
  PT(PandaNode) keep_child = child_node;

  // We have to do this for each upstream pipeline stage.
  bool any_removed = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links) {
    if (stage_remove_child(child_node, pipeline_stage)) {
      any_removed = true;

      sever_connection(this, child_node, pipeline_stage);
      force_bounds_stale(pipeline_stage);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links);

  if (any_removed) {
    // Call callback hooks.
    children_changed();
    child_node->parents_changed();
  }

  return any_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::replace_child
//       Access: Published
//  Description: Searches for the orig_child node in the node's list
//               of children, and replaces it with the new_child
//               instead.  Returns true if the replacement is made, or
//               false if the node is not a child.
////////////////////////////////////////////////////////////////////
bool PandaNode::
replace_child(PandaNode *orig_child, PandaNode *new_child) {
  nassertr(orig_child != (PandaNode *)NULL, false);
  nassertr(new_child != (PandaNode *)NULL, false);

  if (orig_child == new_child) {
    // Trivial no-op.
    return true;
  }
  
  // Make sure the orig_child node is not destructed during the
  // execution of this method.
  PT(PandaNode) keep_orig_child = orig_child;

  // We have to do this for each upstream pipeline stage.
  bool any_replaced = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links) {
    if (stage_replace_child(orig_child, new_child, pipeline_stage)) {
      any_replaced = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links);

  if (any_replaced) {
    children_changed();
    orig_child->parents_changed();
    new_child->parents_changed();
  }

  return any_replaced;
}


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::stash_child
//       Access: Published
//  Description: Stashes the indicated child node.  This removes the
//               child from the list of active children and puts it on
//               a special list of stashed children.  This child node
//               no longer contributes to the bounding volume of the
//               PandaNode, and is not visited in normal traversals.
//               It is invisible and uncollidable.  The child may
//               later be restored by calling unstash_child().
//
//               This can only be called from the top pipeline stage
//               (i.e. from App).
////////////////////////////////////////////////////////////////////
void PandaNode::
stash_child(int child_index) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  nassertv(child_index >= 0 && child_index < get_num_children());

  // Save a reference count for ourselves.
  PT(PandaNode) self = this;

  PT(PandaNode) child_node = get_child(child_index);
  int sort = get_child_sort(child_index);
  
  remove_child(child_index);

  {
    CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
    CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
    
    cdata->_stashed.insert(DownConnection(child_node, sort));
    cdata_child->_up.insert(UpConnection(this));
  }

  new_connection(this, child_node, pipeline_stage);
  force_bounds_stale(pipeline_stage);

  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::unstash_child
//       Access: Published
//  Description: Returns the indicated stashed node to normal child
//               status.  This removes the child from the list of
//               stashed children and puts it on the normal list of
//               active children.  This child node once again
//               contributes to the bounding volume of the PandaNode,
//               and will be visited in normal traversals.  It is
//               visible and collidable.
//
//               This can only be called from the top pipeline stage
//               (i.e. from App).
////////////////////////////////////////////////////////////////////
void PandaNode::
unstash_child(int stashed_index) { 
  nassertv(Thread::get_current_pipeline_stage() == 0);
  nassertv(stashed_index >= 0 && stashed_index < get_num_stashed());

  // Save a reference count for ourselves.  I don't think this should
  // be necessary, but there are occasional crashes in stash() during
  // furniture moving mode.  Perhaps this will eliminate those
  // crashes.
  PT(PandaNode) self = this;

  PT(PandaNode) child_node = get_stashed(stashed_index);
  int sort = get_stashed_sort(stashed_index);
  
  remove_stashed(stashed_index);
  
  {
    CDLinksWriter cdata(_cycler_links);
    CDLinksWriter cdata_child(child_node->_cycler_links);
    
    cdata->_down.insert(DownConnection(child_node, sort));
    cdata_child->_up.insert(UpConnection(this));
  }

  int pipeline_stage = Thread::get_current_pipeline_stage();
  new_connection(this, child_node, pipeline_stage);

  force_bounds_stale();
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::add_stashed
//       Access: Published
//  Description: Adds a new child to the node, directly as a stashed
//               child.  The child is not added in the normal sense,
//               but will be revealed if unstash_child() is called on
//               it later.
//
//               If the same child is added to a node more than once,
//               the previous instance is first removed.
//
//               This can only be called from the top pipeline stage
//               (i.e. from App).
////////////////////////////////////////////////////////////////////
void PandaNode::
add_stashed(PandaNode *child_node, int sort) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);

  {
    CDLinksWriter cdata(_cycler_links);
    CDLinksWriter cdata_child(child_node->_cycler_links);
    
    cdata->_stashed.insert(DownConnection(child_node, sort));
    cdata_child->_up.insert(UpConnection(this));
  }
    
  int pipeline_stage = Thread::get_current_pipeline_stage();
  new_connection(this, child_node, pipeline_stage);

  // Call callback hooks.
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_stashed
//       Access: Published
//  Description: Removes the nth stashed child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_stashed(int child_index) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);

  CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
  nassertv(child_index >= 0 && child_index < (int)cdata->_stashed.size());
  
  PT(PandaNode) child_node = cdata->_stashed[child_index].get_child();
  CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);

  cdata->_stashed.erase(cdata->_stashed.begin() + child_index);
  int num_erased = cdata_child->_up.erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node, pipeline_stage);
  force_bounds_stale(pipeline_stage);

  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_all_children
//       Access: Published
//  Description: Removes all the children from the node at once,
//               including stashed children.
//
//               This can only be called from the top pipeline stage
//               (i.e. from App).
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_all_children() {
  // We have to do this for each upstream pipeline stage.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links) {
    CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
    
    Down::iterator di;
    for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
      PT(PandaNode) child_node = (*di).get_child();
      CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
      cdata_child->_up.erase(UpConnection(this));
      
      sever_connection(this, child_node, pipeline_stage);
      child_node->parents_changed();
    }
    cdata->_down.clear();
    
    for (di = cdata->_stashed.begin(); di != cdata->_stashed.end(); ++di) {
      PT(PandaNode) child_node = (*di).get_child();
      CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
      cdata_child->_up.erase(UpConnection(this));
      
      sever_connection(this, child_node, pipeline_stage);
      child_node->parents_changed();
    }
    cdata->_stashed.clear();
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_links);

  force_bounds_stale();
  children_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::steal_children
//       Access: Published
//  Description: Moves all the children from the other node onto this
//               node.
////////////////////////////////////////////////////////////////////
void PandaNode::
steal_children(PandaNode *other) {
  if (other == this) {
    // Trivial.
    return;
  }

  // We do this through the high-level interface for convenience.
  // This could begin to be a problem if we have a node with hundreds
  // of children to copy; this could break down the ov_set.insert()
  // method, which is an O(n^2) operation.  If this happens, we should
  // rewrite this to do a simpler add_child() operation that involves
  // push_back() instead of insert(), and then sort the down list at
  // the end.

  int num_children = other->get_num_children();
  int i;
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = other->get_child(i);
    int sort = other->get_child_sort(i);
    add_child(child_node, sort);
  }
  int num_stashed = other->get_num_stashed();
  for (i = 0; i < num_stashed; i++) {
    PandaNode *child_node = other->get_stashed(i);
    int sort = other->get_stashed_sort(i);
    add_stashed(child_node, sort);
  }

  other->remove_all_children();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::copy_children
//       Access: Published
//  Description: Makes another instance of all the children of the
//               other node, copying them to this node.
////////////////////////////////////////////////////////////////////
void PandaNode::
copy_children(PandaNode *other) {
  if (other == this) {
    // Trivial.
    return;
  }
  int num_children = other->get_num_children();
  int i;
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = other->get_child(i);
    int sort = other->get_child_sort(i);
    add_child(child_node, sort);
  }
  int num_stashed = other->get_num_stashed();
  for (i = 0; i < num_stashed; i++) {
    PandaNode *child_node = other->get_stashed(i);
    int sort = other->get_stashed_sort(i);
    add_stashed(child_node, sort);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_attrib
//       Access: Published
//  Description: Adds the indicated render attribute to the scene
//               graph on this node.  This attribute will now apply to
//               this node and everything below.  If there was already
//               an attribute of the same type, it is replaced.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_attrib(const RenderAttrib *attrib, int override) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageWriter cdata(_cycler_light, pipeline_stage);
    
    CPT(RenderState) new_state = cdata->_state->add_attrib(attrib, override);
    if (cdata->_state != new_state) {
      cdata->_state = new_state;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);

  // Maybe we changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale();
    state_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_attrib
//       Access: Published
//  Description: Removes the render attribute of the given type from
//               this node.  This node, and the subgraph below, will
//               now inherit the indicated render attribute from the
//               nodes above this one.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_attrib(TypeHandle type) {
  bool any_changed = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageWriter cdata(_cycler_light, pipeline_stage);
    
    CPT(RenderState) new_state = cdata->_state->remove_attrib(type);
    if (cdata->_state != new_state) {
      cdata->_state = new_state;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);

  // We mark the bounds stale when the state changes, in case
  // we have changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale();
    state_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_effect
//       Access: Published
//  Description: Adds the indicated render effect to the scene
//               graph on this node.  If there was already an effect
//               of the same type, it is replaced.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_effect(const RenderEffect *effect) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    cdata->_effects = cdata->_effects->add_effect(effect);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_effect
//       Access: Published
//  Description: Removes the render effect of the given type from
//               this node.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_effect(TypeHandle type) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    cdata->_effects = cdata->_effects->remove_effect(type);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_state
//       Access: Published
//  Description: Sets the complete RenderState that will be applied to
//               all nodes at this level and below.  (The actual state
//               that will be applied to lower nodes is based on the
//               composition of RenderStates from above this node as
//               well).  This completely replaces whatever has been
//               set on this node via repeated calls to set_attrib().
////////////////////////////////////////////////////////////////////
void PandaNode::
set_state(const RenderState *state) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageWriter cdata(_cycler_light, pipeline_stage);
    if (cdata->_state != state) {
      cdata->_state = state;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);

  // Maybe we have changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale();
    state_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_effects
//       Access: Published
//  Description: Sets the complete RenderEffects that will be applied
//               this node.  This completely replaces whatever has
//               been set on this node via repeated calls to
//               set_attrib().
////////////////////////////////////////////////////////////////////
void PandaNode::
set_effects(const RenderEffects *effects) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    cdata->_effects = effects;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_transform
//       Access: Published
//  Description: Sets the transform that will be applied to this node
//               and below.  This defines a new coordinate space at
//               this point in the scene graph and below.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_transform(const TransformState *transform) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageWriter cdata(_cycler_light, pipeline_stage);
    if (cdata->_transform != transform) {
      cdata->_transform = transform;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);

  if (any_changed) {
    mark_bounds_stale();
    transform_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_prev_transform
//       Access: Published
//  Description: Sets the transform that represents this node's
//               "previous" position, one frame ago, for the purposes
//               of detecting motion for accurate collision
//               calculations.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_prev_transform(const TransformState *transform) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageWriter cdata(_cycler_light, pipeline_stage);
    cdata->_prev_transform = transform;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reset_prev_transform
//       Access: Published
//  Description: Resets the "previous" transform on this node to be
//               the same as the current transform.  This is not the
//               same as clearing it to identity.
////////////////////////////////////////////////////////////////////
void PandaNode::
reset_prev_transform() {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light) {
    CDLightStageReader cdata(_cycler_light, pipeline_stage);
    if (cdata->_prev_transform != cdata->_transform) {
      CDLightStageWriter cdataw(_cycler_light, pipeline_stage, cdata);
      cdataw->_prev_transform = cdataw->_transform;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_light);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_tag
//       Access: Published
//  Description: Associates a user-defined value with a user-defined
//               key which is stored on the node.  This value has no
//               meaning to Panda; but it is stored indefinitely on
//               the node until it is requested again.
//
//               Each unique key stores a different string value.
//               There is no effective limit on the number of
//               different keys that may be stored or on the length of
//               any one key's value.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_tag(const string &key, const string &value) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    cdata->_tag_data[key] = value;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_tag
//       Access: Published
//  Description: Removes the value defined for this key on this
//               particular node.  After a call to clear_tag(),
//               has_tag() will return false for the indicated key.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_tag(const string &key) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    cdata->_tag_data.erase(key);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_python_tag
//       Access: Published
//  Description: Associates an arbitrary Python object with a
//               user-defined key which is stored on the node.  This
//               is similar to set_tag(), except it can store any
//               Python object instead of just a string.  However, the
//               Python object is not recorded to a bam file.
//
//               Each unique key stores a different string value.
//               There is no effective limit on the number of
//               different keys that may be stored or on the length of
//               any one key's value.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_python_tag(const string &key, PyObject *value) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDHeavyWriter cdata(_cycler_heavy);
  Py_XINCREF(value);

  pair<PythonTagData::iterator, bool> result;
  result = cdata->_python_tag_data.insert(PythonTagData::value_type(key, value));

  if (!result.second) {
    // The insert was unsuccessful; that means the key was already
    // present in the map.  In this case, we should decrement the
    // original value's reference count and replace it with the new
    // object.
    PythonTagData::iterator ti = result.first;
    PyObject *old_value = (*ti).second;
    Py_XDECREF(old_value);
    (*ti).second = value;
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_python_tag
//       Access: Published
//  Description: Retrieves the Python object that was previously
//               set on this node for the particular key, if any.  If
//               no value has been previously set, returns None.
////////////////////////////////////////////////////////////////////
PyObject *PandaNode::
get_python_tag(const string &key) const {
  CDHeavyReader cdata(_cycler_heavy);
  PythonTagData::const_iterator ti;
  ti = cdata->_python_tag_data.find(key);
  if (ti != cdata->_python_tag_data.end()) {
    PyObject *result = (*ti).second;
    Py_XINCREF(result);
    return result;
  }
  return Py_None;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::has_python_tag
//       Access: Published
//  Description: Returns true if a Python object has been defined on
//               this node for the particular key (even if that object
//               is None), or false if no object has been set.
////////////////////////////////////////////////////////////////////
bool PandaNode::
has_python_tag(const string &key) const {
  CDHeavyReader cdata(_cycler_heavy);
  PythonTagData::const_iterator ti;
  ti = cdata->_python_tag_data.find(key);
  return (ti != cdata->_python_tag_data.end());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_python_tag
//       Access: Published
//  Description: Removes the Python object defined for this key on
//               this particular node.  After a call to
//               clear_python_tag(), has_python_tag() will return
//               false for the indicated key.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_python_tag(const string &key) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDHeavyWriter cdata(_cycler_heavy);
  PythonTagData::iterator ti;
  ti = cdata->_python_tag_data.find(key);
  if (ti != cdata->_python_tag_data.end()) {
    PyObject *value = (*ti).second;
    Py_XDECREF(value);
    cdata->_python_tag_data.erase(ti);
  }
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::copy_tags
//       Access: Published
//  Description: Copies all of the tags stored on the other node onto
//               this node.  If a particular tag exists on both nodes,
//               the contents of this node's value is replaced by that
//               of the other.
////////////////////////////////////////////////////////////////////
void PandaNode::
copy_tags(PandaNode *other) {
  if (other == this) {
    // Trivial.
    return;
  }

  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdataw(_cycler_heavy, pipeline_stage);
    CDHeavyStageReader cdatar(other->_cycler_heavy, pipeline_stage);
      
    TagData::const_iterator ti;
    for (ti = cdatar->_tag_data.begin();
         ti != cdatar->_tag_data.end();
         ++ti) {
      cdataw->_tag_data[(*ti).first] = (*ti).second;
    }
    
#ifdef HAVE_PYTHON
    PythonTagData::const_iterator pti;
    for (pti = cdatar->_python_tag_data.begin();
         pti != cdatar->_python_tag_data.end();
         ++pti) {
      const string &key = (*pti).first;
      PyObject *value = (*pti).second;
      Py_XINCREF(value);
      
      pair<PythonTagData::iterator, bool> result;
      result = cdataw->_python_tag_data.insert(PythonTagData::value_type(key, value));
      
      if (!result.second) {
        // The insert was unsuccessful; that means the key was already
        // present in the map.  In this case, we should decrement the
        // original value's reference count and replace it with the new
        // object.
        PythonTagData::iterator wpti = result.first;
        PyObject *old_value = (*wpti).second;
        Py_XDECREF(old_value);
        (*wpti).second = value;
      }
    }
#endif // HAVE_PYTHON
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::list_tags
//       Access: Published
//  Description: Writes a list of all the tag keys assigned to the
//               node to the indicated stream.  Writes one instance of
//               the separator following each key (but does not write
//               a terminal separator).  The value associated with
//               each key is not written.
//
//               This is mainly for the benefit of the realtime user,
//               to see the list of all of the associated tag keys.
////////////////////////////////////////////////////////////////////
void PandaNode::
list_tags(ostream &out, const string &separator) const {
  CDHeavyReader cdata(_cycler_heavy);
  if (!cdata->_tag_data.empty()) {
    TagData::const_iterator ti = cdata->_tag_data.begin();
    out << (*ti).first;
    ++ti;
    while (ti != cdata->_tag_data.end()) {
      out << separator << (*ti).first;
      ++ti;
    }
  }

#ifdef HAVE_PYTHON
  if (!cdata->_python_tag_data.empty()) {
    if (!cdata->_tag_data.empty()) {
      out << separator;
    }
    PythonTagData::const_iterator ti = cdata->_python_tag_data.begin();
    out << (*ti).first;
    ++ti;
    while (ti != cdata->_python_tag_data.end()) {
      out << separator << (*ti).first;
      ++ti;
    }
  }
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_draw_mask
//       Access: Published
//  Description: Sets the hide/show bits of this particular node.
//
//               During the cull traversal, a node is not visited if
//               none of its draw mask bits intersect with the
//               camera's draw mask bits.  These masks can be used to
//               selectively hide and show different parts of the
//               scene graph from different cameras that are otherwise
//               viewing the same scene.  See
//               Camera::set_camera_mask().
////////////////////////////////////////////////////////////////////
void PandaNode::
set_draw_mask(DrawMask mask) {
  bool any_changed = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    if (cdata->_draw_mask != mask) {
      cdata->_draw_mask = mask;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);

  if (any_changed) {
    draw_mask_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_into_collide_mask
//       Access: Published
//  Description: Sets the "into" CollideMask.  
//
//               This specifies the set of bits that must be shared
//               with a CollisionNode's "from" CollideMask in order
//               for the CollisionNode to detect a collision with this
//               particular node.
//
//               The actual CollideMask that will be set is masked by
//               the return value from get_legal_collide_mask().
//               Thus, the into_collide_mask cannot be set to anything
//               other than nonzero except for those types of nodes
//               that can be collided into, such as CollisionNodes and
//               GeomNodes.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_into_collide_mask(CollideMask mask) {
  mask &= get_legal_collide_mask();

  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    if (cdata->_into_collide_mask != mask) {
      cdata->_into_collide_mask = mask;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);

  if (any_changed) {
    mark_bounds_stale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_legal_collide_mask
//       Access: Published, Virtual
//  Description: Returns the subset of CollideMask bits that may be
//               set for this particular type of PandaNode.  For most
//               nodes, this is 0; it doesn't make sense to set a
//               CollideMask for most kinds of nodes.
//
//               For nodes that can be collided with, such as GeomNode
//               and CollisionNode, this returns all bits on.
////////////////////////////////////////////////////////////////////
CollideMask PandaNode::
get_legal_collide_mask() const {
  return CollideMask::all_off();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_net_collide_mask
//       Access: Published
//  Description: Returns the union of all into_collide_mask() values
//               set at CollisionNodes at this level and below.
////////////////////////////////////////////////////////////////////
CollideMask PandaNode::
get_net_collide_mask() const {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  CDBoundsStageReader cdata(_cycler_bounds, pipeline_stage);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CDBoundsStageWriter cdataw = 
      ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    return cdataw->_net_collide_mask;
  }
  return cdata->_net_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_off_clip_planes
//       Access: Published
//  Description: Returns a ClipPlaneAttrib which represents the union
//               of all of the clip planes that have been turned *off*
//               at this level and below.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) PandaNode::
get_off_clip_planes() const {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  CDBoundsStageReader cdata(_cycler_bounds, pipeline_stage);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CDBoundsStageWriter cdataw = 
      ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    return cdataw->_off_clip_planes;
  }
  return cdata->_off_clip_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PandaNode::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PandaNode::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this;
  if (has_tags()) {
    out << " [";
    list_tags(out, " ");
    out << "]";
  }
  CPT(TransformState) transform = get_transform();
  if (!transform->is_identity()) {
    out << " " << *transform;
  }
  CPT(RenderState) state = get_state();
  if (!state->is_empty()) {
    out << " " << *state;
  }
  CPT(RenderEffects) effects = get_effects();
  if (!effects->is_empty()) {
    out << " " << *effects;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_bounds
//       Access: Published
//  Description: Resets the bounding volume so that it is the
//               indicated volume.  When it is explicitly set, the
//               bounding volume will no longer be automatically
//               computed according to the contents of the node
//               itself, for nodes like GeomNodes and TextNodes that
//               contain substance (but the bounding volume will still
//               be automatically expanded to include its children).
//
//               Call clear_bounds() if you would like to return the
//               bounding volume to its default behavior later.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_bounds(const BoundingVolume *volume) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdata(_cycler_heavy, pipeline_stage);
    if (volume == NULL) {
      cdata->_user_bounds = NULL;
    } else {
      cdata->_user_bounds = volume->make_copy();
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
  
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_bound
//       Access: Published
//  Description: Deprecated.  Use set_bounds() instead.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_bound(const BoundingVolume *volume) {
  pgraph_cat.warning()
    << "Deprecated PandaNode::set_bound() called.  Use set_bounds() instead.\n";
  set_bounds(volume);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bounds
//       Access: Published
//  Description: Returns the external bounding volume of this node: a
//               bounding volume that contains the user bounding
//               volume, the internal bounding volume, and all of the
//               children's bounding volumes.
////////////////////////////////////////////////////////////////////
CPT(BoundingVolume) PandaNode::
get_bounds() const {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  CDBoundsStageReader cdata(_cycler_bounds, pipeline_stage);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CPT(BoundingVolume) result;
    {
      CDBoundsStageWriter cdataw = 
	((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
      result = cdataw->_external_bounds;
    }
    return result;
  }
  return cdata->_external_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_bounds_stale
//       Access: Published
//  Description: Indicates that the bounding volume, or something that
//               influences the bounding volume (or any of the other
//               things stored in CDataBounds, like net_collide_mask),
//               may have changed for this node, and that it must be
//               recomputed.
//
//               With no parameters, this means to iterate through all
//               stages including and upstream of the current pipeline
//               stage.
//
//               This method is intended for internal use; usually it
//               is not necessary for a user to call this directly.
//               It will be called automatically by derived classes
//               when appropriate.
////////////////////////////////////////////////////////////////////
void PandaNode::
mark_bounds_stale() const {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links) {
    mark_bounds_stale(pipeline_stage);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_geom_node
//       Access: Published, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from GeomNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(GeomNode::get_class_type()), since this
//               test is so important to rendering.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_geom_node() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_lod_node
//       Access: Published, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from LODNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(LODNode::get_class_type()).
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_lod_node() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::as_light
//       Access: Published, Virtual
//  Description: Cross-casts the node to a Light pointer, if it is one
//               of the four kinds of Light nodes, or returns NULL if
//               it is not.
////////////////////////////////////////////////////////////////////
Light *PandaNode::
as_light() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bounds
//       Access: Protected
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.
////////////////////////////////////////////////////////////////////
CPT(BoundingVolume) PandaNode::
get_internal_bounds(int pipeline_stage) const {
  CDHeavyStageReader cdata(_cycler_heavy, pipeline_stage);
  if (cdata->_internal_bounds_stale) {
    CDHeavyStageWriter cdataw(((PandaNode *)this)->_cycler_heavy, pipeline_stage, cdata);
    if (cdataw->_user_bounds != (BoundingVolume *)NULL) {
      cdataw->_internal_bounds = cdataw->_user_bounds;
    } else {
      cdataw->_internal_bounds = compute_internal_bounds(pipeline_stage);
    }
    cdataw->_internal_bounds_stale = false;
    return cdataw->_internal_bounds;
  }
  return cdata->_internal_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_internal_bounds
//       Access: Protected
//  Description: This is provided as an alternate way for a node to
//               set its own internal bounds, rather than overloading
//               compute_internal_bounds().  If this method is called,
//               the internal bounding volume will immediately be set
//               to the indicated pointer.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_internal_bounds(const BoundingVolume *volume) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy) {
    CDHeavyStageWriter cdataw(_cycler_heavy, pipeline_stage);
    cdataw->_internal_bounds = volume;
    cdataw->_internal_bounds_stale = false;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler_heavy);
  mark_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::force_bounds_stale
//       Access: Protected
//  Description: Similar to mark_bounds_stale(), except that the
//               parents of this node marked stale even if this node
//               was already considered stale.
//
//               With no parameters, this means to iterate through all
//               stages including and upstream of the current pipeline
//               stage.
////////////////////////////////////////////////////////////////////
void PandaNode::
force_bounds_stale() {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links) {
    force_bounds_stale(pipeline_stage);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::force_bounds_stale
//       Access: Protected
//  Description: Similar to mark_bounds_stale(), except that the
//               parents of this node marked stale even if this node
//               was already considered stale.
////////////////////////////////////////////////////////////////////
void PandaNode::
force_bounds_stale(int pipeline_stage) {
  {
    CDBoundsStageWriter cdata(_cycler_bounds, pipeline_stage);
    ++cdata->_next_update;

    // It is important that we allow this lock to be dropped before we
    // continue up the graph; otherwise, we risk deadlock from another
    // thread walking down the graph.
  }

  // It is similarly important that we use get_parents() here to copy
  // the parents list, instead of keeping the lock open while we walk
  // through the parents list directly on the node.
  ParentsCopy parents = get_parents_copy();
  int num_parents = parents.get_num_parents();
  for (int i = 0; i < num_parents; ++i) {
    PandaNode *parent = parents.get_parent(i);
    parent->mark_bounds_stale(pipeline_stage);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_internal_bounds_stale
//       Access: Protected
//  Description: Should be called by a derived class to mark the
//               internal bounding volume stale, so that
//               compute_internal_bounds() will be called when the
//               bounding volume is next requested.
//
//               With no parameters, this means to iterate through all
//               stages including and upstream of the current pipeline
//               stage.
////////////////////////////////////////////////////////////////////
void PandaNode::
mark_internal_bounds_stale() {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links) {
    mark_internal_bounds_stale(pipeline_stage);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler_links);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated BoundingVolume that
//               represents the internal contents of the node.  Should
//               be overridden by PandaNode classes that contain
//               something internally.
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) PandaNode::
compute_internal_bounds(int pipeline_stage) const {
  return new BoundingSphere;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::parents_changed
//       Access: Protected, Virtual
//  Description: Called after a scene graph update that either adds or
//               remove parents from this node, this just provides a
//               hook for derived PandaNode objects that need to
//               update themselves based on the set of parents the
//               node has.
////////////////////////////////////////////////////////////////////
void PandaNode::
parents_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::children_changed
//       Access: Protected, Virtual
//  Description: Called after a scene graph update that either adds or
//               remove children from this node, this just provides a
//               hook for derived PandaNode objects that need to
//               update themselves based on the set of children the
//               node has.
////////////////////////////////////////////////////////////////////
void PandaNode::
children_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::transform_changed
//       Access: Protected, Virtual
//  Description: Called after the node's transform has been changed
//               for any reason, this just provides a hook so derived
//               classes can do something special in this case.
////////////////////////////////////////////////////////////////////
void PandaNode::
transform_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::state_changed
//       Access: Protected, Virtual
//  Description: Called after the node's RenderState has been changed
//               for any reason, this just provides a hook so derived
//               classes can do something special in this case.
////////////////////////////////////////////////////////////////////
void PandaNode::
state_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::draw_mask_changed
//       Access: Protected, Virtual
//  Description: Called after the node's DrawMask has been changed
//               for any reason, this just provides a hook so derived
//               classes can do something special in this case.
////////////////////////////////////////////////////////////////////
void PandaNode::
draw_mask_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_copy_subgraph
//       Access: Protected, Virtual
//  Description: This is the recursive implementation of copy_subgraph().
//               It returns a copy of the entire subgraph rooted at
//               this node.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of PandaNode
//               that is not also a member of PANDA.DLL *cannot*
//               access this map.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PandaNode::
r_copy_subgraph(PandaNode::InstanceMap &inst_map) const {
  PT(PandaNode) copy = make_copy();
  nassertr(copy != (PandaNode *)NULL, NULL);
  if (copy->get_type() != get_type()) {
    pgraph_cat.warning()
      << "Don't know how to copy nodes of type " << get_type() << "\n";
  }

  copy->r_copy_children(this, inst_map);
  return copy;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_copy_children
//       Access: Protected, Virtual
//  Description: This is called by r_copy_subgraph(); the copy has
//               already been made of this particular node (and this
//               is the copy); this function's job is to copy all of
//               the children from the original.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of PandaNode
//               that is not also a member of PANDA.DLL *cannot*
//               access this map, and probably should not even
//               override this function.
////////////////////////////////////////////////////////////////////
void PandaNode::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map) {
  CDLinksReader from_cdata(from->_cycler_links);
  Down::const_iterator di;
  for (di = from_cdata->_down.begin(); di != from_cdata->_down.end(); ++di) {
    int sort = (*di).get_sort();
    PandaNode *source_child = (*di).get_child();
    PT(PandaNode) dest_child;

    // Check to see if we have already copied this child.  If we
    // have, use the copy.  In this way, a subgraph that contains
    // instances will be correctly duplicated into another subgraph
    // that also contains its own instances.
    InstanceMap::const_iterator ci;
    ci = inst_map.find(source_child);
    if (ci != inst_map.end()) {
      dest_child = (*ci).second;
    } else {
      dest_child = source_child->r_copy_subgraph(inst_map);
      inst_map[source_child] = dest_child;
    }

    add_child(dest_child, sort);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_find_child
//       Access: Private
//  Description: The private implementation of find_child().
////////////////////////////////////////////////////////////////////
int PandaNode::
do_find_child(PandaNode *node, const CDataLinks *cdata) const {
  nassertr(node != (PandaNode *)NULL, -1);

  // We have to search for the child by brute force, since we don't
  // know what sort index it was added as.
  Down::const_iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    if ((*di).get_child() == node) {
      return di - cdata->_down.begin();
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::do_find_stashed
//       Access: Private
//  Description: The private implementation of find_stashed().
////////////////////////////////////////////////////////////////////
int PandaNode::
do_find_stashed(PandaNode *node, const CDataLinks *cdata) const {
  nassertr(node != (PandaNode *)NULL, -1);

  // We have to search for the child by brute force, since we don't
  // know what sort index it was added as.
  Down::const_iterator di;
  for (di = cdata->_stashed.begin(); di != cdata->_stashed.end(); ++di) {
    if ((*di).get_child() == node) {
      return di - cdata->_stashed.begin();
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::stage_remove_child
//       Access: Private
//  Description: The private implementation of remove_child(), for a
//               particular pipeline stage.
////////////////////////////////////////////////////////////////////
bool PandaNode::
stage_remove_child(PandaNode *child_node, int pipeline_stage) {
  CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
      
  // First, look for the parent in the child's up list, to ensure the
  // child is known.
  CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
  int parent_index = child_node->do_find_parent(this, cdata_child);
  if (parent_index < 0) {
    // Nope, no relation.
    return false;
  }

  int child_index = do_find_child(child_node, cdata);
  if (child_index >= 0) {
    // The child exists; remove it.
    cdata->_down.erase(cdata->_down.begin() + child_index);
    int num_erased = cdata_child->_up.erase(UpConnection(this));
    nassertr(num_erased == 1, false);
    return true;
  }

  int stashed_index = do_find_stashed(child_node, cdata);
  if (stashed_index >= 0) {
    // The child has been stashed; remove it.
    cdata->_stashed.erase(cdata->_stashed.begin() + stashed_index);
    int num_erased = cdata_child->_up.erase(UpConnection(this));
    nassertr(num_erased == 1, false);
    return true;
  }

  // Never heard of this child.  This shouldn't be possible, because
  // the parent was in the child's up list, above.  Must be some
  // internal error.
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::stage_replace_child
//       Access: Private
//  Description: The private implementation of replace_child(), for a
//               particular pipeline stage.
////////////////////////////////////////////////////////////////////
bool PandaNode::
stage_replace_child(PandaNode *orig_child, PandaNode *new_child,
                    int pipeline_stage) {
  {
    CDLinksStageWriter cdata(_cycler_links, pipeline_stage);
    CDLinksStageWriter cdata_orig_child(orig_child->_cycler_links, pipeline_stage);
    CDLinksStageWriter cdata_new_child(new_child->_cycler_links, pipeline_stage);
    
    // First, look for the parent in the child's up list, to ensure the
    // child is known.
    int parent_index = orig_child->do_find_parent(this, cdata_orig_child);
    if (parent_index < 0) {
      // Nope, no relation.
      return false;
    }
    
    if (orig_child == new_child) {
      // Trivial no-op.
      return true;
    }
    
    // Don't let orig_child be destructed yet.
    PT(PandaNode) keep_orig_child = orig_child;
    
    int child_index = do_find_child(orig_child, cdata);
    if (child_index >= 0) {
      // The child exists; replace it.
      DownConnection &down = cdata->_down[child_index];
      nassertr(down.get_child() == orig_child, false);
      down.set_child(new_child);
      
    } else {
      int stashed_index = do_find_stashed(orig_child, cdata);
      if (stashed_index >= 0) {
	// The child has been stashed; remove it.
	DownConnection &down = cdata->_stashed[stashed_index];
	nassertr(down.get_child() == orig_child, false);
	down.set_child(new_child);
	
      } else {
	// Never heard of this child.  This shouldn't be possible, because
	// the parent was in the child's up list, above.  Must be some
	// internal error.
	nassertr(false, false);
	return false;
      }
    }
    
    // Now adjust the bookkeeping on both children.
    cdata_new_child->_up.insert(UpConnection(this));
    int num_erased = cdata_orig_child->_up.erase(UpConnection(this));
    nassertr(num_erased == 1, false);
  }

  sever_connection(this, orig_child, pipeline_stage);
  new_connection(this, new_child, pipeline_stage);

  force_bounds_stale(pipeline_stage);
  orig_child->parents_changed();
  new_child->parents_changed();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::attach
//       Access: Private, Static
//  Description: Creates a new parent-child relationship, and returns
//               the new NodePathComponent.  If the child was already
//               attached to the indicated parent, repositions it and
//               returns the original NodePathComponent.
//
//               This operation is automatically propagated back up to
//               pipeline 0, from the specified pipeline stage.
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
attach(NodePathComponent *parent, PandaNode *child_node, int sort,
       int pipeline_stage) {
  if (parent == (NodePathComponent *)NULL) {
    // Attaching to NULL means to create a new "instance" with no
    // attachments, and no questions asked.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, (NodePathComponent *)NULL,
			    pipeline_stage);
    MutexHolder holder(child_node->_paths_lock);
    child_node->_paths.insert(child);
    return child;
  }

  // See if the child was already attached to the parent.  If it was,
  // we'll use that same NodePathComponent.
  PT(NodePathComponent) child = get_component(parent, child_node, pipeline_stage);

  if (child == (NodePathComponent *)NULL) {
    // The child was not already attached to the parent, so get a new
    // component.
    child = get_top_component(child_node, true, pipeline_stage);
  }

  reparent(parent, child, sort, false, pipeline_stage);

  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::detach
//       Access: Private, Static
//  Description: Breaks a parent-child relationship.
//
//               This operation is automatically propagated back up to
//               pipeline 0, from the specified pipeline stage.
////////////////////////////////////////////////////////////////////
void PandaNode::
detach(NodePathComponent *child, int pipeline_stage) {
  nassertv(child != (NodePathComponent *)NULL);

  for (int pipeline_stage_i = pipeline_stage;
       pipeline_stage_i >= 0; 
       --pipeline_stage_i) {
    detach_one_stage(child, pipeline_stage_i);
  }

  child->get_node()->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::detach_one_stage
//       Access: Private, Static
//  Description: Breaks a parent-child relationship.
//
//               This operation is not automatically propagated
//               upstream.  It is applied to the indicated pipeline
//               stage only.
////////////////////////////////////////////////////////////////////
void PandaNode::
detach_one_stage(NodePathComponent *child, int pipeline_stage) {
  nassertv(child != (NodePathComponent *)NULL);
  if (child->is_top_node(pipeline_stage)) {
    return;
  }

  PT(PandaNode) child_node = child->get_node();
  PT(PandaNode) parent_node = child->get_next(pipeline_stage)->get_node();
    
  CDLinksStageWriter cdata_parent(parent_node->_cycler_links, pipeline_stage);
  CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);
  int parent_index = child_node->do_find_parent(parent_node, cdata_child);
  if (parent_index >= 0) {
    // Now look for the child and break the actual connection.
    
    // First, look for and remove the parent node from the child's up
    // list.
    int num_erased = cdata_child->_up.erase(UpConnection(parent_node));
    nassertv(num_erased == 1);
    
    // Now, look for and remove the child node from the parent's down
    // list.  We also check in the stashed list, in case the child node
    // has been stashed.
    Down::iterator di;
    bool found = false;
    for (di = cdata_parent->_down.begin(); 
	 di != cdata_parent->_down.end() && !found; 
	 ++di) {
      if ((*di).get_child() == child_node) {
	cdata_parent->_down.erase(di);
	found = true;
      }
    }
    for (di = cdata_parent->_stashed.begin(); 
	 di != cdata_parent->_stashed.end() && !found; 
	 ++di) {
      if ((*di).get_child() == child_node) {
	cdata_parent->_stashed.erase(di);
	found = true;
      }
    }
    nassertv(found);
  }
    
  // Finally, break the NodePathComponent connection.
  sever_connection(parent_node, child_node, pipeline_stage);

  parent_node->force_bounds_stale(pipeline_stage);
  parent_node->children_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reparent
//       Access: Private, Static
//  Description: Switches a node from one parent to another.  Returns
//               true if the new connection is allowed, or false if it
//               conflicts with another instance (that is, another
//               instance of the child is already attached to the
//               indicated parent).
//
//               This operation is automatically propagated back up to
//               pipeline 0, from the specified pipeline stage.
////////////////////////////////////////////////////////////////////
bool PandaNode::
reparent(NodePathComponent *new_parent, NodePathComponent *child, int sort,
         bool as_stashed, int pipeline_stage) {
  bool any_ok = false;

  for (int pipeline_stage_i = pipeline_stage;
       pipeline_stage_i >= 0; 
       --pipeline_stage_i) {
    if (reparent_one_stage(new_parent, child, sort, as_stashed, 
			   pipeline_stage_i)) {
      any_ok = true;
    }
  }

  if (new_parent != (NodePathComponent *)NULL) {
    new_parent->get_node()->children_changed();
  }
  child->get_node()->parents_changed();

  return any_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reparent_one_stage
//       Access: Private, Static
//  Description: Switches a node from one parent to another.  Returns
//               true if the new connection is allowed, or false if it
//               conflicts with another instance (that is, another
//               instance of the child is already attached to the
//               indicated parent).
//
//               This operation is not automatically propagated
//               upstream.  It is applied to the indicated pipeline
//               stage only.
////////////////////////////////////////////////////////////////////
bool PandaNode::
reparent_one_stage(NodePathComponent *new_parent, NodePathComponent *child, 
		   int sort, bool as_stashed, int pipeline_stage) {
  nassertr(child != (NodePathComponent *)NULL, false);

  // Keep a reference count to the new parent, since detaching the
  // child might lose the count.
  PT(NodePathComponent) keep_parent = new_parent;

  if (!child->is_top_node(pipeline_stage)) {
    detach(child, pipeline_stage);
  }

  if (new_parent != (NodePathComponent *)NULL) {
    PandaNode *child_node = child->get_node();
    PandaNode *parent_node = new_parent->get_node();

    {
      CDLinksStageReader cdata_child(child_node->_cycler_links, pipeline_stage);
      int parent_index = child_node->do_find_parent(parent_node, cdata_child);

      if (parent_index >= 0) {
	// Whoops, there's already another instance of the child there.
	return false;
      }
    }

    // Redirect the connection to the indicated new parent.
    child->set_next(new_parent, pipeline_stage);
    
    // Now reattach the child node at the indicated sort position.
    {
      CDLinksStageWriter cdata_parent(parent_node->_cycler_links, pipeline_stage);
      CDLinksStageWriter cdata_child(child_node->_cycler_links, pipeline_stage);

      if (as_stashed) {
        cdata_parent->_stashed.insert(DownConnection(child_node, sort));
      } else {
        cdata_parent->_down.insert(DownConnection(child_node, sort));
      }
      cdata_child->_up.insert(UpConnection(parent_node));

#ifndef NDEBUG
      // The NodePathComponent should already be in the set.
      {
	MutexHolder holder(child_node->_paths_lock);
	nassertr(child_node->_paths.find(child) != child_node->_paths.end(), false);
      }
#endif // NDEBUG
    }

    child_node->fix_path_lengths(pipeline_stage);
    parent_node->force_bounds_stale(pipeline_stage);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_component
//       Access: Private, Static
//  Description: Returns the NodePathComponent based on the indicated
//               child of the given parent, or NULL if there is no
//               such parent-child relationship.
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
get_component(NodePathComponent *parent, PandaNode *child_node,
	      int pipeline_stage) {
  nassertr(parent != (NodePathComponent *)NULL, (NodePathComponent *)NULL);
  PandaNode *parent_node = parent->get_node();

  MutexHolder holder(child_node->_paths_lock);

  // First, walk through the list of NodePathComponents we already
  // have on the child, looking for one that already exists,
  // referencing the indicated parent component.
  Paths::const_iterator pi;
  for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
    if ((*pi)->get_next(pipeline_stage) == parent) {
      // If we already have such a component, just return it.
      return (*pi);
    }
  }
    
  // We don't already have a NodePathComponent referring to this
  // parent-child relationship.  Are they actually related?
  CDLinksStageReader cdata_child(child_node->_cycler_links, pipeline_stage);
  int parent_index = child_node->do_find_parent(parent_node, cdata_child);
  
  if (parent_index >= 0) {
    // They are.  Create and return a new one.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, parent, pipeline_stage);
    child_node->_paths.insert(child);
    return child;
  } else {
    // They aren't related.  Return NULL.
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_top_component
//       Access: Private, Static
//  Description: Returns a NodePathComponent referencing the
//               indicated node as a singleton.  It is invalid to call
//               this for a node that has parents, unless you are
//               about to create a new instance (and immediately
//               reconnect the NodePathComponent elsewhere).
//
//               If force is true, this will always return something,
//               even if it needs to create a new top component;
//               otherwise, if force is false, it will return NULL if
//               there is not already a top component available.
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
get_top_component(PandaNode *child_node, bool force, int pipeline_stage) {
  MutexHolder holder(child_node->_paths_lock);

  // Walk through the list of NodePathComponents we already have on
  // the child, looking for one that already exists as a top node.
  Paths::const_iterator pi;
  for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
    if ((*pi)->is_top_node(pipeline_stage)) {
      // If we already have such a component, just return it.
      return (*pi);
    }
  }

  if (!force) {
    // If we don't care to force the point, return NULL to indicate
    // there's not already a top component.
    return NULL;
  }

  // We don't already have such a NodePathComponent; create and
  // return a new one.
  PT(NodePathComponent) child = 
    new NodePathComponent(child_node, (NodePathComponent *)NULL,
			  pipeline_stage);
  child_node->_paths.insert(child);

  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_generic_component
//       Access: Private
//  Description: Returns a NodePathComponent referencing this node as
//               a path from the root.  
//
//               Unless accept_ambiguity is true, it is only valid to
//               call this if there is an unambiguous path from the
//               root; otherwise, a warning will be issued and one
//               path will be chosen arbitrarily.
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
get_generic_component(bool accept_ambiguity, int pipeline_stage) {
  bool ambiguity_detected = false;
  PT(NodePathComponent) result = 
    r_get_generic_component(accept_ambiguity, ambiguity_detected,
			    pipeline_stage);

  if (!accept_ambiguity && ambiguity_detected) {
    pgraph_cat.warning()
      << "Chose: " << *result << "\n";
    nassertr(!unambiguous_graph, result);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_get_generic_component
//       Access: Private
//  Description: The recursive implementation of
//               get_generic_component, this simply sets the flag when
//               the ambiguity is detected (so we can report the
//               bottom node that started the ambiguous search).
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
r_get_generic_component(bool accept_ambiguity, bool &ambiguity_detected,
			int pipeline_stage) {
  PT(PandaNode) parent_node;

  {
    CDLinksStageReader cdata(_cycler_links, pipeline_stage);
    
    int num_parents = cdata->_up.size();
    if (num_parents == 0) {
      // No parents; no ambiguity.  This is the root.
      return get_top_component(this, true, pipeline_stage);
    } 

    PT(NodePathComponent) result;
    if (num_parents == 1) {
      // Only one parent; no ambiguity.
      PT(NodePathComponent) parent = 
	get_parent(0)->r_get_generic_component(accept_ambiguity, ambiguity_detected, pipeline_stage);
      return get_component(parent, this, pipeline_stage);
    }

    // Oops, multiple parents; the NodePath is ambiguous.
    if (!accept_ambiguity) {
      pgraph_cat.warning()
        << *this << " has " << num_parents
        << " parents; choosing arbitrary path to root.\n";
    }
    ambiguity_detected = true;
    parent_node = cdata->_up[0].get_parent();
  }

  // Now that the lock is released, it's safe to recurse.
  PT(NodePathComponent) parent = 
    parent_node->r_get_generic_component(accept_ambiguity, ambiguity_detected, pipeline_stage);
  return get_component(parent, this, pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::delete_component
//       Access: Private
//  Description: Removes a NodePathComponent from the set prior to
//               its deletion.  This should only be called by the
//               NodePathComponent destructor.
////////////////////////////////////////////////////////////////////
void PandaNode::
delete_component(NodePathComponent *component) {
  MutexHolder holder(_paths_lock);
  int num_erased = _paths.erase(component);
  nassertv(num_erased == 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::sever_connection
//       Access: Private, Static
//  Description: This is called internally when a parent-child
//               connection is broken to update the NodePathComponents
//               that reflected this connection.
//
//               It severs any NodePathComponents on the child node
//               that reference the indicated parent node.  These
//               components remain unattached; there may therefore be
//               multiple "instances" of a node that all have no
//               parent, even while there are other instances that do
//               have parents.
//
//               This operation is not automatically propagated
//               upstream.  It is applied to the indicated pipeline
//               stage only.
////////////////////////////////////////////////////////////////////
void PandaNode::
sever_connection(PandaNode *parent_node, PandaNode *child_node,
                 int pipeline_stage) {
  {
    MutexHolder holder(child_node->_paths_lock);
    Paths::iterator pi;
    for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
      if (!(*pi)->is_top_node(pipeline_stage) && 
	  (*pi)->get_next(pipeline_stage)->get_node() == parent_node) {
	// Sever the component here.
	(*pi)->set_top_node(pipeline_stage);
      }
    }
  }
  child_node->fix_path_lengths(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::new_connection
//       Access: Private, Static
//  Description: This is called internally when a parent-child
//               connection is established to update the
//               NodePathComponents that might be involved.
//
//               It adjusts any NodePathComponents the child has that
//               reference the child as a top node.  Any other
//               components we can leave alone, because we are making
//               a new instance of the child.
//
//               This operation is not automatically propagated
//               upstream.  It is applied to the indicated pipeline
//               stage only.
////////////////////////////////////////////////////////////////////
void PandaNode::
new_connection(PandaNode *parent_node, PandaNode *child_node,
               int pipeline_stage) {
  {
    MutexHolder holder(child_node->_paths_lock);
    Paths::iterator pi;
    for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
      if ((*pi)->is_top_node(pipeline_stage)) {
	(*pi)->set_next(parent_node->get_generic_component(false, pipeline_stage), pipeline_stage);
      }
    }
  }
  child_node->fix_path_lengths(pipeline_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fix_path_lengths
//       Access: Private
//  Description: Recursively fixes the _length member of each
//               NodePathComponent at this level and below, after an
//               add or delete child operation that might have messed
//               these up.
//
//               This operation is not automatically propagated
//               upstream.  It is applied to the indicated pipeline
//               stage only.
////////////////////////////////////////////////////////////////////
void PandaNode::
fix_path_lengths(int pipeline_stage) {
  MutexHolder holder(_paths_lock);

  bool any_wrong = false;

  Paths::const_iterator pi;
  for (pi = _paths.begin(); pi != _paths.end(); ++pi) {
    if ((*pi)->fix_length(pipeline_stage)) {
      any_wrong = true;
    }
  }
  
  // If any paths were updated, we have to recurse on all of our
  // children, since any one of those paths might be shared by any of
  // our child nodes.  Don't hold any locks while we recurse.
  if (any_wrong) {
    Children children = get_children();
    Stashed stashed = get_stashed();

    int num_children = children.get_num_children();
    int i;
    for (i = 0; i < num_children; ++i) {
      PandaNode *child_node = children.get_child(i);
      child_node->fix_path_lengths(pipeline_stage);
    }
    int num_stashed = stashed.get_num_stashed();
    for (i = 0; i < num_stashed; ++i) {
      PandaNode *child_node = stashed.get_stashed(i);
      child_node->fix_path_lengths(pipeline_stage);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_list_descendants
//       Access: Private
//  Description: The recursive implementation of ls().
////////////////////////////////////////////////////////////////////
void PandaNode::
r_list_descendants(ostream &out, int indent_level) const {
  write(out, indent_level);

  Children children = get_children();
  int num_children = children.get_num_children();

  for (int i = 0; i < num_children; ++i) {
    PandaNode *child = children.get_child(i);
    child->r_list_descendants(out, indent_level + 2);
  }

  // Also report the number of stashed nodes at this level.
  int num_stashed = get_num_stashed();
  if (num_stashed != 0) {
    indent(out, indent_level) << "(" << num_stashed << " stashed)\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::update_bounds
//       Access: Private
//  Description: Updates the cached values of the node that are
//               dependent on its children, such as the
//               external bounding volume, the _net_collide_mask, and
//               the _off_clip_planes.
//
//               The old value should be passed in; it will be
//               released.  The new value is returned.
////////////////////////////////////////////////////////////////////
PandaNode::CDBoundsStageWriter PandaNode::
update_bounds(int pipeline_stage, PandaNode::CDBoundsStageReader &cdata) {
  // We might need to try this a couple of times, in case someone else
  // steps on our result.
  do {
    // Grab the last_update counter, then release the lock.
    UpdateSeq last_update = cdata->_last_update;
    UpdateSeq next_update = cdata->_next_update;
    nassertr(last_update != next_update, CDBoundsStageWriter(_cycler_bounds, pipeline_stage, cdata));
    _cycler_bounds.release_read_stage(pipeline_stage, cdata.take_pointer());

    // Start with a clean slate, or at least with the contents of the
    // node itself.
    CollideMask net_collide_mask;
    {
      CDHeavyStageReader cdata(_cycler_heavy, pipeline_stage);
      net_collide_mask = cdata->_into_collide_mask;
    }
    CPT(RenderAttrib) off_clip_planes;
    {
      CDLightStageReader cdata(_cycler_light, pipeline_stage);
      off_clip_planes = cdata->_state->get_clip_plane();
    }
    if (off_clip_planes == (RenderAttrib *)NULL) {
      off_clip_planes = ClipPlaneAttrib::make();
    }

    // We need to keep references to the bounding volumes, since in a
    // threaded environment the pointers might go away while we're
    // working (since we're not holding a lock on our set of children
    // right now).  But we also need the regular pointers, to pass to
    // BoundingVolume::around().
    pvector<CPT(BoundingVolume) > child_volumes_ref;
    pvector<const BoundingVolume *> child_volumes;
  
    CPT(BoundingVolume) internal_bounds = get_internal_bounds(pipeline_stage);
    child_volumes_ref.push_back(internal_bounds);
    child_volumes.push_back(internal_bounds);

    // Now expand those contents to include all of our children.
    Children children = get_children();
    int num_children = children.get_num_children();

    for (int i = 0; i < num_children; ++i) {
      PandaNode *child = children.get_child(i);

      const ClipPlaneAttrib *orig_cp = DCAST(ClipPlaneAttrib, off_clip_planes);
      
      CDBoundsStageReader child_cdata(child->_cycler_bounds, pipeline_stage);
      if (child_cdata->_last_update != child_cdata->_next_update) {
	// Child needs update.
	CDBoundsStageWriter child_cdataw = child->update_bounds(pipeline_stage, child_cdata);
      
	net_collide_mask |= child_cdataw->_net_collide_mask;
	off_clip_planes = orig_cp->compose_off(child_cdataw->_off_clip_planes);
	child_volumes_ref.push_back(child_cdataw->_external_bounds);
	child_volumes.push_back(child_cdataw->_external_bounds);

      } else {
	// Child is good.
	net_collide_mask |= child_cdata->_net_collide_mask;
	off_clip_planes = orig_cp->compose_off(child_cdata->_off_clip_planes);
	child_volumes_ref.push_back(child_cdata->_external_bounds);
	child_volumes.push_back(child_cdata->_external_bounds);
      }
    }

    {
      // Now grab the write lock on this node.
      CDBoundsStageWriter cdataw(_cycler_bounds, pipeline_stage);
      if (last_update == cdataw->_last_update && 
	  next_update == cdataw->_next_update) {
	// Great, no one has monkeyed with these while we were computing
	// the cache.  Safe to store the computed values and return.
	cdataw->_net_collide_mask = net_collide_mask;
	cdataw->_off_clip_planes = off_clip_planes;

	// Compute the bounding sphere around all of our child
	// volumes.
	PT(GeometricBoundingVolume) gbv = new BoundingSphere;
	const BoundingVolume **child_begin = &child_volumes[0];
	const BoundingVolume **child_end = child_begin + child_volumes.size();
	((BoundingVolume *)gbv)->around(child_begin, child_end);
  
	// If we have a transform, apply it to the bounding volume we
	// just computed.
	CPT(TransformState) transform = get_transform();
	if (!transform->is_identity()) {
	  gbv->xform(transform->get_mat());
	}

	cdataw->_external_bounds = gbv;
	cdataw->_last_update = next_update;
	return cdataw;
      }
      
      if (cdataw->_last_update == cdataw->_next_update) {
	// Someone else has computed the cache for us.  OK.
	return cdataw;
      }
    }

    // We need to go around again.  Release the write lock, and grab
    // the read lock back.
    cdata = CDBoundsStageReader(_cycler_bounds, pipeline_stage);

    if (cdata->_last_update == cdata->_next_update) {
      // Someone else has computed the cache for us while we were
      // diddling with the locks.  OK.
      return CDBoundsStageWriter(_cycler_bounds, pipeline_stage, cdata);
    }

  } while (true);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  dg.add_string(get_name());

  manager->write_cdata(dg, _cycler_light);
  manager->write_cdata(dg, _cycler_heavy);
  manager->write_cdata(dg, _cycler_links);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::write_recorder
//       Access: Public
//  Description: This method is provided for the benefit of classes
//               (like MouseRecorder) that inherit from PandaMode and
//               also RecorderBase.  It's not virtual at this level
//               since it doesn't need to be (it's called up from the
//               derived class).
//
//               This method acts very like write_datagram, but it
//               writes the node as appropriate for writing a
//               RecorderBase object as described in the beginning of
//               a session file, meaning it doesn't need to write
//               things such as children.  It balances with
//               fillin_recorder().
////////////////////////////////////////////////////////////////////
void PandaNode::
write_recorder(BamWriter *, Datagram &dg) {
  dg.add_string(get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type PandaNode is encountered
//               in the Bam file.  It should create the PandaNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PandaNode::
make_from_bam(const FactoryParams &params) {
  PandaNode *node = new PandaNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  string name = scan.get_string();
  set_name(name);

  manager->read_cdata(scan, _cycler_light);
  manager->read_cdata(scan, _cycler_heavy);
  manager->read_cdata(scan, _cycler_links);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fillin_recorder
//       Access: Protected
//  Description: This internal function is called by make_recorder (in
//               classes derived from RecorderBase, such as
//               MouseRecorder) to read in all of the relevant data
//               from the session file.  It balances with
//               write_recorder().
////////////////////////////////////////////////////////////////////
void PandaNode::
fillin_recorder(DatagramIterator &scan, BamReader *) {
  string name = scan.get_string();
  set_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataLight::
CDataLight(const PandaNode::CDataLight &copy) :
  _state(copy._state),
  _transform(copy._transform),
  _prev_transform(copy._prev_transform)
{
  // Note that this copy constructor is not used by the PandaNode copy
  // constructor!  Any elements that must be copied between nodes
  // should also be explicitly copied there.
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataLight::
~CDataLight() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CDataLight::
make_copy() const {
  return new CDataLight(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLight::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _state);
  manager->write_pointer(dg, _transform);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int PandaNode::CDataLight::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the state and transform pointers.
  _state = DCAST(RenderState, p_list[pi++]);
  _transform = DCAST(TransformState, p_list[pi++]);
  _prev_transform = _transform;

  // Finalize these pointers now to decrement their artificially-held
  // reference counts.  We do this now, rather than later, in case
  // some other object reassigns them a little later on during
  // initialization, before they can finalize themselves normally (for
  // instance, the character may change the node's transform).  If
  // that happens, the pointer may discover that no one else holds its
  // reference count when it finalizes, which will constitute a memory
  // leak (see the comments in TransformState::finalize(), etc.).
  manager->finalize_now((RenderState *)_state.p());
  manager->finalize_now((TransformState *)_transform.p());

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLight::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  // Read the state and transform pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataHeavy::
CDataHeavy(const PandaNode::CDataHeavy &copy) :
  _effects(copy._effects),
  _tag_data(copy._tag_data),
  _draw_mask(copy._draw_mask),
  _into_collide_mask(copy._into_collide_mask),
  _user_bounds(copy._user_bounds),
  _internal_bounds(copy._internal_bounds),
  _internal_bounds_stale(copy._internal_bounds_stale),
  _final_bounds(copy._final_bounds)
{
  // Note that this copy constructor is not used by the PandaNode copy
  // constructor!  Any elements that must be copied between nodes
  // should also be explicitly copied there.

#ifdef HAVE_PYTHON
  // Copy and increment all of the Python objects held by the other
  // node.
  _python_tag_data = _python_tag_data;
  inc_py_refs();
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataHeavy::
~CDataHeavy() {
#ifdef HAVE_PYTHON
  // Free all of the Python objects held by this node.
  dec_py_refs();
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CDataHeavy::
make_copy() const {
  return new CDataHeavy(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataHeavy::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _effects);

  dg.add_uint32(_draw_mask.get_word());
  dg.add_uint32(_into_collide_mask.get_word());

  dg.add_uint32(_tag_data.size());
  TagData::const_iterator ti;
  for (ti = _tag_data.begin(); ti != _tag_data.end(); ++ti) {
    dg.add_string((*ti).first);
    dg.add_string((*ti).second);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int PandaNode::CDataHeavy::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the effects pointer.
  _effects = DCAST(RenderEffects, p_list[pi++]);

  // Finalize these pointers now to decrement their artificially-held
  // reference counts.  We do this now, rather than later, in case
  // some other object reassigns them a little later on during
  // initialization, before they can finalize themselves normally (for
  // instance, the character may change the node's transform).  If
  // that happens, the pointer may discover that no one else holds its
  // reference count when it finalizes, which will constitute a memory
  // leak (see the comments in TransformState::finalize(), etc.).
  manager->finalize_now((RenderEffects *)_effects.p());

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataHeavy::
fillin(DatagramIterator &scan, BamReader *manager) {
  // Read the effects pointer.
  manager->read_pointer(scan);

  _draw_mask.set_word(scan.get_uint32());
  _into_collide_mask.set_word(scan.get_uint32());

  // Read in the tag list.
  int num_tags = scan.get_uint32();
  for (int i = 0; i < num_tags; i++) {
    string key = scan.get_string();
    string value = scan.get_string();
    _tag_data[key] = value;
  }
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::inc_py_refs
//       Access: Public
//  Description: Increments the reference counts on all held Python
//               objects.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataHeavy::
inc_py_refs() {
  PythonTagData::const_iterator ti;
  for (ti = _python_tag_data.begin();
       ti != _python_tag_data.end();
       ++ti) {
    PyObject *value = (*ti).second;
    Py_XINCREF(value);
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataHeavy::dec_py_refs
//       Access: Public
//  Description: Decrements the reference counts on all held Python
//               objects.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataHeavy::
dec_py_refs() {
  PythonTagData::const_iterator ti;
  for (ti = _python_tag_data.begin();
       ti != _python_tag_data.end();
       ++ti) {
    PyObject *value = (*ti).second;
    Py_XDECREF(value);
  }
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataBounds::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataBounds::
CDataBounds(const PandaNode::CDataBounds &copy) :
  _net_collide_mask(copy._net_collide_mask),
  _off_clip_planes(copy._off_clip_planes),
  _external_bounds(copy._external_bounds),
  _last_update(copy._last_update),
  _next_update(copy._next_update)
{
  // Note that this copy constructor is not used by the PandaNode copy
  // constructor!  Any elements that must be copied between nodes
  // should also be explicitly copied there.
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataBounds::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataBounds::
~CDataBounds() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataBounds::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CDataBounds::
make_copy() const {
  return new CDataBounds(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataLinks::
CDataLinks(const PandaNode::CDataLinks &copy) :
  _down(copy._down),
  _stashed(copy._stashed),
  _up(copy._up)
{
  // Note that this copy constructor is not used by the PandaNode copy
  // constructor!  Any elements that must be copied between nodes
  // should also be explicitly copied there.
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CDataLinks::
~CDataLinks() {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CDataLinks::
make_copy() const {
  return new CDataLinks(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
write_datagram(BamWriter *manager, Datagram &dg) const {
  write_up_list(_up, manager, dg);
  write_down_list(_down, manager, dg);
  write_down_list(_stashed, manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int PandaNode::CDataLinks::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the parent and child pointers.
  pi += complete_up_list(_up, p_list + pi, manager);
  pi += complete_down_list(_down, p_list + pi, manager);
  pi += complete_down_list(_stashed, p_list + pi, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
fillin(DatagramIterator &scan, BamReader *manager) {
  fillin_up_list(_up, scan, manager);
  fillin_down_list(_down, scan, manager);
  fillin_down_list(_stashed, scan, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::write_up_list
//       Access: Public
//  Description: Writes the indicated list of parent node pointers to
//               the datagram.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
write_up_list(const PandaNode::Up &up_list,
              BamWriter *manager, Datagram &dg) const {
  // When we write a PandaNode, we write out its complete list of
  // child node pointers, but we only write out the parent node
  // pointers that have already been added to the bam file by a
  // previous write operation.  This is a bit of trickery that allows
  // us to write out just a subgraph (instead of the complete graph)
  // when we write out an arbitrary node in the graph, yet also allows
  // us to keep nodes completely in sync when we use the bam format
  // for streaming scene graph operations over the network.

  int num_parents = 0;
  Up::const_iterator ui;
  for (ui = up_list.begin(); ui != up_list.end(); ++ui) {
    PandaNode *parent_node = (*ui).get_parent();
    if (manager->has_object(parent_node)) {
      num_parents++;
    }
  }
  nassertv(num_parents == (int)(PN_uint16)num_parents);
  dg.add_uint16(num_parents);
  for (ui = up_list.begin(); ui != up_list.end(); ++ui) {
    PandaNode *parent_node = (*ui).get_parent();
    if (manager->has_object(parent_node)) {
      manager->write_pointer(dg, parent_node);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::write_down_list
//       Access: Public
//  Description: Writes the indicated list of child node pointers to
//               the datagram.
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
write_down_list(const PandaNode::Down &down_list,
                BamWriter *manager, Datagram &dg) const {
  int num_children = down_list.size();
  nassertv(num_children == (int)(PN_uint16)num_children);
  dg.add_uint16(num_children);

  // Should we smarten up the writing of the sort number?  Most of the
  // time these will all be zero.
  Down::const_iterator di;
  for (di = down_list.begin(); di != down_list.end(); ++di) {
    PandaNode *child_node = (*di).get_child();
    int sort = (*di).get_sort();
    manager->write_pointer(dg, child_node);
    dg.add_int32(sort);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::complete_up_list
//       Access: Public
//  Description: Calls complete_pointers() on the list of parent node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CDataLinks::
complete_up_list(PandaNode::Up &up_list,
                 TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  // Get the parent pointers.
  Up::iterator ui;
  for (ui = up_list.begin(); ui != up_list.end(); ++ui) {
    PandaNode *parent_node = DCAST(PandaNode, p_list[pi++]);

    // For some reason, VC++ won't accept UpConnection as an inline
    // temporary constructor here ("C2226: unexpected type
    // PandaNode::UpConnection"), so we must make this assignment
    // using an explicit temporary variable.
    UpConnection connection(parent_node);
    (*ui) = connection;
  }

  // Now we should sort the list, since the sorting is based on
  // pointer order, which might be different from one session to the
  // next.
  up_list.sort();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::complete_down_list
//       Access: Public
//  Description: Calls complete_pointers() on the list of child node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CDataLinks::
complete_down_list(PandaNode::Down &down_list,
                   TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  Down::iterator di;
  for (di = down_list.begin(); di != down_list.end(); ++di) {
    int sort = (*di).get_sort();
    PT(PandaNode) child_node = DCAST(PandaNode, p_list[pi++]);
    (*di) = DownConnection(child_node, sort);
  }

  // Unlike the up list, we should *not* sort the down list.  The down
  // list is stored in a specific order, not related to pointer order;
  // and this order should be preserved from one session to the next.

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::fillin_up_list
//       Access: Public
//  Description: Reads the indicated list parent node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
fillin_up_list(PandaNode::Up &up_list,
               DatagramIterator &scan, BamReader *manager) {
  int num_parents = scan.get_uint16();
  // Read the list of parent nodes.  Push back a NULL for each one.
  _up.reserve(num_parents);
  for (int i = 0; i < num_parents; i++) {
    manager->read_pointer(scan);
    _up.push_back(UpConnection(NULL));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CDataLinks::fillin_down_list
//       Access: Public
//  Description: Reads the indicated list child node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CDataLinks::
fillin_down_list(PandaNode::Down &down_list,
                 DatagramIterator &scan, BamReader *manager) {
  int num_children = scan.get_uint16();
  // Read the list of child nodes.  Push back a NULL for each one.
  down_list.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    manager->read_pointer(scan);
    int sort = scan.get_int32();
    down_list.push_back(DownConnection(NULL, sort));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ChildrenCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::ChildrenCopy::
ChildrenCopy(const PandaNode::CDLinksReader &cdata) {
  _list.reserve(cdata->_down.size());

  Down::const_iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    PandaNode *child = (*di).get_child();
    _list.push_back(child);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::StashedCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::StashedCopy::
StashedCopy(const PandaNode::CDLinksReader &cdata) {
  _list.reserve(cdata->_down.size());

  Down::const_iterator di;
  for (di = cdata->_stashed.begin(); di != cdata->_stashed.end(); ++di) {
    PandaNode *child = (*di).get_child();
    _list.push_back(child);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::ParentsCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::ParentsCopy::
ParentsCopy(const PandaNode::CDLinksReader &cdata) {
  _list.reserve(cdata->_up.size());

  Up::const_iterator ui;
  for (ui = cdata->_up.begin(); ui != cdata->_up.end(); ++ui) {
    PandaNode *parent = (*ui).get_parent();
    _list.push_back(parent);
  }
}
