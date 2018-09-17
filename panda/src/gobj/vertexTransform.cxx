/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexTransform.cxx
 * @author drose
 * @date 2005-03-23
 */

#include "vertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"
#include "transformTable.h"

PipelineCycler<VertexTransform::CData> VertexTransform::_global_cycler;
UpdateSeq VertexTransform::_next_modified;

TypeHandle VertexTransform::_type_handle;

/**
 *
 */
VertexTransform::
VertexTransform() {
}

/**
 *
 */
VertexTransform::
~VertexTransform() {
  // We shouldn't destruct while any TransformTables are holding our pointer.
  nassertv(_tables.empty());
}

/**
 * Premultiplies this transform's matrix with the indicated previous matrix,
 * so that the result is the net composition of the given transform with this
 * transform.  The result is stored in the parameter "result", which should
 * not be the same matrix as previous.
 */
void VertexTransform::
mult_matrix(LMatrix4 &result, const LMatrix4 &previous) const {
  nassertv(&result != &previous);
  LMatrix4 me;
  get_matrix(me);
  result.multiply(me, previous);
}

/**
 * Adds the value of this transform's matrix, modified by the indicated
 * weight, into the indicated accumulation matrix.  This is used to compute
 * the result of several blended transforms.
 */
void VertexTransform::
accumulate_matrix(LMatrix4 &accum, PN_stdfloat weight) const {
  LMatrix4 me;
  get_matrix(me);
  accum.accumulate(me, weight);
}

/**
 *
 */
void VertexTransform::
output(std::ostream &out) const {
  out << get_type();
}

/**
 *
 */
void VertexTransform::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << *this << ":\n";
  LMatrix4 mat;
  get_matrix(mat);
  mat.write(out, indent_level + 2);
}

/**
 * Returns a monotonically increasing sequence.  Each time this is called, a
 * new sequence number is returned, higher than the previous value.
 *
 * This is used to ensure that all VertexTransform::get_modified() calls
 * return an increasing number in the same space, so that
 * TransformBlend::get_modified() is easy to determine.  It is similar to
 * Geom::get_modified(), but it is in a different space.
 */
UpdateSeq VertexTransform::
get_next_modified(Thread *current_thread) {
  CDWriter cdatag(_global_cycler, true, current_thread);
  ++_next_modified;
  cdatag->_modified = _next_modified;

  return _next_modified;
}

/**
 * Intended to be called by a derived class whenever the reported transform
 * might have changed.  Without calling this method, changes to get_matrix()
 * may not be propagated through the system.
 */
void VertexTransform::
mark_modified(Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_modified = get_next_modified(current_thread);

  Palettes::iterator pi;
  for (pi = _tables.begin(); pi != _tables.end(); ++pi) {
    (*pi)->update_modified(cdata->_modified, current_thread);
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void VertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new VertexTransform.
 */
void VertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
}

/**
 *
 */
CycleData *VertexTransform::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void VertexTransform::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int VertexTransform::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new VertexTransform.
 */
void VertexTransform::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
