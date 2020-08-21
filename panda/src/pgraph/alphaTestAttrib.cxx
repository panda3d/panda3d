/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file alphaTestAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "alphaTestAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "auxBitplaneAttrib.h"

TypeHandle AlphaTestAttrib::_type_handle;
int AlphaTestAttrib::_attrib_slot;

/**
 * Constructs a new AlphaTestAttrib object.
 */
CPT(RenderAttrib) AlphaTestAttrib::
make(PandaCompareFunc mode, PN_stdfloat reference_value) {
  assert((reference_value >=0.0f) && (reference_value <=1.0f));
  AlphaTestAttrib *attrib = new AlphaTestAttrib(mode,reference_value);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) AlphaTestAttrib::
make_default() {
  return return_new(new AlphaTestAttrib);
}

/**
 *
 */
void AlphaTestAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  output_comparefunc(out,_mode);
  out << "," << _reference_alpha;
}

/**
 * Intended to be overridden by derived AlphaTestAttrib types to return a
 * unique number indicating whether this AlphaTestAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two AlphaTestAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two AlphaTestAttrib objects whose get_type()
 * functions return the same.
 */
int AlphaTestAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AlphaTestAttrib *ta = (const AlphaTestAttrib *)other;

  int compare_result = ((int)_mode - (int)ta->_mode) ;
  if (compare_result != 0) {
    return compare_result;
  }

  if (_reference_alpha != ta->_reference_alpha) {
    return _reference_alpha < ta->_reference_alpha ? -1 : 1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t AlphaTestAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  hash = float_hash().add_hash(hash, _reference_alpha);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type AlphaTestAttrib.
 */
void AlphaTestAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AlphaTestAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  dg.add_stdfloat(_reference_alpha);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type AlphaTestAttrib is encountered in the Bam file.  It should create the
 * AlphaTestAttrib and extract its information from the file.
 */
TypedWritable *AlphaTestAttrib::
make_from_bam(const FactoryParams &params) {
  AlphaTestAttrib *attrib = new AlphaTestAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AlphaTestAttrib.
 */
void AlphaTestAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (PandaCompareFunc)scan.get_int8();
  _reference_alpha = scan.get_stdfloat();
}
