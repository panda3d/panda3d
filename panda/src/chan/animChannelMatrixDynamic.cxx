// Filename: animChannelMatrixDynamic.cxx
// Created by:  drose (20Oct03)
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

#include "animChannelMatrixDynamic.h"
#include "animBundle.h"
#include "config_chan.h"

#include "compose_matrix.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle AnimChannelMatrixDynamic::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::Constructor
//       Access: Protected
//  Description: For use only with the bam reader.
/////////////////////////////////////////////////////////////
AnimChannelMatrixDynamic::
AnimChannelMatrixDynamic() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannelMatrixDynamic, just like
//               this one, without copying any children.  The new copy
//               is added to the indicated parent.  Intended to be
//               called by make_copy() only.
////////////////////////////////////////////////////////////////////
AnimChannelMatrixDynamic::
AnimChannelMatrixDynamic(AnimGroup *parent, const AnimChannelMatrixDynamic &copy) : 
  AnimChannelMatrix(parent, copy),
  _value_node(copy._value_node),
  _value(copy._value),
  _last_value(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimChannelMatrixDynamic::
AnimChannelMatrixDynamic(const string &name)
  : AnimChannelMatrix(name) 
{
  _value = TransformState::make_identity();
  _last_value = NULL;  // This is impossible; thus, has_changed() will
                       // always return true the first time.
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
has_changed(int, double, int, double) {
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
get_value(int, LMatrix4 &mat) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  mat = _value->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_value_no_scale_shear
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame,
//               without any scale or shear information.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_value_no_scale_shear(int, LMatrix4 &mat) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  if (_value->has_scale() || _value->has_shear()) {
    compose_matrix(mat, LVecBase3(1.0f, 1.0f, 1.0f),
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
get_scale(int, LVecBase3 &scale) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  scale = _value->get_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_hpr
//       Access: Public, Virtual
//  Description: Returns the h, p, and r components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_hpr(int, LVecBase3 &hpr) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  hpr = _value->get_hpr();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_quat
//       Access: Public, Virtual
//  Description: Returns the rotation component associated with the
//               current frame, expressed as a quaternion.  As above,
//               this only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_quat(int, LQuaternion &quat) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  quat = _value->get_quat();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_pos
//       Access: Public, Virtual
//  Description: Returns the x, y, and z translation components
//               associated with the current frame.  As above, this
//               only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_pos(int, LVecBase3 &pos) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  pos = _value->get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_shear
//       Access: Public, Virtual
//  Description: Returns the a, b, and c shear components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
get_shear(int, LVecBase3 &shear) {
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
  shear = _value->get_shear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::set_value
//       Access: Published
//  Description: Explicitly sets the matrix value.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixDynamic::
set_value(const LMatrix4 &value) {
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
  if (_value_node != (PandaNode *)NULL) {
    _value = _value_node->get_transform();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::make_copy
//       Access: Protected, Virtual
//  Description: Returns a copy of this object, and attaches it to the
//               indicated parent (which may be NULL only if this is
//               an AnimBundle).  Intended to be called by
//               copy_subtree() only.
////////////////////////////////////////////////////////////////////
AnimGroup *AnimChannelMatrixDynamic::
make_copy(AnimGroup *parent) const {
  return new AnimChannelMatrixDynamic(parent, *this);
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


