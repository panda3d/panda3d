// Filename: animChannelMatrixFixed.cxx
// Created by:  drose (19Jan06)
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

#include "animChannelMatrixFixed.h"
#include "compose_matrix.h"

TypeHandle AnimChannelMatrixFixed::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannelMatrixFixed, just like this
//               one, without copying any children.  The new copy is
//               added to the indicated parent.  Intended to be called
//               by make_copy() only.
////////////////////////////////////////////////////////////////////
AnimChannelMatrixFixed::
AnimChannelMatrixFixed(AnimGroup *parent, const AnimChannelMatrixFixed &copy) : 
  AnimChannel<ACMatrixSwitchType>(parent, copy),
  _pos(copy._pos),
  _hpr(copy._hpr),
  _scale(copy._scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AnimChannelMatrixFixed::
AnimChannelMatrixFixed(const string &name, const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) :
  AnimChannel<ACMatrixSwitchType>(name),
  _pos(pos), _hpr(hpr), _scale(scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::has_changed
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool AnimChannelMatrixFixed::
has_changed(int, double, int, double) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::get_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_value(int, LMatrix4 &value) {
  compose_matrix(value, _scale, LVecBase3::zero(), _hpr, _pos);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_value_no_scale_shear
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame,
//               without any scale or shear information.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_value_no_scale_shear(int, LMatrix4 &mat) {
  compose_matrix(mat, LVecBase3(1.0f, 1.0f, 1.0f), LVecBase3::zero(),
                 _hpr, _pos);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_scale
//       Access: Public, Virtual
//  Description: Gets the scale value at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_scale(int, LVecBase3 &scale) {
  scale = _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_hpr
//       Access: Public, Virtual
//  Description: Returns the h, p, and r components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_hpr(int, LVecBase3 &hpr) {
  hpr = _hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_quat
//       Access: Public, Virtual
//  Description: Returns the rotation component associated with the
//               current frame, expressed as a quaternion.  As above,
//               this only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_quat(int, LQuaternion &quat) {
  quat.set_hpr(_hpr);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_pos
//       Access: Public, Virtual
//  Description: Returns the x, y, and z translation components
//               associated with the current frame.  As above, this
//               only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_pos(int, LVecBase3 &pos) {
  pos = _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_shear
//       Access: Public, Virtual
//  Description: Returns the a, b, and c shear components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_shear(int, LVecBase3 &shear) {
  shear = LVecBase3::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
output(ostream &out) const {
  AnimChannel<ACMatrixSwitchType>::output(out);
  out << ": pos " << _pos << " hpr " << _hpr << " scale " << _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               AnimChannelMatrixFixed.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
write_datagram(BamWriter *manager, Datagram &dg) {
  AnimChannel<ACMatrixSwitchType>::write_datagram(manager, dg);

  _pos.write_datagram(dg);
  _hpr.write_datagram(dg);
  _scale.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type AnimChannelMatrixFixed is encountered
//               in the Bam file.  It should create the AnimChannelMatrixFixed
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *AnimChannelMatrixFixed::
make_from_bam(const FactoryParams &params) {
  AnimChannelMatrixFixed *chan = new AnimChannelMatrixFixed("", LVecBase3::zero(), LVecBase3::zero(), LVecBase3::zero());
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  chan->fillin(scan, manager);

  return chan;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new AnimChannelMatrixFixed.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel<ACMatrixSwitchType>::fillin(scan, manager);

  _pos.read_datagram(scan);
  _hpr.read_datagram(scan);
  _scale.read_datagram(scan);
}
