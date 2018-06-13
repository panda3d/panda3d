/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthWriteAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "depthWriteAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DepthWriteAttrib::_type_handle;
int DepthWriteAttrib::_attrib_slot;

/**
 * Constructs a new DepthWriteAttrib object.
 */
CPT(RenderAttrib) DepthWriteAttrib::
make(DepthWriteAttrib::Mode mode) {
  DepthWriteAttrib *attrib = new DepthWriteAttrib(mode);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) DepthWriteAttrib::
make_default() {
  return return_new(new DepthWriteAttrib);
}

/**
 *
 */
void DepthWriteAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_off:
    out << "off";
    break;
  case M_on:
    out << "on";
    break;
  }
}

/**
 * Intended to be overridden by derived DepthWriteAttrib types to return a
 * unique number indicating whether this DepthWriteAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two DepthWriteAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two DepthWriteAttrib objects whose get_type()
 * functions return the same.
 */
int DepthWriteAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const DepthWriteAttrib *ta = (const DepthWriteAttrib *)other;

  return (int)_mode - (int)ta->_mode;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t DepthWriteAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type DepthWriteAttrib.
 */
void DepthWriteAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DepthWriteAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type DepthWriteAttrib is encountered in the Bam file.  It should create the
 * DepthWriteAttrib and extract its information from the file.
 */
TypedWritable *DepthWriteAttrib::
make_from_bam(const FactoryParams &params) {
  DepthWriteAttrib *attrib = new DepthWriteAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new DepthWriteAttrib.
 */
void DepthWriteAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
