// Filename: pointerEventList.cxx
// Created by: jyelon (20Sep2007)
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

#include "pointerEventList.h"
#include "indent.h"

TypeHandle PointerEventList::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::add_events
//       Access: Public
//  Description: Appends the events from the other list onto the end
//               of this one.
////////////////////////////////////////////////////////////////////
void PointerEventList::
add_events(const PointerEventList &other) {
  Events::const_iterator ei;
  for (ei = other._events.begin(); ei != other._events.end(); ++ei) {
    _events.push_back(*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PointerEventList::
output(ostream &out) const {
  if (_events.empty()) {
    out << "(no pointers)";
  } else {
    Events::const_iterator ei;
    ei = _events.begin();
    out << "(" << (*ei);
    ++ei;
    while (ei != _events.end()) {
      out << " " << (*ei);
      ++ei;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PointerEventList::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _events.size() << " events:\n";
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    indent(out, indent_level + 2) << (*ei) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Lens.
////////////////////////////////////////////////////////////////////
void PointerEventList::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PointerEventList::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_events.size());
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    (*ei).write_datagram(dg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Lens is encountered
//               in the Bam file.  It should create the Lens
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PointerEventList::
make_from_bam(const FactoryParams &params) {
  PointerEventList *list = new PointerEventList;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  list->fillin(scan, manager);

  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::fillin
//       Access: Public
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PointerEventList.
//
//               This function is normally protected, but it is
//               declared public in this case so that MouseRecorder
//               may call it to read a PointerEventList from the middle
//               of a datagram.
////////////////////////////////////////////////////////////////////
void PointerEventList::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_events = scan.get_uint16();
  _events.clear();
  for (int i = 0; i < num_events; i++) {
    PointerEvent event;
    event.read_datagram(scan);
    _events.push_back(event);
  }
}
