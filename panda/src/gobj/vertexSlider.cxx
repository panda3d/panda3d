/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexSlider.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "vertexSlider.h"
#include "vertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"
#include "sliderTable.h"

TypeHandle VertexSlider::_type_handle;

/**
 *
 */
VertexSlider::
VertexSlider(const InternalName *name) : _name(name) {
}

/**
 *
 */
VertexSlider::
~VertexSlider() {
  // We shouldn't destruct while any SliderTables are holding our pointer.
  nassertv(_tables.empty());
}

/**
 *
 */
void VertexSlider::
output(std::ostream &out) const {
  out << get_type() << " " << *get_name();
}

/**
 *
 */
void VertexSlider::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << *this << " = " << get_slider() << "\n";
}

/**
 * Intended to be called by a derived class whenever the reported transform
 * might have changed.  Without calling this method, changes to get_slider()
 * may not be propagated through the system.
 */
void VertexSlider::
mark_modified(Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_modified = VertexTransform::get_next_modified(current_thread);

  Tables::iterator ti;
  for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
    (*ti)->update_modified(cdata->_modified, current_thread);
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void VertexSlider::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new VertexSlider.
 */
void VertexSlider::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
}

/**
 *
 */
CycleData *VertexSlider::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void VertexSlider::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int VertexSlider::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new VertexSlider.
 */
void VertexSlider::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
