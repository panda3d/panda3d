// Filename: animChannelScalarDynamic.cxx
// Created by:  drose (20Oct03)
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

#include "animChannelScalarDynamic.h"
#include "animBundle.h"
#include "config_chan.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle AnimChannelScalarDynamic::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimChannelScalarDynamic::
AnimChannelScalarDynamic(AnimGroup *parent, const string &name)
  : AnimChannelScalar(parent, name) 
{
  _last_value = _value = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::Constructor
//       Access: Protected
//  Description: For use only with the bam reader.
////////////////////////////////////////////////////////////////////
AnimChannelScalarDynamic::
AnimChannelScalarDynamic() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::has_changed
//       Access: Public, Virtual
//  Description: Returns true if the value has changed since the last
//               call to has_changed().  last_frame is the frame
//               number of the last call; this_frame is the current
//               frame number.
////////////////////////////////////////////////////////////////////
bool AnimChannelScalarDynamic::
has_changed(int, int) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
    bool has_changed = (_value != _last_value);
    _last_value = _value;
    return has_changed;

  } else {
    bool has_changed = _value_changed;
    _value_changed = false;
    return has_changed;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::get_value
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
get_value(int, float &value) {
  if (_value_node != (PandaNode *)NULL) {
    value = _value->get_pos()[0];

  } else {
    value = _float_value;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::set_value
//       Access: Published
//  Description: Explicitly sets the value.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
set_value(float value) {
  _float_value = value;
  _value_node.clear();
  _value_changed = true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::set_value_node
//       Access: Published
//  Description: Specifies a node whose transform will be queried each
//               frame to implicitly specify the transform of this
//               joint.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
set_value_node(PandaNode *value_node) {
  if (_value_node == (PandaNode *)NULL) {
    _last_value = TransformState::make_pos(LVecBase3f(_float_value, 0.0f, 0.0f));
  }

  _value_node = value_node;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
write_datagram(BamWriter *manager, Datagram &dg) {
  AnimChannelScalar::write_datagram(manager, dg);
  manager->write_pointer(dg, _value_node);
  manager->write_pointer(dg, _value);
  dg.add_float32(_float_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int AnimChannelScalarDynamic::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = AnimChannelScalar::complete_pointers(p_list, manager);

  // Get the _value_node and _value pointers.
  _value_node = DCAST(PandaNode, p_list[pi++]);
  _value = DCAST(TransformState, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::fillin
//       Access: Public
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannelScalar::fillin(scan, manager);

  // Read the _value_node and _value pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);

  _float_value = scan.get_float32();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::make_AnimChannelScalarDynamic
//       Access: Public
//  Description: Factory method to generate a AnimChannelScalarDynamic object
////////////////////////////////////////////////////////////////////
TypedWritable *AnimChannelScalarDynamic::
make_AnimChannelScalarDynamic(const FactoryParams &params) {
  AnimChannelScalarDynamic *me = new AnimChannelScalarDynamic;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarDynamic::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimChannelScalarDynamic object
////////////////////////////////////////////////////////////////////
void AnimChannelScalarDynamic::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelScalarDynamic);
}




