/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file axisEventList.cxx
 * @author jb
 * @date 2023-11-21
 */

#include "axisEventList.h"
#include "indent.h"

TypeHandle AxisEventList::_type_handle;

/**
 * Appends the events from the other list onto the end of this one.
 */
void AxisEventList::
add_events(const AxisEventList &other) {
  _events.reserve(_events.size() + other._events.size());
  Events::const_iterator ei;
  for (ei = other._events.begin(); ei != other._events.end(); ++ei) {
    _events.push_back(*ei);
  }
}

/**
 *
 */
void AxisEventList::
output(std::ostream &out) const {
  if (_events.empty()) {
    out << "(no axis events)";
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

/**
 *
 */
void AxisEventList::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << _events.size() << " axis events:\n";
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    indent(out, indent_level + 2) << (*ei) << "\n";
  }
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void AxisEventList::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AxisEventList::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_events.size());
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    (*ei).write_datagram(dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *AxisEventList::
make_from_bam(const FactoryParams &params) {
  AxisEventList *list = new AxisEventList;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  list->fillin(scan, manager);

  return list;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AxisEventList.
 *
 * This function is normally protected, but it is declared public in this case
 * so that InputDeviceNodeRecorder may call it to read a AxisEventList from the middle
 * of a datagram.
 */
void AxisEventList::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_events = scan.get_uint16();
  _events.clear();
  _events.reserve(num_events);
  for (int i = 0; i < num_events; i++) {
    AxisEvent event;
    event.read_datagram(scan);
    _events.push_back(event);
  }
}
