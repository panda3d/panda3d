// Filename: animChannelMatrixDynamic.cxx
// Created by:  drose (20Oct03)
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

#include "animChannelMatrixDynamic.h"
#include "animBundle.h"
#include "config_chan.h"

#include "compose_matrix.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "fftCompressor.h"

TypeHandle AnimChannelMatrixDynamic::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimChannelMatrixDynamic::
AnimChannelMatrixDynamic(AnimGroup *parent, const string &name)
  : AnimChannelMatrix(parent, name) 
{
  _last_value = _value = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::Constructor
//       Access: Protected
//  Description: For use only with the bam reader.
/////////////////////////////////////////////////////////////
AnimChannelMatrixDynamic::
AnimChannelMatrixDynamic() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::has_changed
//       Access: Public, Virtual
//  Description: Returns true if the value has changed since the last
//               call to has_changed().  last_frame is the frame
//               number of the last call; this_frame is the current
//               frame number.
////////////////////////////////////////////////////////////////////
bool AnimChannelMatrixDynamic::
has_changed(int, int) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  bool has_changed = (_value != _last_value);
  _last_value = _value;
  return has_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_value
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_value(int, LMatrix4f &mat) {
  mat = _value->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_value_no_scale
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame,
//               without any scale information.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_value_no_scale(int frame, LMatrix4f &mat) {
  if (_value->has_scale()) {
    compose_matrix(mat, LVecBase3f(1.0f, 1.0f, 1.0f),
                   _value->get_hpr(), _value->get_pos());
  } else {
    mat = _value->get_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_scale
//       Access: Public, Virtual
//  Description: Gets the scale value at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_scale(int frame, float scale[3]) {
  const LVecBase3f &sc = _value->get_scale();
  scale[0] = sc[0];
  scale[1] = sc[1];
  scale[2] = sc[2]; 
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::set_value
//       Access: Published
//  Description: Explicitly sets the matrix value.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
set_value(const LMatrix4f &value) {
  _value = TransformState::make_mat(value);
  _value_node.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::set_value
//       Access: Published
//  Description: Explicitly sets the matrix value, using the indicated
//               TransformState object as a convenience.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
set_value(const TransformState *value) {
  _value = value;
  _value_node.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::set_value_node
//       Access: Published
//  Description: Specifies a node whose transform will be queried each
//               frame to implicitly specify the transform of this
//               joint.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
set_value_node(PandaNode *value_node) {
  _value_node = value_node;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
write_datagram(BamWriter *manager, Datagram &dg) {
  AnimChannelMatrix::write_datagram(manager, dg);
  manager->write_pointer(dg, _value_node);
  manager->write_pointer(dg, _value);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int AnimChannelMatrixDynamic::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = AnimChannelMatrix::complete_pointers(p_list, manager);

  // Get the _value_node and _value pointers.
  _value_node = DCAST(PandaNode, p_list[pi++]);
  _value = DCAST(TransformState, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::fillin
//       Access: Public
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannelMatrix::fillin(scan, manager);

  // Read the _value_node and _value pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::make_AnimChannelMatrixDynamic
//       Access: Public
//  Description: Factory method to generate an
//               AnimChannelMatrixDynamic object.
////////////////////////////////////////////////////////////////////
TypedWritable *AnimChannelMatrixDynamic::
make_AnimChannelMatrixDynamic(const FactoryParams &params) {
  AnimChannelMatrixDynamic *me = new AnimChannelMatrixDynamic;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate an
//               AnimChannelMatrixDynamic object.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelMatrixDynamic);
}


