// Filename: jointVertexTransform.cxx
// Created by:  drose (24Mar05)
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

#include "jointVertexTransform.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

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
get_matrix(LMatrix4f &matrix) const {
  if (_matrix_stale) {
    ((JointVertexTransform *)this)->_matrix = 
      _joint->_initial_net_transform_inverse *
      _joint->_net_transform;
    ((JointVertexTransform *)this)->_matrix_stale = false;
  }
  
  matrix = _matrix;
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
}
