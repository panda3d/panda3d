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
  manager->write_pointer(dg, _effects);
  manager->write_pointer(dg, _transform);

  dg.add_uint32(_draw_mask.get_word());
  dg.add_uint32(_into_collide_mask.get_word());

  write_up_list(_up, manager, dg);
  write_down_list(_down, manager, dg);
  write_down_list(_stashed, manager, dg);

  dg.add_uint32(_tag_data.size());
  TagData::const_iterator ti;
  for (ti = _tag_data.begin(); ti != _tag_data.end(); ++ti) {
    dg.add_string((*ti).first);
    dg.add_string((*ti).second);
  }
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

  // Get the state, effects, and transform pointers.
  _state = DCAST(RenderState, p_list[pi++]);
  _effects = DCAST(RenderEffects, p_list[pi++]);
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
  manager->finalize_now((RenderEffects *)_effects.p());
  manager->finalize_now((TransformState *)_transform.p());

  // Get the parent and child pointers.
  pi += complete_up_list(_up, p_list + pi, manager);
  pi += complete_down_list(_down, p_list + pi, manager);
  pi += complete_down_list(_stashed, p_list + pi, manager);

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
  // Read the state, effects, and transform pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);
  manager->read_pointer(scan);

  _draw_mask.set_word(scan.get_uint32());
  if (manager->get_file_minor_ver() >= 12) {
    _into_collide_mask.set_word(scan.get_uint32());
  }

  // Read the parent and child pointers.
  fillin_up_list(_up, scan, manager);
  fillin_down_list(_down, scan, manager);
  fillin_down_list(_stashed, scan, manager);

  // Read in the tag list.
  if (manager->get_file_minor_ver() >= 4) {
    int num_tags = scan.get_uint32();
    for (int i = 0; i < num_tags; i++) {
      string key = scan.get_string();
      string value = scan.get_string();
      _tag_data[key] = value;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::write_up_list
//       Access: Private
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
//       Access: Private
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
//     Function: PandaNode::CData::complete_up_list
//       Access: Private
//  Description: Calls complete_pointers() on the list of parent node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CData::
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
//     Function: PandaNode::CData::complete_down_list
//       Access: Private
//  Description: Calls complete_pointers() on the list of child node
//               pointers.
////////////////////////////////////////////////////////////////////
int PandaNode::CData::
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
//     Function: PandaNode::CData::fillin_up_list
//       Access: Private
//  Description: Reads the indicated list parent node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
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
//     Function: PandaNode::CData::fillin_down_list
//       Access: Private
//  Description: Reads the indicated list child node pointers from
//               the datagram (or at least calls read_pointer() for
//               each one).
////////////////////////////////////////////////////////////////////
void PandaNode::CData::
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
ChildrenCopy(const PandaNode::CDReader &cdata) {
  Children cr(cdata);
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    _list.push_back(cr.get_child(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
~PandaNode() {
  // We shouldn't have any parents left by the time we destruct, or
  // there's a refcount fault somewhere.
#ifndef NDEBUG
  {
    CDReader cdata(_cycler);
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
  TypedWritable(copy),
  ReferenceCount(copy),
  Namable(copy)
{
  // Copying a node does not copy its children.

  // Copy the other node's state.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_state = copy_cdata->_state;
  cdata->_effects = copy_cdata->_effects;
  cdata->_transform = copy_cdata->_transform;
  cdata->_prev_transform = copy_cdata->_prev_transform;
  cdata->_tag_data = copy_cdata->_tag_data;
  cdata->_draw_mask = copy_cdata->_draw_mask;
  cdata->_into_collide_mask = copy_cdata->_into_collide_mask;
  cdata->_fixed_internal_bound = copy_cdata->_fixed_internal_bound;
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
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
int PandaNode::
find_child(PandaNode *node) const {
  nassertr(node != (PandaNode *)NULL, -1);
  CDReader cdata(_cycler);

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
  
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_down.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(this));

  new_connection(this, child_node);

  // Mark the bounding volumes stale.
  force_bound_stale();

  // Call callback hooks.
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the nth child from the node.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_child(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_down.size());
  
  PT(PandaNode) child_node = cdata->_down[n].get_child();
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_down.erase(cdata->_down.begin() + n);
  int num_erased = cdata_child->_up.erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node);

  // Mark the bounding volumes stale.
  force_bound_stale();

  // Call callback hooks.
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
  // First, look for the parent in the child's up list, to ensure the
  // child is known.
  int parent_index = child_node->find_parent(this);
  if (parent_index < 0) {
    // Nope, no relation.
    return false;
  }

  int child_index = find_child(child_node);
  if (child_index >= 0) {
    // The child exists; remove it.
    remove_child(child_index);
    return true;
  }

  int stashed_index = find_stashed(child_node);
  if (stashed_index >= 0) {
    // The child has been stashed; remove it.
    remove_stashed(stashed_index);
    return true;
  }

  // Never heard of this child.  This shouldn't be possible, because
  // the parent was in the child's up list, above.  Must be some
  // internal error.
  nassertr(false, false);
  return false;
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
  // First, look for the parent in the child's up list, to ensure the
  // child is known.
  int parent_index = orig_child->find_parent(this);
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

  int child_index = find_child(orig_child);
  if (child_index >= 0) {
    // The child exists; replace it.
    CDWriter cdata(_cycler);
    DownConnection &down = cdata->_down[child_index];
    nassertr(down.get_child() == orig_child, false);
    down.set_child(new_child);

  } else {
    int stashed_index = find_stashed(orig_child);
    if (stashed_index >= 0) {
      // The child has been stashed; remove it.
      CDWriter cdata(_cycler);
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

  CDWriter cdata_orig_child(orig_child->_cycler);
  CDWriter cdata_new_child(new_child->_cycler);
  
  cdata_new_child->_up.insert(UpConnection(this));
  int num_erased = cdata_orig_child->_up.erase(UpConnection(this));
  nassertr(num_erased == 1, false);

  sever_connection(this, orig_child);
  orig_child->parents_changed();

  new_connection(this, new_child);
  new_child->parents_changed();

  // Mark the bounding volumes stale.
  force_bound_stale();
  children_changed();

  return true;
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
////////////////////////////////////////////////////////////////////
void PandaNode::
stash_child(int child_index) {
  nassertv(child_index >= 0 && child_index < get_num_children());

  // Save a reference count for ourselves.  I don't think this should
  // be necessary, but there are occasional crashes in stash() during
  // furniture moving mode.  Perhaps this will eliminate those
  // crashes.
  PT(PandaNode) self = this;

  PT(PandaNode) child_node = get_child(child_index);
  int sort = get_child_sort(child_index);
  
  remove_child(child_index);
  
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_stashed.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(this));

  new_connection(this, child_node);

  // Mark the bounding volumes stale.
  force_bound_stale();

  // Call callback hooks.
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
////////////////////////////////////////////////////////////////////
void PandaNode::
unstash_child(int stashed_index) {
  nassertv(stashed_index >= 0 && stashed_index < get_num_stashed());

  // Save a reference count for ourselves.  I don't think this should
  // be necessary, but there are occasional crashes in stash() during
  // furniture moving mode.  Perhaps this will eliminate those
  // crashes.
  PT(PandaNode) self = this;

  PT(PandaNode) child_node = get_stashed(stashed_index);
  int sort = get_stashed_sort(stashed_index);
  
  remove_stashed(stashed_index);
  
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_down.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(this));

  new_connection(this, child_node);

  // Mark the bounding volumes stale.
  force_bound_stale();

  // Call callback hooks.
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_stashed
//       Access: Published
//  Description: Returns the index of the indicated stashed node, if
//               it is a stashed child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
int PandaNode::
find_stashed(PandaNode *node) const {
  CDReader cdata(_cycler);

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
//     Function: PandaNode::add_stashed
//       Access: Published
//  Description: Adds a new child to the node, directly as a stashed
//               child.  The child is not added in the normal sense,
//               but will be revealed if unstash_child() is called on
//               it later.
//
//               If the same child is added to a node more than once,
//               the previous instance is first removed.
////////////////////////////////////////////////////////////////////
void PandaNode::
add_stashed(PandaNode *child_node, int sort) {
  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);
  
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_stashed.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(this));

  new_connection(this, child_node);

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
remove_stashed(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_stashed.size());
  
  PT(PandaNode) child_node = cdata->_stashed[n].get_child();
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_stashed.erase(cdata->_stashed.begin() + n);
  int num_erased = cdata_child->_up.erase(UpConnection(this));
  nassertv(num_erased == 1);

  sever_connection(this, child_node);

  // Mark the bounding volumes stale.
  force_bound_stale();

  // Call callback hooks.
  children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_all_children
//       Access: Published
//  Description: Removes all the children from the node at once,
//               including stashed children.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_all_children() {
  CDWriter cdata(_cycler);
  Down::iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    PT(PandaNode) child_node = (*di).get_child();
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_up.erase(UpConnection(this));

    sever_connection(this, child_node);
    child_node->parents_changed();
  }
  cdata->_down.clear();

  for (di = cdata->_stashed.begin(); di != cdata->_stashed.end(); ++di) {
    PT(PandaNode) child_node = (*di).get_child();
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_up.erase(UpConnection(this));

    sever_connection(this, child_node);
    child_node->parents_changed();
  }
  cdata->_stashed.clear();

  // Mark the bounding volumes stale.
  force_bound_stale();
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

  CDWriter cdataw(_cycler);
  CDReader cdatar(other->_cycler);
  TagData::const_iterator ti;
  for (ti = cdatar->_tag_data.begin();
       ti != cdatar->_tag_data.end();
       ++ti) {
    cdataw->_tag_data[(*ti).first] = (*ti).second;
  }
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
  CDReader cdata(_cycler);
  if (!cdata->_tag_data.empty()) {
    out << " [";
    list_tags(out, " ");
    out << "]";
  }
  if (!cdata->_transform->is_identity()) {
    out << " " << *cdata->_transform;
  }
  if (!cdata->_state->is_empty()) {
    out << " " << *cdata->_state;
  }
  if (!cdata->_effects->is_empty()) {
    out << " " << *cdata->_effects;
  }
  out << "\n";
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
//     Function: PandaNode::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void PandaNode::
propagate_stale_bound() {
  // Mark all of our parent nodes stale as well.
  CDWriter cdata(_cycler);
  Up::const_iterator ui;
  for (ui = cdata->_up.begin(); ui != cdata->_up.end(); ++ui) {
    PandaNode *parent_node = (*ui).get_parent();
    parent_node->mark_bound_stale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this
//               object.  The default behavior is the compute an empty
//               bounding volume; this may be overridden to extend it
//               to create a nonempty bounding volume.  However, after
//               calling this function, it is guaranteed that the
//               _bound pointer will not be shared with any other
//               stage of the pipeline, and this new pointer is
//               returned.
////////////////////////////////////////////////////////////////////
BoundingVolume *PandaNode::
recompute_bound() {
  // Get the internal bound before we do anything else, since the
  // is_bound_stale() flag may also apply to this.
  const BoundingVolume *internal_bound = &get_internal_bound();

  // Now, get ourselves a fresh, empty bounding volume.  This will
  // reset the is_bound_stale() flag if it was set.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)NULL, bound);

  // Also, recompute the net_collide_mask bits while we do this.
  CDWriter cdata(_cycler);
  cdata->_net_collide_mask = cdata->_into_collide_mask;

  // Now actually compute the bounding volume by putting it around all
  // of our child bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  // It goes around this node's internal bounding volume . . .
  child_volumes.push_back(internal_bound);

  Down::const_iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    // . . . plus each node's external bounding volume.
    PandaNode *child = (*di).get_child();
    const BoundingVolume &child_bound = child->get_bound();
    child_volumes.push_back(&child_bound);
    cdata->_net_collide_mask |= child->get_net_collide_mask();
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();

  bool success =
    bound->around(child_begin, child_end);

#ifndef NDEBUG
  if (!success) {
    pgraph_cat.error()
      << "Unable to recompute bounding volume for " << *this << ":\n"
      << "Cannot put " << bound->get_type() << " around:\n";
    for (int i = 0; i < (int)child_volumes.size(); i++) {
      pgraph_cat.error(false)
        << "  " << *child_volumes[i] << "\n";
    }
  }
#endif

  // Now, if we have a transform, apply it to the bounding volume we
  // just computed.
  const TransformState *transform = get_transform();
  if (!transform->is_identity()) {
    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->xform(transform->get_mat());
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *PandaNode::
recompute_internal_bound() {
  BoundingVolume *result = _internal_bound.recompute_bound();
  return result;
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
  CDReader from_cdata(from->_cycler);
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
//     Function: PandaNode::attach
//       Access: Private, Static
//  Description: Creates a new parent-child relationship, and returns
//               the new NodePathComponent.  If the child was already
//               attached to the indicated parent, repositions it and
//               returns the original NodePathComponent.
////////////////////////////////////////////////////////////////////
PT(NodePathComponent) PandaNode::
attach(NodePathComponent *parent, PandaNode *child_node, int sort) {
  if (parent == (NodePathComponent *)NULL) {
    // Attaching to NULL means to create a new "instance" with no
    // attachments, and no questions asked.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, (NodePathComponent *)NULL);
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_paths.insert(child);
    return child;
  }

  // See if the child was already attached to the parent.  If it was,
  // we'll use that same NodePathComponent.
  PT(NodePathComponent) child = get_component(parent, child_node);

  if (child == (NodePathComponent *)NULL) {
    // The child was not already attached to the parent, so get a new
    // component.
    child = get_top_component(child_node, true);
  }

  reparent(parent, child, sort, false);
  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::detach
//       Access: Private, Static
//  Description: Breaks a parent-child relationship.
////////////////////////////////////////////////////////////////////
void PandaNode::
detach(NodePathComponent *child) {
  nassertv(child != (NodePathComponent *)NULL);
  nassertv(!child->is_top_node());

  PT(PandaNode) child_node = child->get_node();
  PT(PandaNode) parent_node = child->get_next()->get_node();
  
  // We should actually have a parent-child relationship, since this
  // came from a NodePathComponent that ought to know about this
  // sort of thing.
  nassertv(child_node->find_parent(parent_node) >= 0);
  
  CDWriter cdata_child(child_node->_cycler);
  CDWriter cdata_parent(parent_node->_cycler);
  
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

  // Finally, break the NodePathComponent connection.
  sever_connection(parent_node, child_node);

  // Mark the bounding volumes stale.
  parent_node->force_bound_stale();

  // Call callback hooks.
  parent_node->children_changed();
  child_node->parents_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reparent
//       Access: Private, Static
//  Description: Switches a node from one parent to another.  Returns
//               true if the new connection is allowed, or false if it
//               conflicts with another instance (that is, another
//               instance of the child is already attached to the
//               indicated parent).
////////////////////////////////////////////////////////////////////
bool PandaNode::
reparent(NodePathComponent *new_parent, NodePathComponent *child, int sort,
         bool as_stashed) {
  nassertr(child != (NodePathComponent *)NULL, false);

  // Keep a reference count to the new parent, since detaching the
  // child might lose the count.
  PT(NodePathComponent) keep_parent = new_parent;

  if (!child->is_top_node()) {
    detach(child);
  }

  if (new_parent != (NodePathComponent *)NULL) {
    PandaNode *child_node = child->get_node();
    PandaNode *parent_node = new_parent->get_node();

    if (child_node->find_parent(parent_node) >= 0) {
      // Whoops, there's already another instance of the child there.
      return false;
    }

    // Redirect the connection to the indicated new parent.
    child->set_next(new_parent);
    
    // Now reattach the child node at the indicated sort position.
    CDWriter cdata_parent(parent_node->_cycler);
    CDWriter cdata_child(child_node->_cycler);

    if (as_stashed) {
      cdata_parent->_stashed.insert(DownConnection(child_node, sort));
    } else {
      cdata_parent->_down.insert(DownConnection(child_node, sort));
    }
    cdata_child->_up.insert(UpConnection(parent_node));
    
    cdata_child->_paths.insert(child);
    child_node->fix_path_lengths(cdata_child);
    
    // Mark the bounding volumes stale.
    if (!as_stashed) {
      parent_node->force_bound_stale();
    }

    // Call callback hooks.
    parent_node->children_changed();
    child_node->parents_changed();
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
get_component(NodePathComponent *parent, PandaNode *child_node) {
  nassertr(parent != (NodePathComponent *)NULL, (NodePathComponent *)NULL);
  PandaNode *parent_node = parent->get_node();

  {
    CDReader cdata_child(child_node->_cycler);

    // First, walk through the list of NodePathComponents we already
    // have on the child, looking for one that already exists,
    // referencing the indicated parent component.
    Paths::const_iterator pi;
    for (pi = cdata_child->_paths.begin(); 
         pi != cdata_child->_paths.end(); 
         ++pi) {
      if ((*pi)->get_next() == parent) {
        // If we already have such a component, just return it.
        return (*pi);
      }
    }
  }
    
  // We don't already have a NodePathComponent referring to this
  // parent-child relationship.  Are they actually related?
  int child_index = child_node->find_parent(parent_node);
  if (child_index >= 0) {
    // They are.  Create and return a new one.
    PT(NodePathComponent) child = 
      new NodePathComponent(child_node, parent);
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_paths.insert(child);
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
get_top_component(PandaNode *child_node, bool force) {
  {
    CDReader cdata_child(child_node->_cycler);

    // Walk through the list of NodePathComponents we already have on
    // the child, looking for one that already exists as a top node.
    Paths::const_iterator pi;
    for (pi = cdata_child->_paths.begin(); 
         pi != cdata_child->_paths.end(); 
         ++pi) {
      if ((*pi)->is_top_node()) {
        // If we already have such a component, just return it.
        return (*pi);
      }
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
    new NodePathComponent(child_node, (NodePathComponent *)NULL);
  CDWriter cdata_child(child_node->_cycler);
  cdata_child->_paths.insert(child);

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
get_generic_component(bool accept_ambiguity) {
  bool ambiguity_detected = false;
  PT(NodePathComponent) result = 
    r_get_generic_component(accept_ambiguity, ambiguity_detected);

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
r_get_generic_component(bool accept_ambiguity, bool &ambiguity_detected) {
  int num_parents = get_num_parents();
  if (num_parents == 0) {
    // No parents; no ambiguity.  This is the root.
    return get_top_component(this, true);
  } 

  PT(NodePathComponent) result;
  if (num_parents == 1) {
    // Only one parent; no ambiguity.
    PT(NodePathComponent) parent = 
      get_parent(0)->r_get_generic_component(accept_ambiguity, ambiguity_detected);
    result = get_component(parent, this);

  } else {
    // Oops, multiple parents; the NodePath is ambiguous.
    if (!accept_ambiguity) {
      pgraph_cat.warning()
        << *this << " has " << num_parents
        << " parents; choosing arbitrary path to root.\n";
    }
    ambiguity_detected = true;
    PT(NodePathComponent) parent = 
      get_parent(0)->r_get_generic_component(accept_ambiguity, ambiguity_detected);
    result = get_component(parent, this);
  }

  return result;
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
  // We have to remove the component from all of the pipeline stages,
  // not just the current one.
  int max_num_erased = 0;

  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      int num_erased = cdata->_paths.erase(component);
      max_num_erased = max(max_num_erased, num_erased);
      _cycler.release_write_stage(i, cdata);
    }
  }

  nassertv(max_num_erased == 1);
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
////////////////////////////////////////////////////////////////////
void PandaNode::
sever_connection(PandaNode *parent_node, PandaNode *child_node) {
  CDWriter cdata_child(child_node->_cycler);

  Paths::iterator pi;
  for (pi = cdata_child->_paths.begin();
       pi != cdata_child->_paths.end();
       ++pi) {
    if (!(*pi)->is_top_node() && 
        (*pi)->get_next()->get_node() == parent_node) {
      // Sever the component here.
      (*pi)->set_top_node();
    }
  }
  child_node->fix_path_lengths(cdata_child);
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
////////////////////////////////////////////////////////////////////
void PandaNode::
new_connection(PandaNode *parent_node, PandaNode *child_node) {
  CDWriter cdata_child(child_node->_cycler);

  Paths::iterator pi;
  for (pi = cdata_child->_paths.begin();
       pi != cdata_child->_paths.end();
       ++pi) {
    if ((*pi)->is_top_node()) {
      (*pi)->set_next(parent_node->get_generic_component(false));
    }
  }
  child_node->fix_path_lengths(cdata_child);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fix_path_lengths
//       Access: Private
//  Description: Recursively fixes the _length member of each
//               NodePathComponent at this level and below, after an
//               add or delete child operation that might have messed
//               these up.
////////////////////////////////////////////////////////////////////
void PandaNode::
fix_path_lengths(const CData *cdata) {
  bool any_wrong = false;

  Paths::const_iterator pi;
  for (pi = cdata->_paths.begin(); pi != cdata->_paths.end(); ++pi) {
    if ((*pi)->fix_length()) {
      any_wrong = true;
    }
  }
  
  // If any paths were updated, we have to recurse on all of our
  // children, since any one of those paths might be shared by any of
  // our child nodes.
  if (any_wrong) {
    Down::const_iterator di;
    for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
      PandaNode *child_node = (*di).get_child();
      CDReader cdata_child(child_node->_cycler);
      child_node->fix_path_lengths(cdata_child);
    }
    for (di = cdata->_stashed.begin(); di != cdata->_stashed.end(); ++di) {
      PandaNode *child_node = (*di).get_child();
      CDReader cdata_child(child_node->_cycler);
      child_node->fix_path_lengths(cdata_child);
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
  CDReader cdata(_cycler);
  indent(out, indent_level) << *this;
  if (!cdata->_tag_data.empty()) {
    out << " [";
    list_tags(out, " ");
    out << "]";
  }
  if (!cdata->_transform->is_identity()) {
    out << " " << *cdata->_transform;
  }
  if (!cdata->_state->is_empty()) {
    out << " " << *cdata->_state;
  }
  if (!cdata->_effects->is_empty()) {
    out << " " << *cdata->_effects;
  }
  out << "\n";

  Down::const_iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    (*di).get_child()->r_list_descendants(out, indent_level + 2);
  }

  // Also report the number of stashed nodes at this level.
  int num_stashed = get_num_stashed();
  if (num_stashed != 0) {
    indent(out, indent_level) << "(" << num_stashed << " stashed)\n";
  }
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
