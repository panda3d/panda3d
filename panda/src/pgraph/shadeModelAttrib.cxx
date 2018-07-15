/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shadeModelAttrib.cxx
 * @author drose
 * @date 2005-03-14
 */

#include "shadeModelAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ShadeModelAttrib::_type_handle;
int ShadeModelAttrib::_attrib_slot;

/**
 * Constructs a new ShadeModelAttrib object that specifies whether to draw
 * polygons with flat shading or with per-vertex (smooth) shading.
 */
CPT(RenderAttrib) ShadeModelAttrib::
make(ShadeModelAttrib::Mode mode) {
  ShadeModelAttrib *attrib = new ShadeModelAttrib(mode);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ShadeModelAttrib::
make_default() {
  return return_new(new ShadeModelAttrib(M_smooth));
}

/**
 *
 */
void ShadeModelAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_flat:
    out << "flat";
    break;

  case M_smooth:
    out << "smooth";
    break;
  }
}

/**
 * Intended to be overridden by derived ShadeModelAttrib types to return a
 * unique number indicating whether this ShadeModelAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two ShadeModelAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ShadeModelAttrib objects whose get_type()
 * functions return the same.
 */
int ShadeModelAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShadeModelAttrib *ta = (const ShadeModelAttrib *)other;
  return (int)_mode - (int)ta->_mode;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ShadeModelAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  return hash;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) ShadeModelAttrib::
compose_impl(const RenderAttrib *other) const {
  const ShadeModelAttrib *ta = (const ShadeModelAttrib *)other;

  Mode mode = ta->get_mode();
  return make(mode);
}

/**
 * Tells the BamReader how to create objects of type ShadeModelAttrib.
 */
void ShadeModelAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShadeModelAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShadeModelAttrib is encountered in the Bam file.  It should create the
 * ShadeModelAttrib and extract its information from the file.
 */
TypedWritable *ShadeModelAttrib::
make_from_bam(const FactoryParams &params) {
  ShadeModelAttrib *attrib = new ShadeModelAttrib(M_smooth);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShadeModelAttrib.
 */
void ShadeModelAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
