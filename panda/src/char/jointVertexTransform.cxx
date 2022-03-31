/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointVertexTransform.cxx
 * @author drose
 * @date 2005-03-24
 */

#include "jointVertexTransform.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "lightMutexHolder.h"

TypeHandle JointVertexTransform::_type_handle;

/**
 * Constructs an invalid object; used only by the bam loader.
 */
JointVertexTransform::
JointVertexTransform()
{
}

/**
 * Constructs a new object that converts vertices from the indicated joint's
 * coordinate space, into the other indicated joint's space.
 */
JointVertexTransform::
JointVertexTransform(CharacterJoint *joint) :
  _joint(joint)
{
  // Tell the joint that we need to be informed when it moves.
  _joint->_vertex_transforms.insert(this);
  mark_modified(Thread::get_current_thread());
}

/**
 *
 */
JointVertexTransform::
~JointVertexTransform() {
  // Tell the joint to stop informing us about its motion.
  _joint->_vertex_transforms.erase(this);
}

/**
 * Stores the transform's matrix in the indicated object.
 */
void JointVertexTransform::
get_matrix(LMatrix4 &matrix) const {
  matrix = _joint->_skinning_matrix;
}

/**
 * Premultiplies this transform's matrix with the indicated previous matrix,
 * so that the result is the net composition of the given transform with this
 * transform.  The result is stored in the parameter "result", which should
 * not be the same matrix as previous.
 */
void JointVertexTransform::
mult_matrix(LMatrix4 &result, const LMatrix4 &previous) const {
  result.multiply(_joint->_skinning_matrix, previous);
}

/**
 * Adds the value of this transform's matrix, modified by the indicated
 * weight, into the indicated accumulation matrix.  This is used to compute
 * the result of several blended transforms.
 */
void JointVertexTransform::
accumulate_matrix(LMatrix4 &accum, PN_stdfloat weight) const {
  accum.accumulate(_joint->_skinning_matrix, weight);
}

/**
 *
 */
void JointVertexTransform::
output(std::ostream &out) const {
  out << _joint->get_name();
}

/**
 * Tells the BamReader how to create objects of type JointVertexTransform.
 */
void JointVertexTransform::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void JointVertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexTransform::write_datagram(manager, dg);

  manager->write_pointer(dg, _joint);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int JointVertexTransform::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = VertexTransform::complete_pointers(p_list, manager);

  _joint = DCAST(CharacterJoint, p_list[pi++]);
  _joint->_vertex_transforms.insert(this);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type JointVertexTransform is encountered in the Bam file.  It should create
 * the JointVertexTransform and extract its information from the file.
 */
TypedWritable *JointVertexTransform::
make_from_bam(const FactoryParams &params) {
  JointVertexTransform *object = new JointVertexTransform;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new JointVertexTransform.
 */
void JointVertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexTransform::fillin(scan, manager);

  manager->read_pointer(scan);
  mark_modified(Thread::get_current_thread());
}
