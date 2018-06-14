/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file auxBitplaneAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "auxBitplaneAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle AuxBitplaneAttrib::_type_handle;
int AuxBitplaneAttrib::_attrib_slot;
CPT(RenderAttrib) AuxBitplaneAttrib::_default;

/**
 * Constructs a default AuxBitplaneAttrib object.
 */
CPT(RenderAttrib) AuxBitplaneAttrib::
make() {
  if (_default == nullptr) {
    AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(0);
    _default = return_new(attrib);
  }
  return _default;
}

/**
 * Constructs a specified AuxBitplaneAttrib object.
 */
CPT(RenderAttrib) AuxBitplaneAttrib::
make(int outputs) {
  AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(outputs);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) AuxBitplaneAttrib::
make_default() {
  return return_new(new AuxBitplaneAttrib(0));
}

/**
 *
 */
void AuxBitplaneAttrib::
output(std::ostream &out) const {
  out << get_type() << "(" << _outputs << ")";
}

/**
 * Intended to be overridden by derived AuxBitplaneAttrib types to return a
 * unique number indicating whether this AuxBitplaneAttrib is equivalent to
 * the other one.
 *
 * This should return 0 if the two AuxBitplaneAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two AuxBitplaneAttrib objects whose
 * get_type() functions return the same.
 */
int AuxBitplaneAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AuxBitplaneAttrib *ta = (const AuxBitplaneAttrib *)other;

  int compare_result = _outputs - ta->_outputs;
  if (compare_result != 0) {
    return compare_result;
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t AuxBitplaneAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, _outputs);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type AuxBitplaneAttrib.
 */
void AuxBitplaneAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AuxBitplaneAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int32(_outputs);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type AuxBitplaneAttrib is encountered in the Bam file.  It should create
 * the AuxBitplaneAttrib and extract its information from the file.
 */
TypedWritable *AuxBitplaneAttrib::
make_from_bam(const FactoryParams &params) {
  AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(0);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AuxBitplaneAttrib.
 */
void AuxBitplaneAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _outputs = scan.get_int32();
}
