/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthBiasAttrib.cxx
 * @author rdb
 * @date 2021-08-24
 */

#include "depthBiasAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DepthBiasAttrib::_type_handle;
int DepthBiasAttrib::_attrib_slot;

/**
 * Constructs a new DepthBiasAttrib object that indicates the slope factor,
 * constant factor, and an optional clamping value.
 */
CPT(RenderAttrib) DepthBiasAttrib::
make(PN_stdfloat slope_factor, PN_stdfloat constant_factor, PN_stdfloat clamp) {
  DepthBiasAttrib *attrib = new DepthBiasAttrib(slope_factor, constant_factor, clamp);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) DepthBiasAttrib::
make_default() {
  return return_new(new DepthBiasAttrib(0, 0, 0));
}

/**
 *
 */
void DepthBiasAttrib::
output(std::ostream &out) const {
  out << get_type() << ":(" << get_slope_factor() << ", " << get_constant_factor()
      << ", " << get_clamp() << ")";
}

/**
 * Intended to be overridden by derived DepthBiasAttrib types to return a
 * unique number indicating whether this DepthBiasAttrib is equivalent to
 * the other one.
 *
 * This should return 0 if the two DepthBiasAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two DepthBiasAttrib objects whose
 * get_type() functions return the same.
 */
int DepthBiasAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const DepthBiasAttrib *ta = (const DepthBiasAttrib *)other;

  if (_slope_factor != ta->_slope_factor) {
    return _slope_factor < ta->_slope_factor ? -1 : 1;
  }
  if (_constant_factor != ta->_constant_factor) {
    return _constant_factor < ta->_constant_factor ? -1 : 1;
  }
  if (_clamp != ta->_clamp) {
    return _clamp < ta->_clamp ? -1 : 1;
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t DepthBiasAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = float_hash().add_hash(hash, _slope_factor);
  hash = float_hash().add_hash(hash, _constant_factor);
  hash = float_hash().add_hash(hash, _clamp);
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
CPT(RenderAttrib) DepthBiasAttrib::
compose_impl(const RenderAttrib *other) const {
  const DepthBiasAttrib *ba = (const DepthBiasAttrib *)other;

  return return_new(new DepthBiasAttrib(ba->_slope_factor + _slope_factor,
                                        ba->_constant_factor + _constant_factor,
                                        ba->_clamp));
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) DepthBiasAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const DepthBiasAttrib *ba = (const DepthBiasAttrib *)other;

  return return_new(new DepthBiasAttrib(ba->_slope_factor - _slope_factor,
                                        ba->_constant_factor - _constant_factor,
                                        ba->_clamp));
}

/**
 * Tells the BamReader how to create objects of type DepthBiasAttrib.
 */
void DepthBiasAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DepthBiasAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_stdfloat(_slope_factor);
  dg.add_stdfloat(_constant_factor);
  dg.add_stdfloat(_clamp);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type DepthBiasAttrib is encountered in the Bam file.  It should create
 * the DepthBiasAttrib and extract its information from the file.
 */
TypedWritable *DepthBiasAttrib::
make_from_bam(const FactoryParams &params) {
  DepthBiasAttrib *attrib = new DepthBiasAttrib(0, 0, 0);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new DepthBiasAttrib.
 */
void DepthBiasAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _slope_factor = scan.get_stdfloat();
  _constant_factor = scan.get_stdfloat();
  _clamp = scan.get_stdfloat();
}
