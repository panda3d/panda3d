/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fogAttrib.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "fogAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle FogAttrib::_type_handle;
int FogAttrib::_attrib_slot;

/**
 * Constructs a new FogAttrib object suitable for rendering the indicated fog
 * onto geometry.
 */
CPT(RenderAttrib) FogAttrib::
make(Fog *fog) {
  FogAttrib *attrib = new FogAttrib;
  attrib->_fog = fog;
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) FogAttrib::
make_default() {
  return return_new(new FogAttrib);
}

/**
 * Constructs a new FogAttrib object suitable for rendering unfogd geometry.
 */
CPT(RenderAttrib) FogAttrib::
make_off() {
  FogAttrib *attrib = new FogAttrib;
  return return_new(attrib);
}

/**
 *
 */
void FogAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    out << *_fog;
  }
}

/**
 * Intended to be overridden by derived FogAttrib types to return a unique
 * number indicating whether this FogAttrib is equivalent to the other one.
 *
 * This should return 0 if the two FogAttrib objects are equivalent, a number
 * less than zero if this one should be sorted before the other one, and a
 * number greater than zero otherwise.
 *
 * This will only be called with two FogAttrib objects whose get_type()
 * functions return the same.
 */
int FogAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const FogAttrib *ta = (const FogAttrib *)other;

  // Comparing pointers by subtraction is problematic.  Instead of doing this,
  // we'll just depend on the built-in != and < operators for comparing
  // pointers.
  if (_fog != ta->_fog) {
    return _fog < ta->_fog ? -1 : 1;
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t FogAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _fog);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type FogAttrib.
 */
void FogAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void FogAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _fog);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int FogAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *fog = p_list[pi++];
  if (fog != nullptr) {
    _fog = DCAST(Fog, fog);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type FogAttrib is encountered in the Bam file.  It should create the
 * FogAttrib and extract its information from the file.
 */
TypedWritable *FogAttrib::
make_from_bam(const FactoryParams &params) {
  FogAttrib *attrib = new FogAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new FogAttrib.
 */
void FogAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _fog pointer.
  manager->read_pointer(scan);
}
