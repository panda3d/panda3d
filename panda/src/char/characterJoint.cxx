// Filename: characterJoint.cxx
// Created by:  drose (23Feb99)
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

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "char_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "characterJoint.h"
#include "config_char.h"

#include <compose_matrix.h>
#include <transformTransition.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#endif

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
  // We don't copy the sets of transform arcs.
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
//  Description:
////////////////////////////////////////////////////////////////////
void CharacterJoint::
update_internals(PartGroup *parent, bool self_changed, bool) {
  nassertv(parent != NULL);

  bool net_changed = false;
  if (parent->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *parent_joint = DCAST(CharacterJoint, parent);

    _net_transform = _value * parent_joint->_net_transform;
    net_changed = true;

  } else if (self_changed) {
    _net_transform = _value;
    net_changed = true;
  }

  if (net_changed && !_net_transform_arcs.empty()) {
    PT(TransformTransition) t = new TransformTransition(_net_transform);

    ArcList::iterator ai;
    ai = _net_transform_arcs.begin();
    while (ai != _net_transform_arcs.end()) {
      NodeRelation *arc = *ai;
      if (arc->is_attached()) {
        arc->set_transition(t);
        ++ai;
      } else {
        // The arc is now invalid; its geometry must have been
        // removed.  Remove the arc from our set.
        ArcList::iterator invalid = ai;
        ++ai;
        _net_transform_arcs.erase(invalid);
      }
    }
  }

  if (self_changed && !_local_transform_arcs.empty()) {
    PT(TransformTransition) t = new TransformTransition(_value);

    ArcList::iterator ai;
    ai = _local_transform_arcs.begin();
    while (ai != _local_transform_arcs.end()) {
      NodeRelation *arc = *ai;
      if (arc->is_attached()) {
        arc->set_transition(t);
        ++ai;
      } else {
        // The arc is now invalid; its geometry must have been
        // removed.  Remove the arc from our set.
        ArcList::iterator invalid = ai;
        ++ai;
        _local_transform_arcs.erase(invalid);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::add_net_transform
//       Access: Public
//  Description: Adds the indicated arc to the list of arcs that will
//               be updated each frame with the joint's net transform
//               from the root.  Returns true if the arc is
//               successfully added, false if it had already been
//               added.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
add_net_transform(NodeRelation *arc) {
  return _net_transform_arcs.insert(arc).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::remove_net_transform
//       Access: Public
//  Description: Removes the indicated arc from the list of arcs that
//               will be updated each frame with the joint's net
//               transform from the root.  Returns true if the arc is
//               successfully removed, false if it was not on the
//               list.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
remove_net_transform(NodeRelation *arc) {
  return (_net_transform_arcs.erase(arc) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::has_net_transform
//       Access: Public
//  Description: Returns true if the arc is on the list of arcs that
//               will be updated each frame with the joint's net
//               transform from the root, false otherwise.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
has_net_transform(NodeRelation *arc) const {
  return (_net_transform_arcs.count(arc) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::clear_net_transforms
//       Access: Public
//  Description: Removes all arcs from the list of arcs that will be
//               updated each frame with the joint's net transform
//               from the root.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
clear_net_transforms() {
  _net_transform_arcs.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::add_local_transform
//       Access: Public
//  Description: Adds the indicated arc to the list of arcs that will
//               be updated each frame with the joint's local
//               transform from its parent.  Returns true if the arc
//               is successfully added, false if it had already been
//               added.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
add_local_transform(NodeRelation *arc) {
  return _local_transform_arcs.insert(arc).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::remove_local_transform
//       Access: Public
//  Description: Removes the indicated arc from the list of arcs that
//               will be updated each frame with the joint's local
//               transform from its parent.  Returns true if the arc
//               is successfully removed, false if it was not on the
//               list.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
remove_local_transform(NodeRelation *arc) {
  return (_local_transform_arcs.erase(arc) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::has_local_transform
//       Access: Public
//  Description: Returns true if the arc is on the list of arcs that
//               will be updated each frame with the joint's local
//               transform from its parent, false otherwise.
////////////////////////////////////////////////////////////////////
bool CharacterJoint::
has_local_transform(NodeRelation *arc) const {
  return (_local_transform_arcs.count(arc) > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJoint::clear_local_transforms
//       Access: Public
//  Description: Removes all arcs from the list of arcs that will be
//               updated each frame with the joint's local transform
//               from its parent.
////////////////////////////////////////////////////////////////////
void CharacterJoint::
clear_local_transforms() {
  _local_transform_arcs.clear();
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
  ArcList::iterator ai;

  // First, make sure all of our arcs are still valid, before we try
  // to write them out.  Remove any invalid arcs.
  ai = _net_transform_arcs.begin();
  while (ai != _net_transform_arcs.end()) {
    NodeRelation *arc = *ai;
    if (arc->is_attached()) {
      ++ai;
    } else {
      // The arc is now invalid; its geometry must have been
      // removed.  Remove the arc from our set.
      ArcList::iterator invalid = ai;
      ++ai;
      _net_transform_arcs.erase(invalid);
    }
  }
  ai = _local_transform_arcs.begin();
  while (ai != _local_transform_arcs.end()) {
    NodeRelation *arc = *ai;
    if (arc->is_attached()) {
      ++ai;
    } else {
      // The arc is now invalid; its geometry must have been
      // removed.  Remove the arc from our set.
      ArcList::iterator invalid = ai;
      ++ai;
      _local_transform_arcs.erase(invalid);
    }
  }

  MovingPartMatrix::write_datagram(manager, me);
  me.add_uint16(_net_transform_arcs.size());

  for(ai = _net_transform_arcs.begin(); ai != _net_transform_arcs.end(); ai++) {
    manager->write_pointer(me, (*ai));
  }

  me.add_uint16(_local_transform_arcs.size());
  for(ai = _local_transform_arcs.begin(); ai != _local_transform_arcs.end(); ai++)
  {
    manager->write_pointer(me, (*ai));
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
fillin(DatagramIterator& scan, BamReader* manager)
{
  int i;
  MovingPartMatrix::fillin(scan, manager);
  _num_net_arcs = scan.get_uint16();
  for(i = 0; i < _num_net_arcs; i++)
  {
    manager->read_pointer(scan, this);
  }

  _num_local_arcs = scan.get_uint16();
  for(i = 0; i < _num_local_arcs; i++)
  {
    manager->read_pointer(scan, this);
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
complete_pointers(vector_typedWritable &p_list, BamReader* manager)
{
  int i;
  int start = MovingPartMatrix::complete_pointers(p_list, manager);
  int mid = start+_num_net_arcs;
  int end = start+_num_net_arcs+_num_local_arcs;

  for(i = start; i < mid; i++)
  {
    if (p_list[i] == TypedWritable::Null)
    {
      char_cat->warning() << get_name()
                          << " Ignoring null Net NodeRelation" << endl;
    }
    else
    {
      add_net_transform(DCAST(NodeRelation, p_list[i]));
    }
  }

  for(i = mid; i < end; i++)
  {
    if (p_list[i] == TypedWritable::Null)
    {
      char_cat->warning() << get_name()
                          << " Ignoring null Local NodeRelation" << endl;
    }
    else
    {
      add_local_transform(DCAST(NodeRelation, p_list[i]));
    }
  }

  return end;
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
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJoint);
}


