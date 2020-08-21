/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "materialAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle MaterialAttrib::_type_handle;
int MaterialAttrib::_attrib_slot;

/**
 * Constructs a new MaterialAttrib object suitable for rendering the indicated
 * material onto geometry.
 */
CPT(RenderAttrib) MaterialAttrib::
make(Material *material) {
  MaterialAttrib *attrib = new MaterialAttrib;
  attrib->_material = material;
  material->set_attrib_lock();
  return return_new(attrib);
}

/**
 * Constructs a new MaterialAttrib object suitable for rendering unmateriald
 * geometry.
 */
CPT(RenderAttrib) MaterialAttrib::
make_off() {
  MaterialAttrib *attrib = new MaterialAttrib;
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) MaterialAttrib::
make_default() {
  return return_new(new MaterialAttrib);
}

/**
 *
 */
void MaterialAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (_material != nullptr) {
    out << *_material;
  } else if (is_off()) {
    out << "(off)";
  }
}

/**
 * Intended to be overridden by derived MaterialAttrib types to return a
 * unique number indicating whether this MaterialAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two MaterialAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two MaterialAttrib objects whose get_type()
 * functions return the same.
 */
int MaterialAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const MaterialAttrib *ta = (const MaterialAttrib *)other;

  // Comparing pointers by subtraction is problematic.  Instead of doing this,
  // we'll just depend on the built-in != and < operators for comparing
  // pointers.
  if (_material != ta->_material) {
    return _material < ta->_material ? -1 : 1;
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t MaterialAttrib::
get_hash_impl() const {
  return pointer_hash::add_hash(0, _material);
}

/**
 * Tells the BamReader how to create objects of type MaterialAttrib.
 */
void MaterialAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MaterialAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _material);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int MaterialAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *material = p_list[pi++];
  _material = DCAST(Material, material);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type MaterialAttrib is encountered in the Bam file.  It should create the
 * MaterialAttrib and extract its information from the file.
 */
TypedWritable *MaterialAttrib::
make_from_bam(const FactoryParams &params) {
  MaterialAttrib *attrib = new MaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);
  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MaterialAttrib.
 */
void MaterialAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _material pointer.
  manager->read_pointer(scan);
}
