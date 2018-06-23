/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorWriteAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "colorWriteAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ColorWriteAttrib::_type_handle;
int ColorWriteAttrib::_attrib_slot;

/**
 * Constructs a new ColorWriteAttrib object.
 */
CPT(RenderAttrib) ColorWriteAttrib::
make(unsigned int channels) {
  ColorWriteAttrib *attrib = new ColorWriteAttrib(channels);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ColorWriteAttrib::
make_default() {
  return return_new(new ColorWriteAttrib);
}

/**
 *
 */
void ColorWriteAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (_channels == 0) {
    out << "off";
  } else {
    if ((_channels & C_red) != 0) {
      out << "r";
    }
    if ((_channels & C_green) != 0) {
      out << "g";
    }
    if ((_channels & C_blue) != 0) {
      out << "b";
    }
    if ((_channels & C_alpha) != 0) {
      out << "a";
    }
  }
}

/**
 * Intended to be overridden by derived ColorWriteAttrib types to return a
 * unique number indicating whether this ColorWriteAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two ColorWriteAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ColorWriteAttrib objects whose get_type()
 * functions return the same.
 */
int ColorWriteAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorWriteAttrib *ta = (const ColorWriteAttrib *)other;
  return (int)_channels - (int)ta->_channels;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ColorWriteAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_channels);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type ColorWriteAttrib.
 */
void ColorWriteAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ColorWriteAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint8(_channels);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ColorWriteAttrib is encountered in the Bam file.  It should create the
 * ColorWriteAttrib and extract its information from the file.
 */
TypedWritable *ColorWriteAttrib::
make_from_bam(const FactoryParams &params) {
  ColorWriteAttrib *attrib = new ColorWriteAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ColorWriteAttrib.
 */
void ColorWriteAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _channels = scan.get_uint8();
}
