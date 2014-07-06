// Filename: pandaNode.cxx
// Created by:  drose (20Feb02)
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
#include "boundingBox.h"
#include "pStatTimer.h"
#include "config_mathutil.h"
#include "lightReMutexHolder.h"
#include "graphicsStateGuardianBase.h"
#include "py_panda.h"

// This category is just temporary for debugging convenience.
NotifyCategoryDecl(drawmask, EXPCL_PANDA_PGRAPH, EXPTP_PANDA_PGRAPH);
NotifyCategoryDef(drawmask, "");

TypeHandle PandaNode::BamReaderAuxDataDown::_type_handle;

PandaNode::SceneRootFunc *PandaNode::_scene_root_func;

PandaNodeChain PandaNode::_dirty_prev_transforms("_dirty_prev_transforms");
DrawMask PandaNode::_overall_bit = DrawMask::bit(31);

PStatCollector PandaNode::_reset_prev_pcollector("App:Collisions:Reset");
PStatCollector PandaNode::_update_bounds_pcollector("*:Bounds");

TypeHandle PandaNode::_type_handle;
TypeHandle PandaNode::CData::_type_handle;
TypeHandle PandaNodePipelineReader::_type_handle;

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
  Namable(name),
  _paths_lock("PandaNode::_paths_lock"),
  _dirty_prev_transform(false)
{
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Constructing " << (void *)this << ", " << get_name() << "\n";
  }
#ifndef NDEBUG
  _unexpected_change_flags = 0;
#endif // !NDEBUG

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

  if (_dirty_prev_transform) {
    // Need to have this held before we grab any other locks.
    LightMutexHolder holder(_dirty_prev_transforms._lock);
    do_clear_dirty_prev_transform();
  }

  // We shouldn't have any parents left by the time we destruct, or
  // there's a refcount fault somewhere.

  // Actually, that's not necessarily true anymore, since we might be
  // updating a node dynamically via the bam reader, which doesn't
  // necessarily keep related pairs of nodes in sync with each other.
  /*
#ifndef NDEBUG
  {
    CDReader cdata(_cycler);
    nassertv(cdata->get_up()->empty());
  }
#endif  // NDEBUG
  */

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
  Namable(copy),
  _paths_lock("PandaNode::_paths_lock"),
  _dirty_prev_transform(false)
{
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Copying " << (void *)this << ", " << get_name() << "\n";
  }
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  // Copying a node does not copy its children.
#ifndef NDEBUG
  _unexpected_change_flags = 0;
#endif // !NDEBUG

  // Need to have this held before we grab any other locks.
  LightMutexHolder holder(_dirty_prev_transforms._lock);

  // Copy the other node's state.
  {
    CDReader copy_cdata(copy._cycler);
    CDWriter cdata(_cycler, true);
    cdata->_state = copy_cdata->_state;
    cdata->_transform = copy_cdata->_transform;
    cdata->_prev_transform = copy_cdata->_prev_transform;
    if (cdata->_transform != cdata->_prev_transform) {
      do_set_dirty_prev_transform();
    }

    cdata->_effects = copy_cdata->_effects;
    cdata->_tag_data = copy_cdata->_tag_data;
    cdata->_draw_control_mask = copy_cdata->_draw_control_mask;
    cdata->_draw_show_mask = copy_cdata->_draw_show_mask;
    cdata->_into_collide_mask = copy_cdata->_into_collide_mask;
    cdata->_bounds_type = copy_cdata->_bounds_type;
    cdata->_user_bounds = copy_cdata->_user_bounds;
    cdata->_internal_bounds = NULL;
    cdata->_internal_bounds_computed = UpdateSeq::initial();
    cdata->_internal_bounds_mark = UpdateSeq::initial();
    ++cdata->_internal_bounds_mark;
    cdata->_final_bounds = copy_cdata->_final_bounds;
    cdata->_fancy_bits = copy_cdata->_fancy_bits;
    
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
//     Function: PandaNode::as_reference_count
//       Access: Public, Virtual
//  Description: Returns the pointer cast to a ReferenceCount pointer,
//               if it is in fact of that type.
////////////////////////////////////////////////////////////////////
ReferenceCount *PandaNode::
as_reference_count() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::dupe_for_flatten
//       Access: Public, Virtual
//  Description: This is similar to make_copy(), but it makes a copy
//               for the specific purpose of flatten.  Typically, this
//               will be a new PandaNode with a new pointer, but all
//               of the internal data will always be shared with the
//               original; whereas the new node returned by
//               make_copy() might not share the internal data.
////////////////////////////////////////////////////////////////////
PandaNode *PandaNode::
dupe_for_flatten() const {
  return make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of PandaNode by duplicating
//               instances (by calling dupe_for_flatten()), false
//               otherwise (for instance, a Camera cannot be safely
//               flattened, because the Camera pointer itself is
//               meaningful).
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
//               xform() method, false otherwise.
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
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_combine() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::safe_to_combine_children
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine the
//               children of this PandaNode with each other.  For
//               instance, an LODNode's children should not be
//               combined with each other, because the set of children
//               is meaningful.
////////////////////////////////////////////////////////////////////
bool PandaNode::
safe_to_combine_children() const {
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
    const LMatrix4 &mat = attribs._transform->get_mat();
    xform(mat);

    Thread *current_thread = Thread::get_current_thread();
    OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
      CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
      cdata->_effects = cdata->_effects->xform(mat);
      cdata->set_fancy_bit(FB_effects, !cdata->_effects->is_empty());
    }
    CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  }
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PandaNode::
xform(const LMatrix4 &) {
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
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform = transform->compose(get_transform());

  Children cr = get_children(current_thread);
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    cr.get_child(i)->calc_tight_bounds(min_point, max_point,
                                       found_any, next_transform,
                                       current_thread);
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
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
//     Function: PandaNode::is_renderable
//       Access: Public, Virtual
//  Description: Returns true if there is some value to visiting this
//               particular node during the cull traversal for any
//               camera, false otherwise.  This will be used to
//               optimize the result of get_net_draw_show_mask(), so
//               that any subtrees that contain only nodes for which
//               is_renderable() is false need not be visited.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_renderable() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::add_for_draw
//       Access: Public, Virtual
//  Description: Adds the node's contents to the CullResult we are
//               building up during the cull traversal, so that it
//               will be drawn at render time.  For most nodes other
//               than GeomNodes, this is a do-nothing operation.
////////////////////////////////////////////////////////////////////
void PandaNode::
add_for_draw(CullTraverser *, CullTraverserData &) {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::make_copy
//       Access: Published, Virtual
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
//     Function: PandaNode::copy_subgraph
//       Access: Published
//  Description: Allocates and returns a complete copy of this
//               PandaNode and the entire scene graph rooted at this
//               PandaNode.  Some data may still be shared from the
//               original (e.g. vertex index tables), but nothing that
//               will impede normal use of the PandaNode.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PandaNode::
copy_subgraph(Thread *current_thread) const {
  InstanceMap inst_map;
  return r_copy_subgraph(inst_map, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::count_num_descendants
//       Access: Published
//  Description: Returns the number of nodes at and below this level.
////////////////////////////////////////////////////////////////////
int PandaNode::
count_num_descendants() const {
  int count = 1;
  Children children = get_children();
  int num_children = children.get_num_children();

  for (int i = 0; i < num_children; ++i) {
    PandaNode *child = children.get_child(i);
    count += child->count_num_descendants();
  }

  return count;
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
add_child(PandaNode *child_node, int sort, Thread *current_thread) {
  nassertv(child_node != (PandaNode *)NULL);

  if (!verify_child_no_cycles(child_node)) {
    // Whoops, adding this child node would introduce a cycle in the
    // scene graph.
    return;
  }

  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);

  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);
    
    cdata->modify_down()->insert(DownConnection(child_node, sort));
    cdata_child->modify_up()->insert(UpConnection(this));
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler, current_thread) {
    new_connection(this, child_node, pipeline_stage, current_thread);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler);

  force_bounds_stale();

  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the nth child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_child(int child_index, Thread *current_thread) {
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);

  CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
  PT(Down) down = cdata->modify_down();
  nassertv(child_index >= 0 && child_index < (int)down->size());
  
  PT(PandaNode) child_node = (*down)[child_index].get_child();
  CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, 
                                 current_thread);
  PT(Up) up = cdata_child->modify_up();

  down->erase(down->begin() + child_index);
  int num_erased = up->erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node, pipeline_stage, current_thread);
  force_bounds_stale(pipeline_stage, current_thread);

  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
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
remove_child(PandaNode *child_node, Thread *current_thread) {
  nassertr(child_node != (PandaNode *)NULL, false);
  
  // Make sure the child node is not destructed during the execution
  // of this method.
  PT(PandaNode) keep_child = child_node;

  // We have to do this for each upstream pipeline stage.
  bool any_removed = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler, current_thread) {
    if (stage_remove_child(child_node, pipeline_stage, current_thread)) {
      any_removed = true;

      sever_connection(this, child_node, pipeline_stage, current_thread);
      force_bounds_stale(pipeline_stage, current_thread);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler);

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
//               false if the node is not a child or if there is some
//               other problem.
////////////////////////////////////////////////////////////////////
bool PandaNode::
replace_child(PandaNode *orig_child, PandaNode *new_child,
              Thread *current_thread) {
  nassertr(orig_child != (PandaNode *)NULL, false);
  nassertr(new_child != (PandaNode *)NULL, false);

  if (orig_child == new_child) {
    // Trivial no-op.
    return true;
  }

  if (!verify_child_no_cycles(new_child)) {
    // Whoops, adding this child node would introduce a cycle in the
    // scene graph.
    return false;
  }
  
  // Make sure the orig_child node is not destructed during the
  // execution of this method.
  PT(PandaNode) keep_orig_child = orig_child;

  // We have to do this for each upstream pipeline stage.
  bool any_replaced = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    if (stage_replace_child(orig_child, new_child, pipeline_stage, current_thread)) {
      any_replaced = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

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
stash_child(int child_index, Thread *current_thread) {
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);
  nassertv(child_index >= 0 && child_index < get_num_children());

  // Save a reference count for ourselves.
  PT(PandaNode) self = this;

  PT(PandaNode) child_node = get_child(child_index);
  int sort = get_child_sort(child_index);
  
  remove_child(child_index);

  {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);
    
    cdata->modify_stashed()->insert(DownConnection(child_node, sort));
    cdata_child->modify_up()->insert(UpConnection(this));
  }

  new_connection(this, child_node, pipeline_stage, current_thread);
  force_bounds_stale(pipeline_stage, current_thread);

  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
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
unstash_child(int stashed_index, Thread *current_thread) { 
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);
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
    CDWriter cdata(_cycler);
    CDWriter cdata_child(child_node->_cycler);

    cdata->modify_down()->insert(DownConnection(child_node, sort));
    cdata_child->modify_up()->insert(UpConnection(this));
  }

  new_connection(this, child_node, pipeline_stage, current_thread);

  force_bounds_stale();
  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
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
add_stashed(PandaNode *child_node, int sort, Thread *current_thread) {
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);

  if (!verify_child_no_cycles(child_node)) {
    // Whoops, adding this child node would introduce a cycle in the
    // scene graph.
    return;
  }

  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);

  {
    CDWriter cdata(_cycler);
    CDWriter cdata_child(child_node->_cycler);
    
    cdata->modify_stashed()->insert(DownConnection(child_node, sort));
    cdata_child->modify_up()->insert(UpConnection(this));
  }
    
  new_connection(this, child_node, pipeline_stage, current_thread);

  // Call callback hooks.
  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_stashed
//       Access: Published
//  Description: Removes the nth stashed child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_stashed(int child_index, Thread *current_thread) {
  int pipeline_stage = current_thread->get_pipeline_stage();
  nassertv(pipeline_stage == 0);

  CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
  Down &stashed = *cdata->modify_stashed();
  nassertv(child_index >= 0 && child_index < (int)stashed.size());
  
  PT(PandaNode) child_node = stashed[child_index].get_child();
  CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);

  stashed.erase(stashed.begin() + child_index);
  int num_erased = cdata_child->modify_up()->erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node, pipeline_stage, current_thread);
  force_bounds_stale(pipeline_stage, current_thread);

  children_changed();
  child_node->parents_changed();
  mark_bam_modified();
  child_node->mark_bam_modified();
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
remove_all_children(Thread *current_thread) {
  // We have to do this for each upstream pipeline stage.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    PT(Down) down = cdata->modify_down();
    Down::iterator di;
    for (di = down->begin(); di != down->end(); ++di) {
      PT(PandaNode) child_node = (*di).get_child();
      CDStageWriter cdata_child(child_node->_cycler, pipeline_stage,
                                     current_thread);
      cdata_child->modify_up()->erase(UpConnection(this));
      
      sever_connection(this, child_node, pipeline_stage, current_thread);
      child_node->parents_changed();
      child_node->mark_bam_modified();
    }
    down->clear();
    
    Down &stashed = *cdata->modify_stashed();
    for (di = stashed.begin(); di != stashed.end(); ++di) {
      PT(PandaNode) child_node = (*di).get_child();
      CDStageWriter cdata_child(child_node->_cycler, pipeline_stage,
                                     current_thread);
      cdata_child->modify_up()->erase(UpConnection(this));
      
      sever_connection(this, child_node, pipeline_stage, current_thread);
      child_node->parents_changed();
      child_node->mark_bam_modified();
    }
    stashed.clear();
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  force_bounds_stale();
  children_changed();
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::steal_children
//       Access: Published
//  Description: Moves all the children from the other node onto this
//               node.
//
//               Any NodePaths to child nodes of the other node are
//               truncated, rather than moved to the new parent.
////////////////////////////////////////////////////////////////////
void PandaNode::
steal_children(PandaNode *other, Thread *current_thread) {
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
    add_child(child_node, sort, current_thread);
  }
  int num_stashed = other->get_num_stashed();
  for (i = 0; i < num_stashed; i++) {
    PandaNode *child_node = other->get_stashed(i);
    int sort = other->get_stashed_sort(i);
    add_stashed(child_node, sort, current_thread);
  }

  other->remove_all_children(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::copy_children
//       Access: Published
//  Description: Makes another instance of all the children of the
//               other node, copying them to this node.
////////////////////////////////////////////////////////////////////
void PandaNode::
copy_children(PandaNode *other, Thread *current_thread) {
  if (other == this) {
    // Trivial.
    return;
  }
  Children children = other->get_children(current_thread);
  Stashed stashed = other->get_stashed(current_thread);
  int num_children = children.get_num_children();
  int i;
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = children.get_child(i);
    int sort = children.get_child_sort(i);
    add_child(child_node, sort, current_thread);
  }
  int num_stashed = stashed.get_num_stashed();
  for (i = 0; i < num_stashed; i++) {
    PandaNode *child_node = stashed.get_stashed(i);
    int sort = stashed.get_stashed_sort(i);
    add_stashed(child_node, sort, current_thread);
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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    
    CPT(RenderState) new_state = cdata->_state->set_attrib(attrib, override);
    if (cdata->_state != new_state) {
      cdata->_state = new_state;
      cdata->set_fancy_bit(FB_state, true);
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  // Maybe we changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale(current_thread);
    state_changed();
    mark_bam_modified();
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
clear_attrib(int slot) {
  bool any_changed = false;

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    
    CPT(RenderState) new_state = cdata->_state->remove_attrib(slot);
    if (cdata->_state != new_state) {
      cdata->_state = new_state;
      cdata->set_fancy_bit(FB_state, !new_state->is_empty());
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  // We mark the bounds stale when the state changes, in case
  // we have changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale(current_thread);
    state_changed();
    mark_bam_modified();
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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_effects = cdata->_effects->add_effect(effect);
    cdata->set_fancy_bit(FB_effects, true);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_effect
//       Access: Published
//  Description: Removes the render effect of the given type from
//               this node.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_effect(TypeHandle type) {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_effects = cdata->_effects->remove_effect(type);
    cdata->set_fancy_bit(FB_effects, !cdata->_effects->is_empty());
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
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
set_state(const RenderState *state, Thread *current_thread) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    if (cdata->_state != state) {
      cdata->_state = state;
      cdata->set_fancy_bit(FB_state, !state->is_empty());
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  // Maybe we have changed a ClipPlaneAttrib.
  if (any_changed) {
    mark_bounds_stale(current_thread);
    state_changed();
    mark_bam_modified();
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
set_effects(const RenderEffects *effects, Thread *current_thread) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_effects = effects;
    cdata->set_fancy_bit(FB_effects, !effects->is_empty());
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_transform
//       Access: Published
//  Description: Sets the transform that will be applied to this node
//               and below.  This defines a new coordinate space at
//               this point in the scene graph and below.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_transform(const TransformState *transform, Thread *current_thread) {
  // Need to have this held before we grab any other locks.
  LightMutexHolder holder(_dirty_prev_transforms._lock);

  // Apply this operation to the current stage as well as to all
  // upstream stages.
  bool any_changed = false;
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    if (cdata->_transform != transform) {
      cdata->_transform = transform;
      cdata->set_fancy_bit(FB_transform, !transform->is_identity());
      any_changed = true;

      if (pipeline_stage == 0) {
        if (cdata->_transform != cdata->_prev_transform) {
          do_set_dirty_prev_transform();
        }
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if (any_changed) {
    mark_bounds_stale(current_thread);
    transform_changed();
    mark_bam_modified();
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
set_prev_transform(const TransformState *transform, Thread *current_thread) {
  // Need to have this held before we grab any other locks.
  LightMutexHolder holder(_dirty_prev_transforms._lock);

  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_prev_transform = transform;
    if (pipeline_stage == 0) {
      if (cdata->_transform != cdata->_prev_transform) {
        do_set_dirty_prev_transform();
      } else {
        do_clear_dirty_prev_transform();
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reset_prev_transform
//       Access: Published
//  Description: Resets the transform that represents this node's
//               "previous" position to the same as the current
//               transform.  This is not the same thing as clearing it
//               to identity.
////////////////////////////////////////////////////////////////////
void PandaNode::
reset_prev_transform(Thread *current_thread) {
  // Need to have this held before we grab any other locks.
  LightMutexHolder holder(_dirty_prev_transforms._lock);
  do_clear_dirty_prev_transform();

  // Apply this operation to the current stage as well as to all
  // upstream stages.

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_prev_transform = cdata->_transform;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reset_all_prev_transform
//       Access: Published, Static
//  Description: Visits all nodes in the world with the
//               _dirty_prev_transform flag--that is, all nodes whose
//               _prev_transform is different from the _transform in
//               pipeline stage 0--and resets the _prev_transform to
//               be the same as _transform.
////////////////////////////////////////////////////////////////////
void PandaNode::
reset_all_prev_transform(Thread *current_thread) {
  nassertv(current_thread->get_pipeline_stage() == 0);

  PStatTimer timer(_reset_prev_pcollector, current_thread);
  LightMutexHolder holder(_dirty_prev_transforms._lock);

  LinkedListNode *list_node = _dirty_prev_transforms._next;
  while (list_node != &_dirty_prev_transforms) {
    PandaNode *panda_node = (PandaNode *)list_node;
    nassertv(panda_node->_dirty_prev_transform);
    panda_node->_dirty_prev_transform = false;
    
    CDStageWriter cdata(panda_node->_cycler, 0, current_thread);
    cdata->_prev_transform = cdata->_transform;

    list_node = panda_node->_next;
#ifndef NDEBUG
    panda_node->_prev = NULL;
    panda_node->_next = NULL;
#endif  // NDEBUG
    panda_node->mark_bam_modified();
  }

  _dirty_prev_transforms._prev = &_dirty_prev_transforms;
  _dirty_prev_transforms._next = &_dirty_prev_transforms;
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
set_tag(const string &key, const string &value, Thread *current_thread) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_tag_data[key] = value;
    cdata->set_fancy_bit(FB_tag, true);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_tag
//       Access: Published
//  Description: Removes the value defined for this key on this
//               particular node.  After a call to clear_tag(),
//               has_tag() will return false for the indicated key.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_tag(const string &key, Thread *current_thread) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_tag_data.erase(key);
    cdata->set_fancy_bit(FB_tag, !cdata->_tag_data.empty());
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdataw(_cycler, pipeline_stage, current_thread);
    CDStageReader cdatar(other->_cycler, pipeline_stage, current_thread);
      
    TagData::const_iterator ti;
    for (ti = cdatar->_tag_data.begin();
         ti != cdatar->_tag_data.end();
         ++ti) {
      cdataw->_tag_data[(*ti).first] = (*ti).second;
    }
    cdataw->set_fancy_bit(FB_tag, !cdataw->_tag_data.empty());
    
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
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
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
  CDReader cdata(_cycler);
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
//     Function: Filename::get_tag_keys
//       Access: Published
//  Description: Fills the given vector up with the
//               list of tags on this PandaNode.
//
//               It is the user's responsibility to ensure that the
//               keys vector is empty before making this call;
//               otherwise, the new keys will be appended to it.
////////////////////////////////////////////////////////////////////
void PandaNode::
get_tag_keys(vector_string &keys) const {
  CDReader cdata(_cycler);
  if (!cdata->_tag_data.empty()) {
    TagData::const_iterator ti = cdata->_tag_data.begin();
    while (ti != cdata->_tag_data.end()) {
      keys.push_back((*ti).first);
      ++ti;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::compare_tags
//       Access: Published
//  Description: Returns a number less than 0, 0, or greater than 0,
//               to indicate the similarity of tags between this node
//               and the other one.  If this returns 0, the tags are
//               identical.  If it returns other than 0, then the tags
//               are different; and the nodes may be sorted into a
//               consistent (but arbitrary) ordering based on this
//               number.
////////////////////////////////////////////////////////////////////
int PandaNode::
compare_tags(const PandaNode *other) const {
  CDReader cdata(_cycler);
  CDReader cdata_other(other->_cycler);

  TagData::const_iterator ati = cdata->_tag_data.begin();
  TagData::const_iterator bti = cdata_other->_tag_data.begin();
  while (ati != cdata->_tag_data.end() &&
         bti != cdata_other->_tag_data.end()) {
    int cmp = strcmp((*ati).first.c_str(), (*bti).first.c_str());
    if (cmp != 0) {
      return cmp;
    }

    cmp = strcmp((*ati).second.c_str(), (*bti).second.c_str());
    if (cmp != 0) {
      return cmp;
    }

    ++ati;
    ++bti;
  }
  if (ati != cdata->_tag_data.end()) {
    // list A is longer.
    return 1;
  }
  if (bti != cdata_other->_tag_data.end()) {
    // list B is longer.
    return -1;
  }

#ifdef HAVE_PYTHON
  PythonTagData::const_iterator api = cdata->_python_tag_data.begin();
  PythonTagData::const_iterator bpi = cdata_other->_python_tag_data.begin();
  while (api != cdata->_python_tag_data.end() &&
         bpi != cdata_other->_python_tag_data.end()) {
    int cmp = strcmp((*api).first.c_str(), (*bpi).first.c_str());
    if (cmp != 0) {
      return cmp;
    }

#if PY_MAJOR_VERSION >= 3
    if (PyObject_RichCompareBool((*api).second, (*bpi).second, Py_LT) == 1) {
      return -1;
    } else if (PyObject_RichCompareBool((*api).second, (*bpi).second, Py_GT) == 1) {
      return 1;
    } else if (PyObject_RichCompareBool((*api).second, (*bpi).second, Py_EQ) == 1) {
      cmp = 0;
    } else {
#else
    if (PyObject_Cmp((*api).second, (*bpi).second, &cmp) == -1) {
#endif
      // Unable to compare objects; just compare pointers.
      if ((*api).second != (*bpi).second) {
        cmp = (*api).second < (*bpi).second ? -1 : 1;
      } else {
        cmp = 0;
      }
    }
    if (cmp != 0) {
      return cmp;
    }

    ++api;
    ++bpi;
  }
  if (api != cdata->_python_tag_data.end()) {
    // list A is longer.
    return 1;
  }
  if (bpi != cdata_other->_python_tag_data.end()) {
    // list B is longer.
    return -1;
  }
#endif  // HAVE_PYTHON

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::copy_all_properties
//       Access: Published
//  Description: Copies the TransformState, RenderState,
//               RenderEffects, tags, Python tags, and the show/hide
//               state from the other node onto this one.  Typically
//               this is used to prepare a node to replace another
//               node in the scene graph (also see replace_node()).
////////////////////////////////////////////////////////////////////
void PandaNode::
copy_all_properties(PandaNode *other) {
  if (other == this) {
    // Trivial.
    return;
  }

  // Need to have this held before we grab any other locks.
  LightMutexHolder holder(_dirty_prev_transforms._lock);

  bool any_transform_changed = false;
  bool any_state_changed = false;
  bool any_draw_mask_changed = false;
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdataw(_cycler, pipeline_stage, current_thread);
    CDStageReader cdatar(other->_cycler, pipeline_stage, current_thread);

    if (cdataw->_transform != cdatar->_transform) {
      any_transform_changed = true;
    }
    if (cdataw->_state != cdatar->_state) {
      any_state_changed = true;
    }
    if (cdataw->_draw_control_mask != cdatar->_draw_control_mask ||
        cdataw->_draw_show_mask != cdatar->_draw_show_mask) {
      any_draw_mask_changed = true;
    }

    cdataw->_transform = cdatar->_transform;
    cdataw->_prev_transform = cdatar->_prev_transform;
    cdataw->_state = cdatar->_state;
    cdataw->_effects = cdatar->_effects;
    cdataw->_draw_control_mask = cdatar->_draw_control_mask;
    cdataw->_draw_show_mask = cdatar->_draw_show_mask;

    // The collide mask becomes the union of the two masks.  This is
    // important to preserve properties such as the default GeomNode
    // bitmask.
    cdataw->_into_collide_mask |= cdatar->_into_collide_mask;
      
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

    static const int change_bits = (FB_transform | FB_state | FB_effects |
                                    FB_tag | FB_draw_mask);
    cdataw->_fancy_bits =
      (cdataw->_fancy_bits & ~change_bits) |
      (cdatar->_fancy_bits & change_bits);

    if (pipeline_stage == 0) {
      if (cdataw->_transform != cdataw->_prev_transform) {
        do_set_dirty_prev_transform();
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if (any_transform_changed || any_state_changed || any_draw_mask_changed) {
    mark_bounds_stale(current_thread);

    if (any_transform_changed) {
      transform_changed();
    }
    if (any_state_changed) {
      state_changed();
    }
    if (any_draw_mask_changed) {
      draw_mask_changed();
    }
    mark_bam_modified();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::replace_node
//       Access: Published
//  Description: Inserts this node into the scene graph in place of
//               the other one, and removes the other node.  All scene
//               graph attributes (TransformState, RenderState, etc.)
//               are copied to this node.  
//
//               All children are moved to this node, and removed from
//               the old node.  The new node is left in the same place
//               in the old node's parent's list of children.
//
//               Even NodePaths that reference the old node are
//               updated in-place to reference the new node instead.
//
//               This method is intended to be used to replace a node
//               of a given type in the scene graph with a node of a
//               different type.
////////////////////////////////////////////////////////////////////
void PandaNode::
replace_node(PandaNode *other) {
  //  nassertv(Thread::get_current_pipeline_stage() == 0);

  if (other == this) {
    // Trivial.
    return;
  }

  // Make sure the other node is not destructed during the
  // execution of this method.
  PT(PandaNode) keep_other = other;

  // Get all the important scene graph properties.
  copy_all_properties(other);

  // Fix up the NodePaths.
  {
    LightReMutexHolder holder1(other->_paths_lock);
    LightReMutexHolder holder2(_paths_lock);
    Paths::iterator pi;
    for (pi = other->_paths.begin(); pi != other->_paths.end(); ++pi) {
      (*pi)->_node = this;
      _paths.insert(*pi);
    }
    other->_paths.clear();
  }

  // Get the children.
  steal_children(other);

  // Switch the parents.
  Thread *current_thread = Thread::get_current_thread();
  Parents other_parents = other->get_parents();
  for (int i = 0; i < other_parents.get_num_parents(); ++i) {
    PandaNode *parent = other_parents.get_parent(i);
    if (find_parent(parent) != -1) {
      // This node was already a child of this parent; don't change
      // it.
      parent->remove_child(other);
    } else {
      // This node was not yet a child of this parent; now it is.
      parent->replace_child(other, this, current_thread);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_unexpected_change
//       Access: Published
//  Description: Sets one or more of the PandaNode::UnexpectedChange
//               bits on, indicating that the corresponding property
//               should not change again on this node.  Once one of
//               these bits has been set, if the property changes, an
//               assertion failure will be raised, which is designed
//               to assist the developer in identifying the
//               troublesome code that modified the property
//               unexpectedly.
//
//               The input parameter is the union of bits that are to
//               be set.  To clear these bits later, use
//               clear_unexpected_change().
//
//               Since this is a developer debugging tool only, this
//               function does nothing in a production (NDEBUG) build.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_unexpected_change(unsigned int flags) {
#ifndef NDEBUG
  _unexpected_change_flags |= flags;
#endif // !NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_unexpected_change
//       Access: Published
//  Description: Returns nonzero if any of the bits in the input
//               parameter are set on this node, or zero if none of
//               them are set.  More specifically, this returns the
//               particular set of bits (masked by the input
//               parameter) that have been set on this node.  See
//               set_unexpected_change().
//
//               Since this is a developer debugging tool only, this
//               function always returns zero in a production (NDEBUG)
//               build.
////////////////////////////////////////////////////////////////////
unsigned int PandaNode::
get_unexpected_change(unsigned int flags) const {
#ifndef NDEBUG
  return _unexpected_change_flags & flags;
#else
  return 0;
#endif // !NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::clear_unexpected_change
//       Access: Published
//  Description: Sets one or more of the PandaNode::UnexpectedChange
//               bits off, indicating that the corresponding property
//               may once again change on this node.  See
//               set_unexpected_change().
//
//               The input parameter is the union of bits that are to
//               be cleared.
//
//               Since this is a developer debugging tool only, this
//               function does nothing in a production (NDEBUG) build.
////////////////////////////////////////////////////////////////////
void PandaNode::
clear_unexpected_change(unsigned int flags) {
#ifndef NDEBUG
  _unexpected_change_flags &= ~flags;
#endif // !NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::adjust_draw_mask
//       Access: Published
//  Description: Adjusts the hide/show bits of this particular node.
//
//               These three parameters can be used to adjust the
//               _draw_control_mask and _draw_show_mask independently,
//               which work together to provide per-camera visibility
//               for the node and its descendents.
//
//               _draw_control_mask indicates the bits in
//               _draw_show_mask that are significant.  Each different
//               bit corresponds to a different camera (and these bits
//               are assigned via Camera::set_camera_mask()).
//
//               Where _draw_control_mask has a 1 bit, a 1 bit in
//               _draw_show_mask indicates the node is visible to that
//               camera, and a 0 bit indicates the node is hidden to
//               that camera.  Where _draw_control_mask is 0, the node
//               is hidden only if a parent node is hidden.
//
//               The meaning of the three parameters is as follows:
//
//               * Wherever show_mask is 1, _draw_show_mask and
//               _draw_control_mask will be set 1.  Thus, show_mask
//               indicates the set of cameras to which the node should
//               be shown.
//
//               * Wherever hide_mask is 1, _draw_show_mask will be
//               set 0 and _draw_control_mask will be set 1.  Thus,
//               hide_mask indicates the set of cameras from which the
//               node should be hidden.
//
//               * Wherever clear_mask is 1, _draw_control_mask will
//               be set 0.  Thus, clear_mask indicates the set of
//               cameras from which the hidden state should be
//               inherited from a parent.
////////////////////////////////////////////////////////////////////
void PandaNode::
adjust_draw_mask(DrawMask show_mask, DrawMask hide_mask, DrawMask clear_mask) {
  bool any_changed = false;

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    
    DrawMask draw_control_mask = (cdata->_draw_control_mask | show_mask | hide_mask) & ~clear_mask;
    DrawMask draw_show_mask = (cdata->_draw_show_mask | show_mask) & ~hide_mask;
    // The uncontrolled bits are implicitly on.
    draw_show_mask |= ~draw_control_mask;

    if (cdata->_draw_control_mask != draw_control_mask ||
        cdata->_draw_show_mask != draw_show_mask) {
      cdata->_draw_control_mask = draw_control_mask;
      cdata->_draw_show_mask = draw_show_mask;
      any_changed = true;
    }
    cdata->set_fancy_bit(FB_draw_mask, !draw_control_mask.is_zero());
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if (any_changed) {
    mark_bounds_stale(current_thread);
    draw_mask_changed();
    mark_bam_modified();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_net_draw_control_mask
//       Access: Published
//  Description: Returns the set of bits in get_net_draw_show_mask()
//               that have been explicitly set via adjust_draw_mask(),
//               rather than implicitly inherited.
//
//               A 1 bit in any position of this mask indicates that
//               (a) this node has renderable children, and (b) some
//               child of this node has made an explicit hide() or
//               show_through() call for the corresponding bit.
////////////////////////////////////////////////////////////////////
DrawMask PandaNode::
get_net_draw_control_mask() const {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    PStatTimer timer(_update_bounds_pcollector);
    CDStageWriter cdataw = 
      ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    return cdataw->_net_draw_control_mask;
  }
  return cdata->_net_draw_control_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_net_draw_show_mask
//       Access: Published
//  Description: Returns the union of all draw_show_mask values--of
//               renderable nodes only--at this level and below.  If
//               any bit in this mask is 0, there is no reason to
//               traverse below this node for a camera with the
//               corresponding camera_mask.
//
//               The bits in this mask that do not correspond to a 1
//               bit in the net_draw_control_mask are meaningless (and
//               will be set to 1).  For bits that *do* correspond to
//               a 1 bit in the net_draw_control_mask, a 1 bit
//               indicates that at least one child should be visible,
//               while a 0 bit indicates that all children are hidden.
////////////////////////////////////////////////////////////////////
DrawMask PandaNode::
get_net_draw_show_mask() const {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    PStatTimer timer(_update_bounds_pcollector);
    CDStageWriter cdataw = 
      ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    return cdataw->_net_draw_show_mask;
  }
  return cdata->_net_draw_show_mask;
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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    if (cdata->_into_collide_mask != mask) {
      cdata->_into_collide_mask = mask;
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if (any_changed) {
    mark_bounds_stale(current_thread);
    mark_bam_modified();
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
get_net_collide_mask(Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    PStatTimer timer(_update_bounds_pcollector);
    CDStageWriter cdataw = 
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
get_off_clip_planes(Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    PStatTimer timer(_update_bounds_pcollector);
    CDStageWriter cdataw = 
      ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
    return cdataw->_off_clip_planes;
  }
  return cdata->_off_clip_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::prepare_scene
//       Access: Published
//  Description: Walks through the scene graph beginning at this node,
//               and does whatever initialization is required to
//               render the scene properly with the indicated GSG.  It
//               is not strictly necessary to call this, since the GSG
//               will initialize itself when the scene is rendered,
//               but this may take some of the overhead away from that
//               process.
//
//               In particular, this will ensure that textures and
//               vertex buffers within the scene are loaded into
//               graphics memory.
////////////////////////////////////////////////////////////////////
void PandaNode::
prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state) {
  GeomTransformer transformer;
  Thread *current_thread = Thread::get_current_thread();
  r_prepare_scene(gsg, node_state, transformer, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_scene_root
//       Access: Published
//  Description: Returns true if this particular node is known to be
//               the render root of some active DisplayRegion
//               associated with the global GraphicsEngine, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_scene_root() const {
  // This function pointer has to be filled in when the global
  // GraphicsEngine is created, because we can't link with the
  // GraphicsEngine functions directly.
  if (_scene_root_func != (SceneRootFunc *)NULL) {
    return (*_scene_root_func)(this);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_under_scene_root
//       Access: Published
//  Description: Returns true if this particular node is in a live
//               scene graph: that is, it is a child or descendent of
//               a node that is itself a scene root.  If this is true,
//               this node may potentially be traversed by the render
//               traverser.  Stashed nodes don't count for this
//               purpose, but hidden nodes do.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_under_scene_root() const {
  if (is_scene_root()) {
    return true;
  }

  Parents parents = get_parents();
  for (int i = 0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    if (parent->find_stashed((PandaNode *)this) == -1) {
      if (parent->is_under_scene_root()) {
        return true;
      }
    }
  }
  return false;
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
  DrawMask draw_control_mask = get_draw_control_mask();
  if (!draw_control_mask.is_zero()) {
    DrawMask draw_show_mask = get_draw_show_mask();
    if (!(draw_control_mask & _overall_bit).is_zero()) {
      if (!(draw_show_mask & _overall_bit).is_zero()) {
        out << " (show_through)";
      } else {
        out << " (hidden)";
      }
    }
    if (!(draw_control_mask & ~_overall_bit).is_zero()) {
      draw_control_mask &= ~_overall_bit;
      if (!(draw_show_mask & draw_control_mask).is_zero()) {
        out << " (per-camera show_through)";
      }
      if (!(~draw_show_mask & draw_control_mask).is_zero()) {
        out << " (per-camera hidden)";
      }
    }
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_bounds_type
//       Access: Published
//  Description: Specifies the desired type of bounding volume that
//               will be created for this node.  This is normally
//               BoundingVolume::BT_default, which means to set the
//               type according to the config variable "bounds-type".
//
//               If this is BT_sphere or BT_box, a BoundingSphere or
//               BoundingBox is explicitly created.  If it is BT_best,
//               the appropriate type to best enclose the node's
//               children is created.
//
//               This affects the bounding volume returned by
//               get_bounds(), which is not exactly the same bounding
//               volume modified by set_bounds(), because a new
//               bounding volume has to be created that includes this
//               node and all of its children.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_bounds_type(BoundingVolume::BoundsType bounds_type) {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->_bounds_type = bounds_type;
    mark_bounds_stale(pipeline_stage, current_thread);

    // GeomNodes, CollisionNodes, and PGItems all have an internal
    // bounds that may need to be updated when the bounds_type
    // changes.
    mark_internal_bounds_stale(pipeline_stage, current_thread);
    mark_bam_modified();
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bounds_type
//       Access: Published
//  Description: Returns the bounding volume type set with
//               set_bounds_type().
////////////////////////////////////////////////////////////////////
BoundingVolume::BoundsType PandaNode::
get_bounds_type() const {
  CDReader cdata(_cycler);
  return cdata->_bounds_type;
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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    if (volume == NULL) {
      cdata->_user_bounds = NULL;
    } else {
      cdata->_user_bounds = volume->make_copy();
    }
    mark_bounds_stale(pipeline_stage, current_thread);
    mark_bam_modified();
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
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
get_bounds(Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CPT(BoundingVolume) result;
    {
      PStatTimer timer(_update_bounds_pcollector);
      CDStageWriter cdataw = 
        ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
      result = cdataw->_external_bounds;
    }
    return result;
  }
  return cdata->_external_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_bounds
//       Access: Published
//  Description: This flavor of get_bounds() return the external
//               bounding volume, and also fills in seq with the
//               bounding volume's current sequence number.  When this
//               sequence number changes, it indicates that the
//               bounding volume might have changed, e.g. because some
//               nested child's bounding volume has changed.
//
//               Although this might occasionally increment without
//               changing the bounding volume, the bounding volume
//               will never change without incrementing this counter,
//               so as long as this counter remains unchanged you can
//               be confident the bounding volume is also unchanged.
////////////////////////////////////////////////////////////////////
CPT(BoundingVolume) PandaNode::
get_bounds(UpdateSeq &seq, Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    CPT(BoundingVolume) result;
    {
      PStatTimer timer(_update_bounds_pcollector);
      CDStageWriter cdataw = 
        ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
      result = cdataw->_external_bounds;
      seq = cdataw->_last_update;
    }
    return result;
  }
  seq = cdata->_last_update;
  return cdata->_external_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_nested_vertices
//       Access: Published
//  Description: Returns the total number of vertices that will be
//               rendered by this node and all of its descendents.
//
//               This is not necessarily an accurate count of vertices
//               that will actually be rendered, since this will
//               include all vertices of all LOD's, and it will also
//               include hidden nodes.  It may also omit or only
//               approximate certain kinds of dynamic geometry.
//               However, it will not include stashed nodes.
////////////////////////////////////////////////////////////////////
int PandaNode::
get_nested_vertices(Thread *current_thread) const {
  int pipeline_stage = current_thread->get_pipeline_stage();
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);
  if (cdata->_last_update != cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.
    int result;
    {
      PStatTimer timer(_update_bounds_pcollector);
      CDStageWriter cdataw = 
        ((PandaNode *)this)->update_bounds(pipeline_stage, cdata); 
      result = cdataw->_nested_vertices;
    }
    return result;
  }
  return cdata->_nested_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_bounds_stale
//       Access: Published
//  Description: Indicates that the bounding volume, or something that
//               influences the bounding volume (or any of the other
//               things stored in CData, like net_collide_mask),
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
mark_bounds_stale(Thread *current_thread) const {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler, current_thread) {
    mark_bounds_stale(pipeline_stage, current_thread);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::mark_internal_bounds_stale
//       Access: Published
//  Description: Should be called by a derived class to mark the
//               internal bounding volume stale, so that
//               compute_internal_bounds() will be called when the
//               bounding volume is next requested.
//
//               With no parameters, this means to iterate through all
//               stages including and upstream of the current pipeline
//               stage.
//
//               It is normally not necessary to call this method
//               directly; each node should be responsible for calling
//               it when its internals have changed.
////////////////////////////////////////////////////////////////////
void PandaNode::
mark_internal_bounds_stale(Thread *current_thread) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler, current_thread) {
    mark_internal_bounds_stale(pipeline_stage, current_thread);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler);
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
//     Function: PandaNode::is_collision_node
//       Access: Published, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from CollisionNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(CollisionNode::get_class_type()).
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_collision_node() const {
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
//     Function: PandaNode::is_ambient_light
//       Access: Published, Virtual
//  Description: Returns true if this is an AmbientLight, false if it
//               is not a light, or it is some other kind of light.
////////////////////////////////////////////////////////////////////
bool PandaNode::
is_ambient_light() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::decode_from_bam_stream
//       Access: Published, Static
//  Description: Reads the string created by a previous call to
//               encode_to_bam_stream(), and extracts and returns the
//               single object on that string.  Returns NULL on error.
//
//               This method is intended to replace
//               decode_raw_from_bam_stream() when you know the stream
//               in question returns an object of type PandaNode,
//               allowing for easier reference count management.  Note
//               that the caller is still responsible for maintaining
//               the reference count on the return value.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PandaNode::
decode_from_bam_stream(const string &data, BamReader *reader) {
  TypedWritable *object;
  ReferenceCount *ref_ptr;

  if (!TypedWritable::decode_raw_from_bam_stream(object, ref_ptr, data, reader)) {
    return NULL;
  }

  return DCAST(PandaNode, object);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_bounds
//       Access: Protected
//  Description: Returns the node's internal bounding volume.  This is
//               the bounding volume around the node alone, without
//               including children.
////////////////////////////////////////////////////////////////////
CPT(BoundingVolume) PandaNode::
get_internal_bounds(int pipeline_stage, Thread *current_thread) const {
  while (true) {
    UpdateSeq mark;
    {
      CDStageReader cdata(_cycler, pipeline_stage, current_thread);
      if (cdata->_user_bounds != (BoundingVolume *)NULL) {
        return cdata->_user_bounds;
      }
      
      if (cdata->_internal_bounds_mark == cdata->_internal_bounds_computed) {
        return cdata->_internal_bounds;
      }

      mark = cdata->_internal_bounds_mark;
    }

    // First, call compute_internal_bounds without acquiring the lock.
    // This avoids a deadlock condition.
    CPT(BoundingVolume) internal_bounds;
    int internal_vertices;
    compute_internal_bounds(internal_bounds, internal_vertices,
                            pipeline_stage, current_thread);
    nassertr(!internal_bounds.is_null(), NULL);
    
    // Now, acquire the lock, and apply the above-computed bounds.
    CDStageWriter cdataw(((PandaNode *)this)->_cycler, pipeline_stage);
    if (cdataw->_internal_bounds_mark == mark) {
      cdataw->_internal_bounds_computed = mark;
      cdataw->_internal_bounds = internal_bounds;
      cdataw->_internal_vertices = internal_vertices;
      ((PandaNode *)this)->mark_bam_modified();
      return cdataw->_internal_bounds;
    }

    // Dang, someone in another thread incremented
    // _internal_bounds_mark while we weren't holding the lock.  That
    // means we need to go back and do it again.
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_internal_vertices
//       Access: Protected
//  Description: Returns the total number of vertices that will be
//               rendered by this particular node alone, not
//               accounting for its children.
//
//               This may not include all vertices for certain dynamic
//               effects.
////////////////////////////////////////////////////////////////////
int PandaNode::
get_internal_vertices(int pipeline_stage, Thread *current_thread) const {
  while (true) {
    UpdateSeq mark;
    {
      CDStageReader cdata(_cycler, pipeline_stage, current_thread);
      if (cdata->_internal_bounds_mark == cdata->_internal_bounds_computed) {
        return cdata->_internal_vertices;
      }

      mark = cdata->_internal_bounds_mark;
    }

    // First, call compute_internal_bounds without acquiring the lock.
    // This avoids a deadlock condition.
    CPT(BoundingVolume) internal_bounds;
    int internal_vertices;
    compute_internal_bounds(internal_bounds, internal_vertices,
                            pipeline_stage, current_thread);
    nassertr(!internal_bounds.is_null(), 0);
    
    // Now, acquire the lock, and apply the above-computed bounds.
    CDStageWriter cdataw(((PandaNode *)this)->_cycler, pipeline_stage);
    if (cdataw->_internal_bounds_mark == mark) {
      cdataw->_internal_bounds_computed = mark;
      cdataw->_internal_bounds = internal_bounds;
      cdataw->_internal_vertices = internal_vertices;
      ((PandaNode *)this)->mark_bam_modified();
      return cdataw->_internal_vertices;
    }

    // Dang, someone in another thread incremented
    // _internal_bounds_mark while we weren't holding the lock.  That
    // means we need to go back and do it again.
  }
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
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdataw(_cycler, pipeline_stage, current_thread);
    cdataw->_internal_bounds = volume;
    cdataw->_internal_bounds_computed = cdataw->_internal_bounds_mark;
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bounds_stale(current_thread);
  mark_bam_modified();
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
force_bounds_stale(Thread *current_thread) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler, current_thread) {
    force_bounds_stale(pipeline_stage, current_thread);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM_NOLOCK(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::force_bounds_stale
//       Access: Protected
//  Description: Similar to mark_bounds_stale(), except that the
//               parents of this node marked stale even if this node
//               was already considered stale.
////////////////////////////////////////////////////////////////////
void PandaNode::
force_bounds_stale(int pipeline_stage, Thread *current_thread) {
  {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    ++cdata->_next_update;
    mark_bam_modified();

    // It is important that we allow this lock to be dropped before we
    // continue up the graph; otherwise, we risk deadlock from another
    // thread walking down the graph.
  }

  // It is similarly important that we use get_parents() here to copy
  // the parents list, instead of keeping the lock open while we walk
  // through the parents list directly on the node.
  Parents parents;
  {
    CDStageReader cdata(_cycler, pipeline_stage, current_thread);
    parents = Parents(cdata);
  }
  int num_parents = parents.get_num_parents();
  for (int i = 0; i < num_parents; ++i) {
    PandaNode *parent = parents.get_parent(i);
    parent->mark_bounds_stale(pipeline_stage, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_mark_geom_bounds_stale
//       Access: Protected, Virtual
//  Description: Recursively calls Geom::mark_bounds_stale() on every
//               Geom at this node and below.
////////////////////////////////////////////////////////////////////
void PandaNode::
r_mark_geom_bounds_stale(Thread *current_thread) {
  Children children = get_children(current_thread);

  int i;
  for (i = 0; i < children.get_num_children(); i++) {
    PandaNode *child = children.get_child(i);
    child->r_mark_geom_bounds_stale(current_thread);
  }

  Stashed stashed = get_stashed(current_thread);
  for (i = 0; i < stashed.get_num_stashed(); i++) {
    PandaNode *child = stashed.get_stashed(i);
    child->r_mark_geom_bounds_stale(current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated BoundingVolume that
//               represents the internal contents of the node.  Should
//               be overridden by PandaNode classes that contain
//               something internally.
////////////////////////////////////////////////////////////////////
void PandaNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  internal_bounds = new BoundingSphere;
  internal_vertices = 0;
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
  nassertv((_unexpected_change_flags & UC_parents) == 0);
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
  nassertv((_unexpected_change_flags & UC_children) == 0);
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
  nassertv((_unexpected_change_flags & UC_transform) == 0);
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
  nassertv((_unexpected_change_flags & UC_state) == 0);
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
  nassertv((_unexpected_change_flags & UC_draw_mask) == 0);
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
r_copy_subgraph(PandaNode::InstanceMap &inst_map, Thread *current_thread) const {
  PT(PandaNode) copy = make_copy();
  nassertr(copy != (PandaNode *)NULL, NULL);
  if (copy->get_type() != get_type()) {
    pgraph_cat.warning()
      << "Don't know how to copy nodes of type " << get_type() << "\n";

    if (no_unsupported_copy) {
      nassertr(false, NULL);
    }
  }

  copy->r_copy_children(this, inst_map, current_thread);
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
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map,
                Thread *current_thread) {
  CDReader from_cdata(from->_cycler, current_thread);
  CPT(Down) from_down = from_cdata->get_down();
  Down::const_iterator di;
  for (di = from_down->begin(); di != from_down->end(); ++di) {
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
      dest_child = source_child->r_copy_subgraph(inst_map, current_thread);
      inst_map[source_child] = dest_child;
    }

    quick_add_new_child(dest_child, sort, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::r_prepare_scene
//       Access: Public, Virtual
//  Description: The recursive implementation of prepare_scene().
//               Don't call this directly; call
//               PandaNode::prepare_scene() or
//               NodePath::prepare_scene() instead.
////////////////////////////////////////////////////////////////////
void PandaNode::
r_prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state,
                GeomTransformer &transformer, Thread *current_thread) {
  Children children = get_children(current_thread);
  // We must call get_num_children() each time through the loop, in
  // case we're running SIMPLE_THREADS and we get interrupted.
  int i;
  for (i = 0; i < children.get_num_children(); i++) {
    PandaNode *child = children.get_child(i);
    CPT(RenderState) child_state = node_state->compose(child->get_state());
    child->r_prepare_scene(gsg, child_state, transformer, current_thread);
  }

  Stashed stashed = get_stashed(current_thread);
  for (i = 0; i < stashed.get_num_stashed(); i++) {
    PandaNode *child = stashed.get_stashed(i);
    CPT(RenderState) child_state = node_state->compose(child->get_state());
    child->r_prepare_scene(gsg, child_state, transformer, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_cull_callback
//       Access: Protected
//  Description: Intended to be called in the constructor by any
//               subclass that defines cull_callback(), this sets up
//               the flags to indicate that the cullback needs to be
//               called.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_cull_callback() {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->set_fancy_bit(FB_cull_callback, true);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::disable_cull_callback
//       Access: Protected
//  Description: disables the call back
////////////////////////////////////////////////////////////////////
void PandaNode::
disable_cull_callback() {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    cdata->set_fancy_bit(FB_cull_callback, false);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_bam_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::stage_remove_child
//       Access: Private
//  Description: The private implementation of remove_child(), for a
//               particular pipeline stage.
////////////////////////////////////////////////////////////////////
bool PandaNode::
stage_remove_child(PandaNode *child_node, int pipeline_stage,
                   Thread *current_thread) {
  CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
      
  // First, look for the parent in the child's up list, to ensure the
  // child is known.
  CDStageWriter cdata_child(child_node->_cycler, pipeline_stage,
                            current_thread);
  int parent_index = child_node->do_find_parent(this, cdata_child);
  if (parent_index < 0) {
    // Nope, no relation.
    return false;
  }

  PT(Down) down = cdata->modify_down();
  int child_index = do_find_child(child_node, down);
  if (child_index >= 0) {
    // The child exists; remove it.
    down->erase(down->begin() + child_index);
    int num_erased = cdata_child->modify_up()->erase(UpConnection(this));
    nassertr(num_erased == 1, false);
    return true;
  }

  PT(Down) stashed = cdata->modify_stashed();
  int stashed_index = do_find_child(child_node, stashed);
  if (stashed_index >= 0) {
    // The child has been stashed; remove it.
    stashed->erase(stashed->begin() + stashed_index);
    int num_erased = cdata_child->modify_up()->erase(UpConnection(this));
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
                    int pipeline_stage, Thread *current_thread) {
  {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    CDStageWriter cdata_orig_child(orig_child->_cycler, pipeline_stage, current_thread);
    CDStageWriter cdata_new_child(new_child->_cycler, pipeline_stage, current_thread);
    
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

    // If we already have new_child as a child, remove it first.
    if (stage_remove_child(new_child, pipeline_stage, current_thread)) {
      sever_connection(this, new_child, pipeline_stage, current_thread);
    }
    
    PT(Down) down = cdata->modify_down();
    int child_index = do_find_child(orig_child, down);
    if (child_index >= 0) {
      // The child exists; replace it.
      DownConnection &dc = (*down)[child_index];
      nassertr(dc.get_child() == orig_child, false);
      dc.set_child(new_child);
      
    } else {
      PT(Down) stashed = cdata->modify_stashed();
      int stashed_index = do_find_child(orig_child, stashed);
      if (stashed_index >= 0) {
        // The child has been stashed; remove it.
        DownConnection &dc = (*stashed)[stashed_index];
        nassertr(dc.get_child() == orig_child, false);
        dc.set_child(new_child);
        
      } else {
        // Never heard of this child.  This shouldn't be possible, because
        // the parent was in the child's up list, above.  Must be some
        // internal error.
        nassertr(false, false);
        return false;
      }
    }
    
    // Now adjust the bookkeeping on both children.
    cdata_new_child->modify_up()->insert(UpConnection(this));
    int num_erased = cdata_orig_child->modify_up()->erase(UpConnection(this));
    nassertr(num_erased == 1, false);
  }

  sever_connection(this, orig_child, pipeline_stage, current_thread);
  new_connection(this, new_child, pipeline_stage, current_thread);

  force_bounds_stale(pipeline_stage, current_thread);
  orig_child->parents_changed();
  new_child->parents_changed();
  mark_bam_modified();
  orig_child->mark_bam_modified();
  new_child->mark_bam_modified();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::quick_add_new_child
//       Access: Private
//  Description: Similar to add_child(), but performs fewer checks.
//               The purpose of this method is to add a child node
//               that was newly constructed, to a parent node that was
//               newly constructed, so we know we have to make fewer
//               sanity checks.  This is a private method; do not call
//               it directly.
////////////////////////////////////////////////////////////////////
void PandaNode::
quick_add_new_child(PandaNode *child_node, int sort, Thread *current_thread) {
  // Apply this operation to the current stage as well as to all
  // upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);
    
    cdata->modify_down()->insert(DownConnection(child_node, sort));
    cdata_child->modify_up()->insert(UpConnection(this));
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::report_cycle
//       Access: Private
//  Description: Raises an assertion when a graph cycle attempt is
//               detected (and aborted).
////////////////////////////////////////////////////////////////////
void PandaNode::
report_cycle(PandaNode *child_node) {
  ostringstream strm;
  strm << "Detected attempt to create a cycle in the scene graph: " 
       << NodePath::any_path(this) << " : " << *child_node;
  nassert_raise(strm.str());
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_node_above
//       Access: Private
//  Description: Returns true if the indicated node is this node, or
//               any ancestor of this node; or false if it is not in
//               this node's ancestry.
////////////////////////////////////////////////////////////////////
bool PandaNode::
find_node_above(PandaNode *node) {
  if (node == this) {
    return true;
  }

  Parents parents = get_parents();
  for (int i = 0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    if (parent->find_node_above(node)) {
      return true;
    }
  }

  return false;
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
       int pipeline_stage, Thread *current_thread) {
  if (parent == (NodePathComponent *)NULL) {
    // Attaching to NULL means to create a new "instance" with no
    // attachments, and no questions asked.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, (NodePathComponent *)NULL,
                            pipeline_stage, current_thread);
    LightReMutexHolder holder(child_node->_paths_lock);
    child_node->_paths.insert(child);
    return child;
  }

  // See if the child was already attached to the parent.  If it was,
  // we'll use that same NodePathComponent.
  PT(NodePathComponent) child = get_component(parent, child_node, pipeline_stage, current_thread);

  if (child == (NodePathComponent *)NULL) {
    // The child was not already attached to the parent, so get a new
    // component.
    child = get_top_component(child_node, true, pipeline_stage, current_thread);
  }

  reparent(parent, child, sort, false, pipeline_stage, current_thread);

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
detach(NodePathComponent *child, int pipeline_stage, Thread *current_thread) {
  nassertv(child != (NodePathComponent *)NULL);

  for (int pipeline_stage_i = pipeline_stage;
       pipeline_stage_i >= 0; 
       --pipeline_stage_i) {
    detach_one_stage(child, pipeline_stage_i, current_thread);
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
detach_one_stage(NodePathComponent *child, int pipeline_stage, 
                 Thread *current_thread) {
  nassertv(child != (NodePathComponent *)NULL);
  if (child->is_top_node(pipeline_stage, current_thread)) {
    return;
  }

  PT(PandaNode) child_node = child->get_node();
  PT(PandaNode) parent_node = child->get_next(pipeline_stage, current_thread)->get_node();
    
  CDStageWriter cdata_parent(parent_node->_cycler, pipeline_stage, current_thread);
  CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);
  int parent_index = child_node->do_find_parent(parent_node, cdata_child);
  if (parent_index >= 0) {
    // Now look for the child and break the actual connection.
    
    // First, look for and remove the parent node from the child's up
    // list.
    int num_erased = cdata_child->modify_up()->erase(UpConnection(parent_node));
    nassertv(num_erased == 1);
    
    // Now, look for and remove the child node from the parent's down
    // list.  We also check in the stashed list, in case the child node
    // has been stashed.
    Down::iterator di;
    bool found = false;
    PT(Down) down = cdata_parent->modify_down();
    for (di = down->begin(); di != down->end(); ++di) {
      if ((*di).get_child() == child_node) {
        down->erase(di);
        found = true;
        break;
      }
    }
    if (!found) {
      PT(Down) stashed = cdata_parent->modify_stashed();
      for (di = stashed->begin(); di != stashed->end(); ++di) {
        if ((*di).get_child() == child_node) {
          stashed->erase(di);
          found = true;
          break;
        }
      }
    }
    nassertv(found);
  }
    
  // Finally, break the NodePathComponent connection.
  sever_connection(parent_node, child_node, pipeline_stage, current_thread);

  parent_node->force_bounds_stale(pipeline_stage, current_thread);
  parent_node->children_changed();
  parent_node->mark_bam_modified();
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
         bool as_stashed, int pipeline_stage, Thread *current_thread) {
  bool any_ok = false;

  if (new_parent != (NodePathComponent *)NULL &&
      !new_parent->get_node()->verify_child_no_cycles(child->get_node())) {
    // Whoops, adding this child node would introduce a cycle in the
    // scene graph.
    return false;
  }

  for (int pipeline_stage_i = pipeline_stage;
       pipeline_stage_i >= 0; 
       --pipeline_stage_i) {
    if (reparent_one_stage(new_parent, child, sort, as_stashed, 
                           pipeline_stage_i, current_thread)) {
      any_ok = true;
    }
  }

  if (new_parent != (NodePathComponent *)NULL) {
    new_parent->get_node()->children_changed();
    new_parent->get_node()->mark_bam_modified();
  }
  child->get_node()->parents_changed();
  child->get_node()->mark_bam_modified();

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
                   int sort, bool as_stashed, int pipeline_stage, 
                   Thread *current_thread) {
  nassertr(child != (NodePathComponent *)NULL, false);

  // Keep a reference count to the new parent, since detaching the
  // child might lose the count.
  PT(NodePathComponent) keep_parent = new_parent;

  if (!child->is_top_node(pipeline_stage, current_thread)) {
    detach(child, pipeline_stage, current_thread);
  }

  if (new_parent != (NodePathComponent *)NULL) {
    PandaNode *child_node = child->get_node();
    PandaNode *parent_node = new_parent->get_node();

    {
      CDStageReader cdata_child(child_node->_cycler, pipeline_stage, current_thread);
      int parent_index = child_node->do_find_parent(parent_node, cdata_child);

      if (parent_index >= 0) {
        // Whoops, there's already another instance of the child there.
        return false;
      }
    }

    // Redirect the connection to the indicated new parent.
    child->set_next(new_parent, pipeline_stage, current_thread);
    
    // Now reattach the child node at the indicated sort position.
    {
      CDStageWriter cdata_parent(parent_node->_cycler, pipeline_stage, current_thread);
      CDStageWriter cdata_child(child_node->_cycler, pipeline_stage, current_thread);

      if (as_stashed) {
        cdata_parent->modify_stashed()->insert(DownConnection(child_node, sort));
      } else {
        cdata_parent->modify_down()->insert(DownConnection(child_node, sort));
      }
      cdata_child->modify_up()->insert(UpConnection(parent_node));

#ifndef NDEBUG
      // The NodePathComponent should already be in the set.
      {
        LightReMutexHolder holder(child_node->_paths_lock);
        nassertr(child_node->_paths.find(child) != child_node->_paths.end(), false);
      }
#endif // NDEBUG
    }

    child_node->fix_path_lengths(pipeline_stage, current_thread);
    parent_node->force_bounds_stale(pipeline_stage, current_thread);
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
              int pipeline_stage, Thread *current_thread) {
  nassertr(parent != (NodePathComponent *)NULL, (NodePathComponent *)NULL);
  PandaNode *parent_node = parent->get_node();

  LightReMutexHolder holder(child_node->_paths_lock);

  // First, walk through the list of NodePathComponents we already
  // have on the child, looking for one that already exists,
  // referencing the indicated parent component.
  Paths::const_iterator pi;
  for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
    if ((*pi)->get_next(pipeline_stage, current_thread) == parent) {
      // If we already have such a component, just return it.
      return (*pi);
    }
  }
    
  // We don't already have a NodePathComponent referring to this
  // parent-child relationship.  Are they actually related?
  CDStageReader cdata_child(child_node->_cycler, pipeline_stage, current_thread);
  int parent_index = child_node->do_find_parent(parent_node, cdata_child);
  
  if (parent_index >= 0) {
    // They are.  Create and return a new one.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, parent, pipeline_stage, current_thread);
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
get_top_component(PandaNode *child_node, bool force, int pipeline_stage, 
                  Thread *current_thread) {
  LightReMutexHolder holder(child_node->_paths_lock);

  // Walk through the list of NodePathComponents we already have on
  // the child, looking for one that already exists as a top node.
  Paths::const_iterator pi;
  for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
    if ((*pi)->is_top_node(pipeline_stage, current_thread)) {
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
                          pipeline_stage, current_thread);
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
get_generic_component(bool accept_ambiguity, int pipeline_stage, 
                      Thread *current_thread) {
  bool ambiguity_detected = false;
  PT(NodePathComponent) result = 
    r_get_generic_component(accept_ambiguity, ambiguity_detected,
                            pipeline_stage, current_thread);

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
                        int pipeline_stage, Thread *current_thread) {
  PT(PandaNode) parent_node;

  {
    CDStageReader cdata(_cycler, pipeline_stage, current_thread);
    
    int num_parents = cdata->get_up()->size();
    if (num_parents == 0) {
      // No parents; no ambiguity.  This is the root.
      return get_top_component(this, true, pipeline_stage, current_thread);
    } 

    PT(NodePathComponent) result;
    if (num_parents == 1) {
      // Only one parent; no ambiguity.
      PT(NodePathComponent) parent = 
        get_parent(0)->r_get_generic_component(accept_ambiguity, ambiguity_detected, pipeline_stage, current_thread);
      return get_component(parent, this, pipeline_stage, current_thread);
    }

    // Oops, multiple parents; the NodePath is ambiguous.
    if (!accept_ambiguity) {
      pgraph_cat.warning()
        << *this << " has " << num_parents
        << " parents; choosing arbitrary path to root.\n";
    }
    ambiguity_detected = true;
    CPT(Up) up = cdata->get_up();
    parent_node = (*up)[0].get_parent();
  }

  // Now that the lock is released, it's safe to recurse.
  PT(NodePathComponent) parent = 
    parent_node->r_get_generic_component(accept_ambiguity, ambiguity_detected, pipeline_stage, current_thread);
  return get_component(parent, this, pipeline_stage, current_thread);
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
  LightReMutexHolder holder(_paths_lock);
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
                 int pipeline_stage, Thread *current_thread) {
  {
    LightReMutexHolder holder(child_node->_paths_lock);
    Paths::iterator pi;
    for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
      if (!(*pi)->is_top_node(pipeline_stage, current_thread) && 
          (*pi)->get_next(pipeline_stage, current_thread)->get_node() == parent_node) {
        // Sever the component here.
        (*pi)->set_top_node(pipeline_stage, current_thread);
      }
    }
  }
  child_node->fix_path_lengths(pipeline_stage, current_thread);
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
               int pipeline_stage, Thread *current_thread) {
  {
    LightReMutexHolder holder(child_node->_paths_lock);
    Paths::iterator pi;
    for (pi = child_node->_paths.begin(); pi != child_node->_paths.end(); ++pi) {
      if ((*pi)->is_top_node(pipeline_stage, current_thread)) {
        (*pi)->set_next(parent_node->get_generic_component(false, pipeline_stage, current_thread), pipeline_stage, current_thread);
      }
    }
  }
  child_node->fix_path_lengths(pipeline_stage, current_thread);
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
fix_path_lengths(int pipeline_stage, Thread *current_thread) {
  LightReMutexHolder holder(_paths_lock);

  bool any_wrong = false;

  Paths::const_iterator pi;
  for (pi = _paths.begin(); pi != _paths.end(); ++pi) {
    if ((*pi)->fix_length(pipeline_stage, current_thread)) {
      any_wrong = true;
    }
  }
  
  // If any paths were updated, we have to recurse on all of our
  // children, since any one of those paths might be shared by any of
  // our child nodes.  Don't hold any locks while we recurse.
  if (any_wrong) {
    Children children;
    Stashed stashed;
    {
      CDStageReader cdata(_cycler, pipeline_stage, current_thread);
      children = Children(cdata);
      stashed = Stashed(cdata);
    }

    int num_children = children.get_num_children();
    int i;
    for (i = 0; i < num_children; ++i) {
      PandaNode *child_node = children.get_child(i);
      child_node->fix_path_lengths(pipeline_stage, current_thread);
    }
    int num_stashed = stashed.get_num_stashed();
    for (i = 0; i < num_stashed; ++i) {
      PandaNode *child_node = stashed.get_stashed(i);
      child_node->fix_path_lengths(pipeline_stage, current_thread);
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
//     Function: PandaNode::do_find_child
//       Access: Private
//  Description: The private implementation of find_child().
////////////////////////////////////////////////////////////////////
int PandaNode::
do_find_child(PandaNode *node, const PandaNode::Down *down) const {
  nassertr(node != (PandaNode *)NULL, -1);

  // We have to search for the child by brute force, since we don't
  // know what sort index it was added as.
  Down::const_iterator di;
  for (di = down->begin(); di != down->end(); ++di) {
    if ((*di).get_child() == node) {
      return di - down->begin();
    }
  }

  return -1;
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
PandaNode::CDStageWriter PandaNode::
update_bounds(int pipeline_stage, PandaNode::CDLockedStageReader &cdata) {
  // We might need to try this a couple of times, in case someone else
  // steps on our result.
  if (drawmask_cat.is_debug()) {
    drawmask_cat.debug(false) 
      << *this << "::update_bounds() {\n";
  }
  Thread *current_thread = cdata.get_current_thread();

  do {
    // Grab the last_update counter.
    UpdateSeq last_update = cdata->_last_update;
    UpdateSeq next_update = cdata->_next_update;
    nassertr(last_update != next_update, CDStageWriter(_cycler, pipeline_stage, cdata));

    // Start with a clean slate.
    CollideMask net_collide_mask = cdata->_into_collide_mask;
    DrawMask net_draw_control_mask, net_draw_show_mask;
    bool renderable = is_renderable();

    if (renderable) {
      // If this node is itself renderable, it contributes to the net
      // draw mask.
      net_draw_control_mask = cdata->_draw_control_mask;
      net_draw_show_mask = cdata->_draw_show_mask;
    }

    if (drawmask_cat.is_debug()) {
      drawmask_cat.debug(false)
        << "net_draw_control_mask = " << net_draw_control_mask
        << "\nnet_draw_show_mask = " << net_draw_show_mask
        << "\n";
    }
    CPT(RenderAttrib) off_clip_planes = cdata->_state->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (off_clip_planes == (RenderAttrib *)NULL) {
      off_clip_planes = ClipPlaneAttrib::make();
    }

    // Also get the list of the node's children.
    Children children(cdata);

    int num_vertices = cdata->_internal_vertices;

    // Now that we've got all the data we need from the node, we can
    // release the lock.
    _cycler.release_read_stage(pipeline_stage, cdata.take_pointer());

    int num_children = children.get_num_children();

    // We need to keep references to the bounding volumes, since in a
    // threaded environment the pointers might go away while we're
    // working (since we're not holding a lock on our set of children
    // right now).  But we also need the regular pointers, to pass to
    // BoundingVolume::around().
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    pvector<CPT(BoundingVolume) > child_volumes_ref;
    child_volumes_ref.reserve(num_children + 1);
#endif
    const BoundingVolume **child_volumes = (const BoundingVolume **)alloca(sizeof(BoundingVolume *) * (num_children + 1));
    int child_volumes_i = 0;
  
    bool all_box = true;
    CPT(BoundingVolume) internal_bounds = 
      get_internal_bounds(pipeline_stage, current_thread);

    if (!internal_bounds->is_empty()) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
      child_volumes_ref.push_back(internal_bounds);
#endif
      nassertr(child_volumes_i < num_children + 1, CDStageWriter(_cycler, pipeline_stage, cdata));
      child_volumes[child_volumes_i++] = internal_bounds;
      if (internal_bounds->as_bounding_box() == NULL) {
        all_box = false;
      }
    }

    // Now expand those contents to include all of our children.

    for (int i = 0; i < num_children; ++i) {
      PandaNode *child = children.get_child(i);

      const ClipPlaneAttrib *orig_cp = DCAST(ClipPlaneAttrib, off_clip_planes);
      
      CDLockedStageReader child_cdata(child->_cycler, pipeline_stage, current_thread);
      if (child_cdata->_last_update != child_cdata->_next_update) {
        // Child needs update.
        CDStageWriter child_cdataw = child->update_bounds(pipeline_stage, child_cdata);
      
        net_collide_mask |= child_cdataw->_net_collide_mask;

        if (drawmask_cat.is_debug()) {
          drawmask_cat.debug(false)
            << "\nchild update " << *child << ":\n";
        }

        DrawMask child_control_mask = child_cdataw->_net_draw_control_mask;
        DrawMask child_show_mask = child_cdataw->_net_draw_show_mask;
        if (!(child_control_mask | child_show_mask).is_zero()) {
          // This child includes a renderable node or subtree.  Thus,
          // we should propagate its draw masks.
          renderable = true;

          // For each bit position in the masks, we have assigned the
          // following semantic meaning.  The number on the left
          // represents the pairing of the corresponding bit from the
          // control mask and from the show mask:
          
          //   00 : not a renderable node   (control 0, show 0)
          //   01 : a normally visible node (control 0, show 1)
          //   10 : a hidden node           (control 1, show 0)
          //   11 : a show-through node     (control 1, show 1)

          // Now, when we accumulate these masks, we want to do so
          // according to the following table, for each bit position:

          //          00   01   10   11     (child)
          //        ---------------------
          //     00 | 00   01   10   11
          //     01 | 01   01   01*  11
          //     10 | 10   01*  10   11
          //     11 | 11   11   11   11
          // (parent)

          // This table is almost the same as the union of both masks,
          // with one exception, marked with a * in the above table:
          // if one is 10 and the other is 01--that is, one is hidden
          // and the other is normally visible--then the result should
          // be 01, normally visible.  This is because we only want to
          // propagate the hidden bit upwards if *all* renderable
          // nodes are hidden.

          // Get the set of exception bits for which the above rule
          // applies.  These are the bits for which both bits have
          // flipped, but which were not the same in the original.
          DrawMask exception_mask = (net_draw_control_mask ^ child_control_mask) & (net_draw_show_mask ^ child_show_mask);
          exception_mask &= (net_draw_control_mask ^ net_draw_show_mask);

          if (drawmask_cat.is_debug()) {
            drawmask_cat.debug(false)
              << "exception_mask = " << exception_mask << "\n";
          }

          // Now compute the union, applying the above exception.
          net_draw_control_mask |= child_control_mask;
          net_draw_show_mask |= child_show_mask;
          
          net_draw_control_mask &= ~exception_mask;
          net_draw_show_mask |= exception_mask;
        }

        if (drawmask_cat.is_debug()) {
          drawmask_cat.debug(false)
            << "child_control_mask = " << child_control_mask
            << "\nchild_show_mask = " << child_show_mask
            << "\nnet_draw_control_mask = " << net_draw_control_mask
            << "\nnet_draw_show_mask = " << net_draw_show_mask
            << "\n";
        }
            
        off_clip_planes = orig_cp->compose_off(child_cdataw->_off_clip_planes);
        if (!child_cdataw->_external_bounds->is_empty()) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
          child_volumes_ref.push_back(child_cdataw->_external_bounds);
#endif
          nassertr(child_volumes_i < num_children + 1, CDStageWriter(_cycler, pipeline_stage, cdata));
          child_volumes[child_volumes_i++] = child_cdataw->_external_bounds;
          if (child_cdataw->_external_bounds->as_bounding_box() == NULL) {
            all_box = false;
          }
        }
        num_vertices += child_cdataw->_nested_vertices;

      } else {
        // Child is good.
        net_collide_mask |= child_cdata->_net_collide_mask;

        // See comments in similar block above.
        if (drawmask_cat.is_debug()) {
          drawmask_cat.debug(false)
            << "\nchild fresh " << *child << ":\n";
        }
        DrawMask child_control_mask = child_cdata->_net_draw_control_mask;
        DrawMask child_show_mask = child_cdata->_net_draw_show_mask;
        if (!(child_control_mask | child_show_mask).is_zero()) {
          renderable = true;

          DrawMask exception_mask = (net_draw_control_mask ^ child_control_mask) & (net_draw_show_mask ^ child_show_mask);
          exception_mask &= (net_draw_control_mask ^ net_draw_show_mask);

          if (drawmask_cat.is_debug()) {
            drawmask_cat.debug(false)
              << "exception_mask = " << exception_mask << "\n";
          }

          // Now compute the union, applying the above exception.
          net_draw_control_mask |= child_control_mask;
          net_draw_show_mask |= child_show_mask;
          
          net_draw_control_mask &= ~exception_mask;
          net_draw_show_mask |= exception_mask;
        }

        if (drawmask_cat.is_debug()) {
          drawmask_cat.debug(false)
            << "child_control_mask = " << child_control_mask
            << "\nchild_show_mask = " << child_show_mask
            << "\nnet_draw_control_mask = " << net_draw_control_mask
            << "\nnet_draw_show_mask = " << net_draw_show_mask
            << "\n";
        }

        off_clip_planes = orig_cp->compose_off(child_cdata->_off_clip_planes);
        if (!child_cdata->_external_bounds->is_empty()) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
          child_volumes_ref.push_back(child_cdata->_external_bounds);
#endif
          nassertr(child_volumes_i < num_children + 1, CDStageWriter(_cycler, pipeline_stage, cdata));
          child_volumes[child_volumes_i++] = child_cdata->_external_bounds;
          if (child_cdata->_external_bounds->as_bounding_box() == NULL) {
            all_box = false;
          }
        }
        num_vertices += child_cdata->_nested_vertices;
      }
    }

    {
      // Now grab the write lock on this node.
      CDStageWriter cdataw(_cycler, pipeline_stage, current_thread);
      if (last_update == cdataw->_last_update && 
          next_update == cdataw->_next_update) {
        // Great, no one has monkeyed with these while we were computing
        // the cache.  Safe to store the computed values and return.
        cdataw->_net_collide_mask = net_collide_mask;

        if (renderable) {
          // Any explicit draw control mask on this node trumps anything
          // inherited from below, except a show-through.
          DrawMask draw_control_mask = cdataw->_draw_control_mask;
          DrawMask draw_show_mask = cdataw->_draw_show_mask;

          DrawMask show_through_mask = net_draw_control_mask & net_draw_show_mask;
          
          net_draw_control_mask |= draw_control_mask;
          net_draw_show_mask = (net_draw_show_mask & ~draw_control_mask) | (draw_show_mask & draw_control_mask);

          net_draw_show_mask |= show_through_mask;
          
          // There are renderable nodes below, so the implicit draw
          // bits are all on.
          cdataw->_net_draw_control_mask = net_draw_control_mask;
          cdataw->_net_draw_show_mask = net_draw_show_mask | ~net_draw_control_mask;
          if (drawmask_cat.is_debug()) {
            drawmask_cat.debug(false)
              << "renderable, set mask " << cdataw->_net_draw_show_mask << "\n";
          }
        } else {
          // There are no renderable nodes below, so the implicit draw
          // bits are all off.  Also, we don't care about the draw
          // mask on this particular node (since nothing below it is
          // renderable anyway).
          cdataw->_net_draw_control_mask = net_draw_control_mask;
          cdataw->_net_draw_show_mask = net_draw_show_mask;
          if (drawmask_cat.is_debug()) {
            drawmask_cat.debug(false)
              << "not renderable, set mask " << cdataw->_net_draw_show_mask << "\n";
          }
        }

        cdataw->_off_clip_planes = off_clip_planes;
        cdataw->_nested_vertices = num_vertices;

        CPT(TransformState) transform = get_transform(current_thread);
        PT(GeometricBoundingVolume) gbv;

        BoundingVolume::BoundsType btype = cdataw->_bounds_type;
        if (btype == BoundingVolume::BT_default) {
          btype = bounds_type;
        }

        if (btype == BoundingVolume::BT_box ||
            (btype != BoundingVolume::BT_sphere && all_box && transform->is_identity())) {
          // If all of the child volumes are a BoundingBox, and we
          // have no transform, then our volume is also a
          // BoundingBox.
          
          gbv = new BoundingBox;
        } else {
          // Otherwise, it's a sphere.
          gbv = new BoundingSphere;
        }
        
        if (child_volumes_i > 0) {
          const BoundingVolume **child_begin = &child_volumes[0];
          const BoundingVolume **child_end = child_begin + child_volumes_i;
          ((BoundingVolume *)gbv)->around(child_begin, child_end);
        }
        
        // If we have a transform, apply it to the bounding volume we
        // just computed.
        if (!transform->is_identity()) {
          gbv->xform(transform->get_mat());
        }

        cdataw->_external_bounds = gbv;
        cdataw->_last_update = next_update;

        if (drawmask_cat.is_debug()) {
          drawmask_cat.debug(false)
            << "} " << *this << "::update_bounds();\n";
        }

        nassertr(cdataw->_last_update == cdataw->_next_update, cdataw);
    
        // Even though implicit bounding volume is not (yet?) part of
        // the bam stream.
        mark_bam_modified();
        return cdataw;
      }
      
      if (cdataw->_last_update == cdataw->_next_update) {
        // Someone else has computed the cache for us.  OK.
        return cdataw;
      }
    }

    // We need to go around again.  Release the write lock, and grab
    // the read lock back.
    cdata = CDLockedStageReader(_cycler, pipeline_stage, current_thread);

    if (cdata->_last_update == cdata->_next_update) {
      // Someone else has computed the cache for us while we were
      // diddling with the locks.  OK.
      return CDStageWriter(_cycler, pipeline_stage, cdata);
    }

  } while (true);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::set_scene_root_func
//       Access: Public, Static
//  Description: This is used by the GraphicsEngine to hook in a
//               pointer to the scene_root_func(), the function to
//               determine whether the node is an active scene root.
//               This back-pointer is necessary because we can't make
//               calls directly into GraphicsEngine, which is in the
//               display module.
////////////////////////////////////////////////////////////////////
void PandaNode::
set_scene_root_func(SceneRootFunc *func) {
  _scene_root_func = func;
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

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::update_bam_nested
//       Access: Public, Virtual
//  Description: Called by the BamWriter when this object has not
//               itself been modified recently, but it should check
//               its nested objects for updates.
////////////////////////////////////////////////////////////////////
void PandaNode::
update_bam_nested(BamWriter *manager) {
  CDReader cdata(_cycler);
  cdata->update_bam_nested(manager);
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

  manager->read_cdata(scan, _cycler);
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
//     Function: PandaNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CData::
CData() :
  _state(RenderState::make_empty()),
  _transform(TransformState::make_identity()),
  _prev_transform(TransformState::make_identity()),

  _effects(RenderEffects::make_empty()),
  _draw_control_mask(DrawMask::all_off()),
  _draw_show_mask(DrawMask::all_on()),
  _into_collide_mask(CollideMask::all_off()),
  _bounds_type(BoundingVolume::BT_default),
  _user_bounds(NULL),
  _final_bounds(false),
  _fancy_bits(0),

  _net_collide_mask(CollideMask::all_off()),
  _net_draw_control_mask(DrawMask::all_off()),
  _net_draw_show_mask(DrawMask::all_off()),

  _down(new PandaNode::Down(PandaNode::get_class_type())),
  _stashed(new PandaNode::Down(PandaNode::get_class_type())),
  _up(new PandaNode::Up(PandaNode::get_class_type()))
{
  ++_next_update;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CData::
CData(const PandaNode::CData &copy) :
  BoundsData(copy),
  _state(copy._state),
  _transform(copy._transform),
  _prev_transform(copy._prev_transform),

  _effects(copy._effects),
  _tag_data(copy._tag_data),
  // _python_tag_data appears below.
  _draw_control_mask(copy._draw_control_mask),
  _draw_show_mask(copy._draw_show_mask),
  _into_collide_mask(copy._into_collide_mask),
  _bounds_type(copy._bounds_type),
  _user_bounds(copy._user_bounds),
  _final_bounds(copy._final_bounds),
  _fancy_bits(copy._fancy_bits),

  _net_collide_mask(copy._net_collide_mask),
  _net_draw_control_mask(copy._net_draw_control_mask),
  _net_draw_show_mask(copy._net_draw_show_mask),
  _off_clip_planes(copy._off_clip_planes),
  _nested_vertices(copy._nested_vertices),
  _external_bounds(copy._external_bounds),
  _last_update(copy._last_update),
  _next_update(copy._next_update),

  _down(copy._down),
  _stashed(copy._stashed),
  _up(copy._up)
{
  // Note that this copy constructor is not used by the PandaNode copy
  // constructor!  Any elements that must be copied between nodes
  // should also be explicitly copied there.

#ifdef HAVE_PYTHON
  // Copy and increment all of the Python objects held by the other
  // node.
  _python_tag_data = copy._python_tag_data;
  inc_py_refs();
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CData::
~CData() {
#ifdef HAVE_PYTHON
  // Free all of the Python objects held by this node.
  dec_py_refs();
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PandaNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _state);
  manager->write_pointer(dg, _transform);

  //
  manager->write_pointer(dg, _effects);

  dg.add_uint32(_draw_control_mask.get_word());
  dg.add_uint32(_draw_show_mask.get_word());
  dg.add_uint32(_into_collide_mask.get_word());
  dg.add_uint8(_bounds_type);

  dg.add_uint32(_tag_data.size());
  TagData::const_iterator ti;
  for (ti = _tag_data.begin(); ti != _tag_data.end(); ++ti) {
    dg.add_string((*ti).first);
    dg.add_string((*ti).second);
  }

  //
  write_up_list(*get_up(), manager, dg);
  write_down_list(*get_down(), manager, dg);
  write_down_list(*get_stashed(), manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::update_bam_nested
//       Access: Public
//  Description: Called by the BamWriter when this object has not
//               itself been modified recently, but it should check
//               its nested objects for updates.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
update_bam_nested(BamWriter *manager) const {
  // No need to check the state pointers for updates, since they're
  // all immutable objects.
  //manager->consider_update(_state);
  //manager->consider_update(_transform);
  //manager->consider_update(_effects);

  update_up_list(*get_up(), manager);
  update_down_list(*get_down(), manager);
  update_down_list(*get_stashed(), manager);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int PandaNode::CData::
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

  //

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

  //

  // Get the parent and child pointers.
  pi += complete_up_list(*modify_up(), "up", p_list + pi, manager);
  pi += complete_down_list(*modify_down(), "down", p_list + pi, manager);
  pi += complete_down_list(*modify_stashed(), "stashed", p_list + pi, manager);

  // Since the _effects and _states members have been finalized by
  // now, this should be safe.
  set_fancy_bit(FB_transform, !_transform->is_identity());
  set_fancy_bit(FB_state, !_state->is_empty());
  set_fancy_bit(FB_effects, !_effects->is_empty());
  set_fancy_bit(FB_tag, !_tag_data.empty());

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  // Read the state and transform pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);

  //
  // Read the effects pointer.
  manager->read_pointer(scan);

  if (manager->get_file_minor_ver() < 2) {
    DrawMask draw_mask;
    draw_mask.set_word(scan.get_uint32());

    if (draw_mask == DrawMask::all_off()) {
      // Hidden.
      _draw_control_mask = _overall_bit;
      _draw_show_mask = ~_overall_bit;

    } else if (draw_mask == DrawMask::all_on()) {
      // Normally visible.
      _draw_control_mask = DrawMask::all_off();
      _draw_show_mask = DrawMask::all_on();

    } else {
      // Some per-camera combination.
      draw_mask &= ~_overall_bit;
      _draw_control_mask = ~draw_mask;
      _draw_show_mask = draw_mask;
    }

  } else {
    _draw_control_mask.set_word(scan.get_uint32());
    _draw_show_mask.set_word(scan.get_uint32());
  }

  _into_collide_mask.set_word(scan.get_uint32());

  _bounds_type = BoundingVolume::BT_default;
  if (manager->get_file_minor_ver() >= 19) {
    _bounds_type = (BoundingVolume::BoundsType)scan.get_uint8();
  }

  // Read in the tag list.
  int num_tags = scan.get_uint32();
  for (int i = 0; i < num_tags; i++) {
    string key = scan.get_string();
    string value = scan.get_string();
    _tag_data[key] = value;
  }

  //
  fillin_up_list(*modify_up(), "up", scan, manager);
  fillin_down_list(*modify_down(), "down", scan, manager);
  fillin_down_list(*modify_stashed(), "stashed", scan, manager);
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::inc_py_refs
//       Access: Public
//  Description: Increments the reference counts on all held Python
//               objects.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
inc_py_refs() {
  if (!_python_tag_data.empty()) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    // This might happen at any time, so be sure the Python state is
    // ready for it.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif
    PythonTagData::const_iterator ti;
    for (ti = _python_tag_data.begin();
         ti != _python_tag_data.end();
         ++ti) {
      PyObject *value = (*ti).second;
      Py_XINCREF(value);
    }
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::dec_py_refs
//       Access: Public
//  Description: Decrements the reference counts on all held Python
//               objects.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
dec_py_refs() {
  if (!_python_tag_data.empty()) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    // This might happen at any time, so be sure the Python state is
    // ready for it.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif
    
    PythonTagData::const_iterator ti;
    for (ti = _python_tag_data.begin();
         ti != _python_tag_data.end();
         ++ti) {
      PyObject *value = (*ti).second;
      Py_XDECREF(value);
    }
    
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::write_up_list
//       Access: Public
//  Description: Writes the indicated list of parent node pointers to
//               the datagram.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
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
//     Function: PandaNode::CData::write_down_list
//       Access: Public
//  Description: Writes the indicated list of child node pointers to
//               the datagram.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
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
//     Function: PandaNode::CData::update_up_list
//       Access: Public
//  Description: Calls consider_update on each node of the indicated
//               up list.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
update_up_list(const PandaNode::Up &up_list, BamWriter *manager) const {
  Up::const_iterator ui;
  for (ui = up_list.begin(); ui != up_list.end(); ++ui) {
    PandaNode *parent_node = (*ui).get_parent();
    if (manager->has_object(parent_node)) {
      manager->consider_update(parent_node);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::update_down_list
//       Access: Public
//  Description: Calls consider_update on each node of the indicated
//               up list.
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
update_down_list(const PandaNode::Down &down_list, BamWriter *manager) const {
  Down::const_iterator di;
  for (di = down_list.begin(); di != down_list.end(); ++di) {
    PandaNode *child_node = (*di).get_child();
    manager->consider_update(child_node);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::complete_up_list
//       Access: Public
//  Description: Calls complete_pointers() on the list of parent node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CData::
complete_up_list(PandaNode::Up &up_list, const string &tag,
                 TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  int num_parents = manager->get_int_tag(tag);
  Up new_up_list(PandaNode::get_class_type());
  new_up_list.reserve(num_parents);
  for (int i = 0; i < num_parents; i++) {
    PandaNode *parent_node = DCAST(PandaNode, p_list[pi++]);
    UpConnection connection(parent_node);
    new_up_list.push_back(connection);
  }

  // Now we should sort the list, since the sorting is based on
  // pointer order, which might be different from one session to the
  // next.
  new_up_list.sort();

  // Make it permanent.
  up_list.swap(new_up_list);
  new_up_list.clear();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::complete_down_list
//       Access: Public
//  Description: Calls complete_pointers() on the list of child node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CData::
complete_down_list(PandaNode::Down &down_list, const string &tag,
                   TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  BamReaderAuxDataDown *aux;
  DCAST_INTO_R(aux, manager->get_aux_tag(tag), pi);

  Down &new_down_list = aux->_down_list;
  for (Down::iterator di = new_down_list.begin();
       di != new_down_list.end(); 
       ++di) {
    PandaNode *child_node = DCAST(PandaNode, p_list[pi++]);
    (*di).set_child(child_node);
  }

  // Unlike the up list, we should *not* sort the down list.  The down
  // list is stored in a specific order, not related to pointer order;
  // and this order should be preserved from one session to the next.

  // Make it permanent.
  down_list.swap(new_down_list);
  new_down_list.clear();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::fillin_up_list
//       Access: Public
//  Description: Reads the indicated list parent node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
fillin_up_list(PandaNode::Up &up_list, const string &tag,
               DatagramIterator &scan, BamReader *manager) {
  int num_parents = scan.get_uint16();
  manager->set_int_tag(tag, num_parents);
  manager->read_pointers(scan, num_parents);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::fillin_down_list
//       Access: Public
//  Description: Reads the indicated list child node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
fillin_down_list(PandaNode::Down &down_list, const string &tag,
                 DatagramIterator &scan, BamReader *manager) {
  int num_children = scan.get_uint16();

  // Create a temporary down_list, with the right number of elements,
  // but a NULL value for each pointer (we'll fill in the pointers
  // later).  We need to do this to associate the sort values with
  // their pointers.
  Down new_down_list(PandaNode::get_class_type());
  new_down_list.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    manager->read_pointer(scan);
    int sort = scan.get_int32();
    DownConnection connection(NULL, sort);
    new_down_list.push_back(connection);
  }

  // Now store the temporary down_list in the BamReader, so we can get
  // it during the call to complete_down_list().
  PT(BamReaderAuxDataDown) aux = new BamReaderAuxDataDown;
  aux->_down_list.swap(new_down_list);
  manager->set_aux_tag(tag, aux);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodePipelineReader::check_bounds
//       Access: Public
//  Description: Ensures that the bounding volume is properly computed
//               on this node.
////////////////////////////////////////////////////////////////////
void PandaNodePipelineReader::
check_bounds() const {
  if (_cdata->_last_update != _cdata->_next_update) {
    // The cache is stale; it needs to be rebuilt.

    // We'll need to get a fresh read pointer, since another thread
    // might already have modified the pointer on the object since we
    // queried it.
#ifdef DO_PIPELINING
    node_unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING
    ((PandaNodePipelineReader *)this)->_cdata = NULL;
    int pipeline_stage = _current_thread->get_pipeline_stage();
    PandaNode::CDLockedStageReader fresh_cdata(_object->_cycler, pipeline_stage, _current_thread);
    if (fresh_cdata->_last_update == fresh_cdata->_next_update) {
      // What luck, some other thread has already freshened the
      // cache for us.  Save the new pointer, and let the lock
      // release itself.
      if (_cdata != (const PandaNode::CData *)fresh_cdata) {
        ((PandaNodePipelineReader *)this)->_cdata = fresh_cdata;
#ifdef DO_PIPELINING
        _cdata->node_ref();
#endif  // DO_PIPELINING
      }
      
    } else {
      // No, the cache is still stale.  We have to do the work of
      // freshening it.
      PStatTimer timer(PandaNode::_update_bounds_pcollector);
      PandaNode::CDStageWriter cdataw = ((PandaNode *)_object)->update_bounds(pipeline_stage, fresh_cdata);
      nassertv(cdataw->_last_update == cdataw->_next_update);
      // As above, we save the new pointer, and then let the lock
      // release itself.
      if (_cdata != (const PandaNode::CData *)cdataw) {
        ((PandaNodePipelineReader *)this)->_cdata = cdataw;
#ifdef DO_PIPELINING
        _cdata->node_ref();
#endif  // DO_PIPELINING
      }
    }
  }

  nassertv(_cdata->_last_update == _cdata->_next_update);
}
