/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transparencyAttrib.cxx
 * @author drose
 * @date 2002-02-28
 */

#include "transparencyAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle TransparencyAttrib::_type_handle;
int TransparencyAttrib::_attrib_slot;

/**
 * Constructs a new TransparencyAttrib object.
 */
CPT(RenderAttrib) TransparencyAttrib::
make(TransparencyAttrib::Mode mode) {
  TransparencyAttrib *attrib = new TransparencyAttrib(mode);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) TransparencyAttrib::
make_default() {
  return return_new(new TransparencyAttrib);
}

/**
 *
 */
void TransparencyAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_none:
    out << "none";
    break;

  case M_alpha:
    out << "alpha";
    break;

  case M_premultiplied_alpha:
    out << "premultiplied alpha";
    break;

  case M_multisample:
    out << "multisample";
    break;

  case M_multisample_mask:
    out << "multisample mask";
    break;

  case M_binary:
    out << "binary";
    break;

  case M_dual:
    out << "dual";
    break;
  }
}

/**
 * Intended to be overridden by derived TransparencyAttrib types to return a
 * unique number indicating whether this TransparencyAttrib is equivalent to
 * the other one.
 *
 * This should return 0 if the two TransparencyAttrib objects are equivalent,
 * a number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two TransparencyAttrib objects whose
 * get_type() functions return the same.
 */
int TransparencyAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TransparencyAttrib *ta = (const TransparencyAttrib *)other;

  return (int)_mode - (int)ta->_mode;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t TransparencyAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type TransparencyAttrib.
 */
void TransparencyAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TransparencyAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type TransparencyAttrib is encountered in the Bam file.  It should create
 * the TransparencyAttrib and extract its information from the file.
 */
TypedWritable *TransparencyAttrib::
make_from_bam(const FactoryParams &params) {
  TransparencyAttrib *attrib = new TransparencyAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TransparencyAttrib.
 */
void TransparencyAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
