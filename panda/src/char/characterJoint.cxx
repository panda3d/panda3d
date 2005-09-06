// Filename: characterJoint.cxx
// Created by:  drose (23Feb99)
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

#include "characterJoint.h"
#include "config_char.h"
#include "jointVertexTransform.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CharacterJoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
CharacterJoint::
CharacterJoint() {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
CharacterJoint::
CharacterJoint(const CharacterJoint &copy) :
  MovingPartMatrix(copy),
  _net_transform(copy._net_transform),
  _initial_net_transform_inverse(copy._initial_net_transform_inverse)
{
  // We don't copy the sets of transform nodes.
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CharacterJoint::
CharacterJoint(PartGroup *parent, const string &name,
               const LMatrix4f &initial_value)
  : MovingPartMatrix(parent, name, initial_value)
{
  // Now that we've constructed and we're in the tree, let's call
  // update_internals() to get our _net_transform set properly.
  update_internals(parent, true, false);

  // And then compute its inverse.  This is needed for
  // ComputedVertices, during animation.
  _initial_net_transform_inverse = invert(_net_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CharacterJoint::
~CharacterJoint() {
  nassertv(_vertex_transforms.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *CharacterJoint::
make_copy() const {
  return new CharacterJoint(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::update_internals
//       Access: Public, Virtual
//  Description: This is called by do_update() whenever the part or
//               some ancestor has changed values.  It is a hook for
//               derived classes to update whatever cache they may
//               have that depends on these.
//
//               The return value is true if the part has changed as a
//               result of the update, or false otherwise.
//
//               In the case of a CharacterJoint, of course, it means
//               to recompute the joint angles and associated
//               transforms for this particular joint.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
update_internals(PartGroup *parent, bool self_changed, bool parent_changed) {
  nassertr(parent != (PartGroup *)NULL, false);

  bool net_changed = false;
  if (parent->is_of_type(CharacterJoint::get_class_type())) {
    // The joint is not a toplevel joint; its parent therefore affects
    // its net transform.
    if (parent_changed || self_changed) {
      CharacterJoint *parent_joint = DCAST(CharacterJoint, parent);
      
      _net_transform = _value * parent_joint->_net_transform;
      net_changed = true;
    }

  } else {
    // The joint *is* a toplevel joint, and the only thing that
    // affects its net transform is the joint itself.
    if (self_changed) {
      _net_transform = _value;
      net_changed = true;
    }
  }

  if (net_changed) {
    if (!_net_transform_nodes.empty()) {
      CPT(TransformState) t = TransformState::make_mat(_net_transform);
      
      NodeList::iterator ai;
      ai = _net_transform_nodes.begin();
      while (ai != _net_transform_nodes.end()) {
        PandaNode *node = *ai;
        node->set_transform(t);
        ++ai;
      }
    }

    // Also tell our related JointVertexTransforms that they now need
    // to recompute themselves.
    VertexTransforms::iterator vti;
    for (vti = _vertex_transforms.begin(); vti != _vertex_transforms.end(); ++vti) {
      (*vti)->_matrix_stale = true;
      (*vti)->mark_modified();
    }
  }

  if (self_changed && !_local_transform_nodes.empty()) {
    CPT(TransformState) t = TransformState::make_mat(_value);

    NodeList::iterator ai;
    ai = _local_transform_nodes.begin();
    while (ai != _local_transform_nodes.end()) {
      PandaNode *node = *ai;
      node->set_transform(t);
      ++ai;
    }
  }

  return self_changed || net_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::add_net_transform
//       Access: Published
//  Description: Adds the indicated node to the list of nodes that will
//               be updated each frame with the joint's net transform
//               from the root.  Returns true if the node is
//               successfully added, false if it had already been
//               added.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
add_net_transform(PandaNode *node) {
  return _net_transform_nodes.insert(node).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::remove_net_transform
//       Access: Published
//  Description: Removes the indicated node from the list of nodes that
//               will be updated each frame with the joint's net
//               transform from the root.  Returns true if the node is
//               successfully removed, false if it was not on the
//               list.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
remove_net_transform(PandaNode *node) {
  return (_net_transform_nodes.erase(node) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::has_net_transform
//       Access: Published
//  Description: Returns true if the node is on the list of nodes that
//               will be updated each frame with the joint's net
//               transform from the root, false otherwise.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
has_net_transform(PandaNode *node) const {
  return (_net_transform_nodes.count(node) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::clear_net_transforms
//       Access: Published
//  Description: Removes all nodes from the list of nodes that will be
//               updated each frame with the joint's net transform
//               from the root.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
clear_net_transforms() {
  _net_transform_nodes.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::add_local_transform
//       Access: Published
//  Description: Adds the indicated node to the list of nodes that will
//               be updated each frame with the joint's local
//               transform from its parent.  Returns true if the node
//               is successfully added, false if it had already been
//               added.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
add_local_transform(PandaNode *node) {
  return _local_transform_nodes.insert(node).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::remove_local_transform
//       Access: Published
//  Description: Removes the indicated node from the list of nodes that
//               will be updated each frame with the joint's local
//               transform from its parent.  Returns true if the node
//               is successfully removed, false if it was not on the
//               list.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
remove_local_transform(PandaNode *node) {
  return (_local_transform_nodes.erase(node) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::has_local_transform
//       Access: Published
//  Description: Returns true if the node is on the list of nodes that
//               will be updated each frame with the joint's local
//               transform from its parent, false otherwise.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
has_local_transform(PandaNode *node) const {
  return (_local_transform_nodes.count(node) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::clear_local_transforms
//       Access: Published
//  Description: Removes all nodes from the list of nodes that will be
//               updated each frame with the joint's local transform
//               from its parent.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
clear_local_transforms() {
  _local_transform_nodes.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::get_transform
//       Access: Published
//  Description: Copies the joint's current transform into the
//               indicated matrix.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
get_transform(LMatrix4f &transform) const {
  transform = _value;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::get_net_transform
//       Access: Published
//  Description: Copies the joint's current net transform (composed
//               from the root of the character joint hierarchy) into
//               the indicated matrix.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
get_net_transform(LMatrix4f &transform) const {
  transform = _net_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CharacterJoint::
write_datagram(BamWriter *manager, Datagram &me)
{
  NodeList::iterator ni;
  MovingPartMatrix::write_datagram(manager, me);

  me.add_uint16(_net_transform_nodes.size());
  for(ni = _net_transform_nodes.begin(); 
      ni != _net_transform_nodes.end(); 
      ni++) {
    manager->write_pointer(me, (*ni));
  }

  me.add_uint16(_local_transform_nodes.size());
  for(ni = _local_transform_nodes.begin(); 
      ni != _local_transform_nodes.end(); 
      ni++) {
    manager->write_pointer(me, (*ni));
  }

  _initial_net_transform_inverse.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CharacterJoint::
fillin(DatagramIterator &scan, BamReader *manager) {
  int i;
  MovingPartMatrix::fillin(scan, manager);

  _num_net_nodes = scan.get_uint16();
  for(i = 0; i < _num_net_nodes; i++) {
    manager->read_pointer(scan);
  }
  
  _num_local_nodes = scan.get_uint16();
  for(i = 0; i < _num_local_nodes; i++) {
    manager->read_pointer(scan);
  }

  _initial_net_transform_inverse.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int CharacterJoint::
complete_pointers(TypedWritable **p_list, BamReader* manager)
{
  int pi = MovingPartMatrix::complete_pointers(p_list, manager);

  int i;
  for (i = 0; i < _num_net_nodes; i++) {
    add_net_transform(DCAST(PandaNode, p_list[pi++]));
  }

  for (i = 0; i < _num_local_nodes; i++) {
    add_local_transform(DCAST(PandaNode, p_list[pi++]));
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::make_CharacterJoint
//       Access: Protected
//  Description: Factory method to generate a CharacterJoint object
////////////////////////////////////////////////////////////////////
TypedWritable* CharacterJoint::
make_CharacterJoint(const FactoryParams &params)
{
  CharacterJoint *me = new CharacterJoint;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CharacterJoint object
////////////////////////////////////////////////////////////////////
void CharacterJoint::
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJoint);
}


