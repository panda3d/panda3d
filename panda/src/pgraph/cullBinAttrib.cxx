/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinAttrib.cxx
 * @author drose
 * @date 2002-03-01
 */

#include "cullBinAttrib.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CullBinAttrib::_type_handle;
int CullBinAttrib::_attrib_slot;

/**
 * Constructs a new CullBinAttrib assigning geometry into the named bin.  If
 * the bin name is the empty string, the default bin is used.
 *
 * The draw_order specifies further ordering information which is relevant
 * only to certain kinds of bins (in particular CullBinFixed type bins).
 */
CPT(RenderAttrib) CullBinAttrib::
make(const std::string &bin_name, int draw_order) {
  CullBinAttrib *attrib = new CullBinAttrib;
  attrib->_bin_name = bin_name;
  attrib->_draw_order = draw_order;
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) CullBinAttrib::
make_default() {
  return return_new(new CullBinAttrib);
}

/**
 *
 */
void CullBinAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (_bin_name.empty()) {
    out << "(default)";
  } else {
    out << _bin_name << "," << _draw_order;
  }
}

/**
 * Intended to be overridden by derived CullBinAttrib types to return a unique
 * number indicating whether this CullBinAttrib is equivalent to the other
 * one.
 *
 * This should return 0 if the two CullBinAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two CullBinAttrib objects whose get_type()
 * functions return the same.
 */
int CullBinAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const CullBinAttrib *ta = (const CullBinAttrib *)other;

  if (_draw_order != ta->_draw_order) {
    return _draw_order - ta->_draw_order;
  }
  return strcmp(_bin_name.c_str(), ta->_bin_name.c_str());
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t CullBinAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, _draw_order);
  hash = string_hash::add_hash(hash, _bin_name);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type CullBinAttrib.
 */
void CullBinAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CullBinAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_string(_bin_name);
  dg.add_int32(_draw_order);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CullBinAttrib is encountered in the Bam file.  It should create the
 * CullBinAttrib and extract its information from the file.
 */
TypedWritable *CullBinAttrib::
make_from_bam(const FactoryParams &params) {
  CullBinAttrib *attrib = new CullBinAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CullBinAttrib.
 */
void CullBinAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _bin_name = scan.get_string();
  _draw_order = scan.get_int32();
}
