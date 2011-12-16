// Filename: jointVertexTransform.cxx
// Created by:  drose (24Mar05)
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

#include "jointVertexTransform.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "lightMutexHolder.h"

TypeHandle JointVertexTransform::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::Default Constructor
//       Access: Private
//  Description: Constructs an invalid object; used only by the bam
//               loader.
////////////////////////////////////////////////////////////////////
JointVertexTransform::
JointVertexTransform() :
  _matrix_stale(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::Constructor
//       Access: Published
//  Description: Constructs a new object that converts vertices from
//               the indicated joint's coordinate space, into the
//               other indicated joint's space.
////////////////////////////////////////////////////////////////////
JointVertexTransform::
JointVertexTransform(CharacterJoint *joint) :
  _joint(joint),
  _matrix_stale(true)
{
  // Tell the joint that we need to be informed when it moves.
  _joint->_vertex_transforms.insert(this);
  mark_modified(Thread::get_current_thread());
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
JointVertexTransform::
~JointVertexTransform() {
  // Tell the joint to stop informing us about its motion.
  _joint->_vertex_transforms.erase(this);
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::get_matrix
//       Access: Published, Virtual
//  Description: Stores the transform's matrix in the indicated object.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
get_matrix(LMatrix4 &matrix) const {
  check_matrix();
  matrix = _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::mult_matrix
//       Access: Published, Virtual
//  Description: Premultiplies this transform's matrix with the
//               indicated previous matrix, so that the result is the
//               net composition of the given transform with this
//               transform.  The result is stored in the parameter
//               "result", which should not be the same matrix as
//               previous.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
mult_matrix(LMatrix4 &result, const LMatrix4 &previous) const {
  check_matrix();
  result.multiply(_matrix, previous);
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::accumulate_matrix
//       Access: Published, Virtual
//  Description: Adds the value of this transform's matrix, modified
//               by the indicated weight, into the indicated
//               accumulation matrix.  This is used to compute the
//               result of several blended transforms.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
accumulate_matrix(LMatrix4 &accum, PN_stdfloat weight) const {
  check_matrix();

  accum.accumulate(_matrix, weight);
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
output(ostream &out) const {
  out << _joint->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::compute_matrix
//       Access: Private
//  Description: Recomputes _matrix if it needs it.  Uses locking.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
compute_matrix() {
  LightMutexHolder holder(_lock);
  if (_matrix_stale) {
    _matrix = _joint->_initial_net_transform_inverse * _joint->_net_transform;
    _matrix_stale = false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               JointVertexTransform.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexTransform::write_datagram(manager, dg);

  manager->write_pointer(dg, _joint);
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int JointVertexTransform::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = VertexTransform::complete_pointers(p_list, manager);

  _joint = DCAST(CharacterJoint, p_list[pi++]);    
  _joint->_vertex_transforms.insert(this);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type JointVertexTransform is encountered
//               in the Bam file.  It should create the JointVertexTransform
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *JointVertexTransform::
make_from_bam(const FactoryParams &params) {
  JointVertexTransform *object = new JointVertexTransform;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new JointVertexTransform.
////////////////////////////////////////////////////////////////////
void JointVertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexTransform::fillin(scan, manager);

  manager->read_pointer(scan);
  _matrix_stale = true;
  mark_modified(Thread::get_current_thread());
}
