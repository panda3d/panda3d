// Filename: pandaNode.cxx
// Created by:  drose (20Feb02)
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

#include "pandaNode.h"
#include "config_pgraph.h"
#include "nodeChainComponent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"


TypeHandle PandaNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PandaNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::CData::
CData(const PandaNode::CData &copy) :
  _down(copy._down),
  _up(copy._up),
  _chains(copy._chains),
  _node_bounds(copy._node_bounds),
  _subgraph_bounds(copy._subgraph_bounds),
  _state(copy._state),
  _transform(copy._transform)
{
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
//     Function: PandaNode::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode::
PandaNode(const PandaNode &copy) :
  TypedWritable(copy),
  Namable(copy),
  ReferenceCount(copy)
{
  // Copying a node does not copy its children.

  // Copy the other node's state and bounding volume.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_state = copy_cdata->_state;
  cdata->_transform = copy_cdata->_transform;
  cdata->_node_bounds = copy_cdata->_node_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PandaNode::
operator = (const PandaNode &copy) {
  TypedWritable::operator = (copy);
  Namable::operator = (copy);
  ReferenceCount::operator = (copy);

  // Copy the other node's state and bounding volume.
  CDReader copy_cdata(copy._cycler);
  CDWriter cdata(_cycler);
  cdata->_state = copy_cdata->_state;
  cdata->_transform = copy_cdata->_transform;
  cdata->_node_bounds = copy_cdata->_node_bounds;
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
  CDReader cdata(_cycler);
  nassertv(cdata->_up.empty());

  remove_all_children();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::find_child
//       Access: Published
//  Description: Returns the index of the indicated child node, if it
//               is a child, or -1 if it is not.
////////////////////////////////////////////////////////////////////
int PandaNode::
find_child(PandaNode *node) const {
  CDReader cdata(_cycler);

  // We have to search for the child by brute force, since we don't
  // know what sort index it was added as.
  Down::const_iterator ci;
  for (ci = cdata->_down.begin(); ci != cdata->_down.end(); ++ci) {
    if ((*ci).get_child() == node) {
      return ci - cdata->_down.begin();
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
  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  remove_child(child_node);
  CDWriter cdata(_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata->_down.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(this));

  // We also have to adjust any NodeChainComponents the child might
  // have that reference the child as a top node.  Any other
  // components we can leave alone, because we are making a new
  // instance of the child.
  Chains::iterator ci;
  for (ci = cdata_child->_chains.begin();
       ci != cdata_child->_chains.end();
       ++ci) {
    if ((*ci)->is_top_node()) {
      (*ci)->set_next(get_generic_component());
    }
  }

  child_node->fix_chain_lengths();
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

  // Now sever any NodeChainComponents on the child that reference
  // this node.  If we have multiple of these, we have to collapse
  // them together.
  NodeChainComponent *collapsed = (NodeChainComponent *)NULL;
  Chains::iterator ci;
  ci = cdata_child->_chains.begin();
  while (ci != cdata_child->_chains.end()) {
    Chains::iterator cnext = ci;
    ++cnext;
    if (!(*ci)->is_top_node() && (*ci)->get_next()->get_node() == this) {
      if (collapsed == (NodeChainComponent *)NULL) {
        (*ci)->set_top_node();
        collapsed = (*ci);
      } else {
        // This is a different component that used to reference a
        // different instance, but now it's all just the same topnode.
        // We have to collapse this and the previous one together.
        // However, there might be some NodeChains out there that
        // still keep a pointer to this one, so we can't remove it
        // altogether.
        (*ci)->collapse_with(collapsed);
        cdata_child->_chains.erase(ci);
      }
    }
    ci = cnext;
  }

  child_node->fix_chain_lengths();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_child
//       Access: Published
//  Description: Removes the indicated child from the node.  Returns
//               true if the child was removed, false if it was not
//               already a child of the node.
////////////////////////////////////////////////////////////////////
bool PandaNode::
remove_child(PandaNode *child_node) {
  // Ensure the child_node is not deleted while we do this.
  PT(PandaNode) keep_child = child_node;
  CDWriter cdata_child(child_node->_cycler);

  // First, look for and remove this node from the child's parent
  // list.
  int num_erased = cdata_child->_up.erase(UpConnection(this));
  if (num_erased == 0) {
    // No such node; it wasn't our child to begin with.
    return false;
  }

  // Now sever any NodeChainComponents on the child that reference
  // this node.  If we have multiple of these, we have to collapse
  // them together (see above).
  NodeChainComponent *collapsed = (NodeChainComponent *)NULL;
  Chains::iterator ci;
  ci = cdata_child->_chains.begin();
  while (ci != cdata_child->_chains.end()) {
    Chains::iterator cnext = ci;
    ++cnext;
    if (!(*ci)->is_top_node() && (*ci)->get_next()->get_node() == this) {
      if (collapsed == (NodeChainComponent *)NULL) {
        (*ci)->set_top_node();
        collapsed = (*ci);
      } else {
        (*ci)->collapse_with(collapsed);
        cdata_child->_chains.erase(ci);
      }
    }
    ci = cnext;
  }

  child_node->fix_chain_lengths();

  CDWriter cdata(_cycler);

  // Now, look for and remove the child node from our down list.
  Down::iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    if ((*di).get_child() == child_node) {
      cdata->_down.erase(di);
      return true;
    }
  }

  // We shouldn't get here unless there was a parent-child mismatch.
  nassertr(false, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::remove_all_children
//       Access: Published
//  Description: Removes all the children from the node at once.
////////////////////////////////////////////////////////////////////
void PandaNode::
remove_all_children() {
  CDWriter cdata(_cycler);
  Down::iterator ci;
  for (ci = cdata->_down.begin(); ci != cdata->_down.end(); ++ci) {
    PT(PandaNode) child_node = (*ci).get_child();
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_up.erase(UpConnection(this));

    // Now sever any NodeChainComponents on the child that reference
    // this node.  If we have multiple of these, we have to collapse
    // them together (see above).
    NodeChainComponent *collapsed = (NodeChainComponent *)NULL;
    Chains::iterator ci;
    ci = cdata_child->_chains.begin();
    while (ci != cdata_child->_chains.end()) {
      Chains::iterator cnext = ci;
      ++cnext;
      if (!(*ci)->is_top_node() && (*ci)->get_next()->get_node() == this) {
        if (collapsed == (NodeChainComponent *)NULL) {
          (*ci)->set_top_node();
          collapsed = (*ci);
        } else {
          (*ci)->collapse_with(collapsed);
          cdata_child->_chains.erase(ci);
        }
      }
      ci = cnext;
    }

    child_node->fix_chain_lengths();
  }
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
  if (!cdata->_transform->is_identity()) {
    out << " T";
  }
  if (!cdata->_state->is_empty()) {
    out << " " << *cdata->_state;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::is_geom_node
//       Access: Public, Virtual
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
//     Function: PandaNode::attach
//       Access: Private, Static
//  Description: Creates a new parent-child relationship, and returns
//               the new NodeChainComponent.  If the child was already
//               attached to the indicated parent, repositions it and
//               returns the original NodeChainComponent.
////////////////////////////////////////////////////////////////////
PT(NodeChainComponent) PandaNode::
attach(NodeChainComponent *parent, PandaNode *child_node, int sort) {
  nassertr(parent != (NodeChainComponent *)NULL, (NodeChainComponent *)NULL);

  // See if the child was already attached to the parent.  If it was,
  // we'll use that same NodeChainComponent.
  PT(NodeChainComponent) child = get_component(parent, child_node);

  if (child == (NodeChainComponent *)NULL) {
    // The child was not already attached to the parent, so get a new
    // component.
    child = get_top_component(child_node);
  }

  reparent(parent, child, sort);
  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::detach
//       Access: Private, Static
//  Description: Breaks a parent-child relationship.
////////////////////////////////////////////////////////////////////
void PandaNode::
detach(NodeChainComponent *child) {
  nassertv(child != (NodeChainComponent *)NULL);
  nassertv(!child->is_top_node());
  PandaNode *child_node = child->get_node();
  PandaNode *parent_node = child->get_next()->get_node();

  // Break the NodeChainComponent connection.
  child->set_top_node();

  CDWriter cdata_child(child_node->_cycler);

  // Any other components in the same child_node that previously
  // referenced the same parent has now become invalid and must be
  // collapsed into this one and removed from the chains set.
  Chains::iterator ci;
  ci = cdata_child->_chains.begin();
  while (ci != cdata_child->_chains.end()) {
    Chains::iterator cnext = ci;
    ++cnext;
    if ((*ci) != child && !(*ci)->is_top_node() && 
        (*ci)->get_next()->get_node() == parent_node) {
      (*ci)->collapse_with(child);
      cdata_child->_chains.erase(ci);
    }
    ci = cnext;
  }

  child_node->fix_chain_lengths();

  // Now look for the child and break the actual connection.

  // First, look for and remove the parent node from the child's up
  // list.
  int num_erased = cdata_child->_up.erase(UpConnection(parent_node));
  nassertv(num_erased == 1);

  CDWriter cdata_parent(parent_node->_cycler);

  // Now, look for and remove the child node from the parent's down list.
  Down::iterator di;
  for (di = cdata_parent->_down.begin(); 
       di != cdata_parent->_down.end(); 
       ++di) {
    if ((*di).get_child() == child_node) {
      cdata_parent->_down.erase(di);
      return;
    }
  }

  // We shouldn't get here unless there was a parent-child mismatch.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::reparent
//       Access: Private, Static
//  Description: Switches a node from one parent to another.
////////////////////////////////////////////////////////////////////
void PandaNode::
reparent(NodeChainComponent *new_parent, NodeChainComponent *child, int sort) {
  nassertv(new_parent != (NodeChainComponent *)NULL);
  nassertv(child != (NodeChainComponent *)NULL);

  if (!child->is_top_node()) {
    detach(child);
  }

  // Adjust the NodeChainComponents.
  child->set_next(new_parent);

  PandaNode *child_node = child->get_node();
  PandaNode *parent_node = new_parent->get_node();

  // Now reattach at the indicated sort position.
  CDWriter cdata_parent(parent_node->_cycler);
  CDWriter cdata_child(child_node->_cycler);
  
  cdata_parent->_down.insert(DownConnection(child_node, sort));
  cdata_child->_up.insert(UpConnection(parent_node));

  cdata_child->_chains.insert(child);
  child_node->fix_chain_lengths();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_component
//       Access: Private, Static
//  Description: Returns the NodeChainComponent based on the indicated
//               child of the given parent, or NULL if there is no
//               such parent-child relationship.
////////////////////////////////////////////////////////////////////
PT(NodeChainComponent) PandaNode::
get_component(NodeChainComponent *parent, PandaNode *child_node) {
  nassertr(parent != (NodeChainComponent *)NULL, (NodeChainComponent *)NULL);
  PandaNode *parent_node = parent->get_node();

  {
    CDReader cdata_child(child_node->_cycler);

    // First, walk through the list of NodeChainComponents we already
    // have on the child, looking for one that already exists,
    // referencing the indicated parent component.
    Chains::const_iterator ci;
    for (ci = cdata_child->_chains.begin(); 
         ci != cdata_child->_chains.end(); 
         ++ci) {
      if ((*ci)->get_next() == parent) {
        // If we already have such a component, just return it.
        return (*ci);
      }
    }
  }
    
  // We don't already have a NodeChainComponent referring to this
  // parent-child relationship.  Are they actually related?
  int child_index = child_node->find_parent(parent_node);
  if (child_index >= 0) {
    // They are.  Create and return a new one.
    PT(NodeChainComponent) child = 
      new NodeChainComponent(child_node, parent);
    CDWriter cdata_child(child_node->_cycler);
    cdata_child->_chains.insert(child);
    return child;
  } else {
    // They aren't related.  Return NULL.
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_top_component
//       Access: Private, Static
//  Description: Returns a NodeChainComponent referencing the
//               indicated node as a singleton.  It is invalid to call
//               this for a node that has parents, unless you are
//               about to create a new instance (and immediately
//               reconnect the NodeChainComponent elsewhere).
////////////////////////////////////////////////////////////////////
PT(NodeChainComponent) PandaNode::
get_top_component(PandaNode *child_node) {
  {
    CDReader cdata_child(child_node->_cycler);

    // Walk through the list of NodeChainComponents we already have on
    // the child, looking for one that already exists as a top node.
    Chains::const_iterator ci;
    for (ci = cdata_child->_chains.begin(); 
         ci != cdata_child->_chains.end(); 
         ++ci) {
      if ((*ci)->is_top_node()) {
        // If we already have such a component, just return it.
        return (*ci);
      }
    }
  }

  // We don't already have such a NodeChainComponent; create and
  // return a new one.
  PT(NodeChainComponent) child = 
    new NodeChainComponent(child_node, (NodeChainComponent *)NULL);
  CDWriter cdata_child(child_node->_cycler);
  cdata_child->_chains.insert(child);

  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::get_generic_component
//       Access: Private
//  Description: Returns a NodeChainComponent referencing this node as
//               a chain from the root.  It is only valid to call this
//               if there is an unambiguous path from the root;
//               otherwise, a warning will be issued and one path will
//               be chosen arbitrarily.
////////////////////////////////////////////////////////////////////
PT(NodeChainComponent) PandaNode::
get_generic_component() {
  int num_parents = get_num_parents();
  if (num_parents == 0) {
    return get_top_component(this);

  } else {
    if (num_parents != 1) {
      pgraph_cat.warning()
        << *this << " has " << num_parents
        << " parents; choosing arbitrary path to root.\n";
    }
    PT(NodeChainComponent) parent = get_parent(0)->get_generic_component();
    return get_component(parent, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::delete_component
//       Access: Private
//  Description: Removes a NodeChainComponent from the set prior to
//               its deletion.  This should only be called by the
//               NodeChainComponent destructor.
////////////////////////////////////////////////////////////////////
void PandaNode::
delete_component(NodeChainComponent *component) {
  // We have to remove the component from all of the pipeline stages,
  // not just the current one.
  int max_num_erased = 0;

  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      int num_erased = cdata->_chains.erase(component);
      max_num_erased = max(max_num_erased, num_erased);
      _cycler.release_write_stage(i, cdata);
    }
  }
  nassertv(max_num_erased == 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNode::fix_chain_lengths
//       Access: Private
//  Description: Recursively fixes the _length member of each
//               NodeChainComponent at this level and below, after an
//               add or delete child operation that might have messed
//               these up.
////////////////////////////////////////////////////////////////////
void PandaNode::
fix_chain_lengths() {
  CDReader cdata(_cycler);
  bool any_wrong = false;

  Chains::const_iterator ci;
  for (ci = cdata->_chains.begin(); ci != cdata->_chains.end(); ++ci) {
    if ((*ci)->fix_length()) {
      any_wrong = true;
    }
  }
  
  // If any chains were updated, we have to recurse on all of our
  // children, since any one of those chains might be shared by any of
  // our child nodes.
  if (any_wrong) {
    Down::const_iterator di;
    for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
      (*di).get_child()->fix_chain_lengths();
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

  CDReader cdata(_cycler);
  Down::const_iterator di;
  for (di = cdata->_down.begin(); di != cdata->_down.end(); ++di) {
    (*di).get_child()->r_list_descendants(out, indent_level + 2);
  }

  // Also report the number of stashed nodes at this level.
  /*
  int num_stashed = get_num_stashed();
  if (num_stashed != 0) {
    indent(out, indent_level) << "(" << num_stashed << " stashed)\n";
  }
  */
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
}
