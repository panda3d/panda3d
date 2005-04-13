// Filename: vertexSlider.cxx
// Created by:  drose (28Mar05)
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

#include "vertexSlider.h"
#include "vertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"

TypeHandle VertexSlider::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexSlider::
VertexSlider(const InternalName *name) : _name(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VertexSlider::
~VertexSlider() {
  // We shouldn't destruct while any SliderTables are holding our
  // pointer.
  nassertv(_tables.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexSlider::
output(ostream &out) const {
  out << get_type() << " " << *get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexSlider::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) 
    << *this << " = " << get_slider() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::mark_modified
//       Access: Protected
//  Description: Intended to be called by a derived class whenever the
//               reported transform might have changed.  Without
//               calling this method, changes to get_slider() may not
//               be propagated through the system.
////////////////////////////////////////////////////////////////////
void VertexSlider::
mark_modified() {
  CDWriter cdata(_cycler);
  cdata->_modified = VertexTransform::get_next_modified();
  
  Tables::iterator ti;
  for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
    (*ti)->update_modified(cdata->_modified);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void VertexSlider::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new VertexSlider.
////////////////////////////////////////////////////////////////////
void VertexSlider::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *VertexSlider::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void VertexSlider::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int VertexSlider::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new VertexSlider.
////////////////////////////////////////////////////////////////////
void VertexSlider::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
