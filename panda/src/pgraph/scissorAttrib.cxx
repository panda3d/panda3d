/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorAttrib.cxx
 * @author drose
 * @date 2008-07-29
 */

#include "scissorAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

using std::max;
using std::min;

TypeHandle ScissorAttrib::_type_handle;
int ScissorAttrib::_attrib_slot;
CPT(RenderAttrib) ScissorAttrib::_off_attrib;

/**
 * Use ScissorAttrib::make() to construct a new ScissorAttrib object.
 */
ScissorAttrib::
ScissorAttrib(const LVecBase4 &frame) :
  _frame(frame),
  _off(false)
{
  // Impose sensible bounds.
  _frame[0] = max(min(_frame[0], (PN_stdfloat)1.0), (PN_stdfloat)0.0);
  _frame[1] = max(min(_frame[1], (PN_stdfloat)1.0), _frame[0]);
  _frame[2] = max(min(_frame[2], (PN_stdfloat)1.0), (PN_stdfloat)0.0);
  _frame[3] = max(min(_frame[3], (PN_stdfloat)1.0), _frame[2]);
}

/**
 * Constructs a new ScissorAttrib object that removes the scissor region and
 * fills the DisplayRegion.
 */
CPT(RenderAttrib) ScissorAttrib::
make_off() {
  if (_off_attrib != nullptr) {
    return _off_attrib;
  }
  ScissorAttrib *attrib = new ScissorAttrib(LVecBase4(0.0f, 1.0f, 0.0f, 1.0f));
  attrib->_off = true;
  _off_attrib = return_new(attrib);
  return _off_attrib;
}

/**
 * Constructs a ScissorAttrib that restricts rendering to the indicated frame
 * within the current DisplayRegion.  (0,0) is the lower-left corner of the
 * DisplayRegion, and (1,1) is the upper-right corner.
 */
CPT(RenderAttrib) ScissorAttrib::
make(const LVecBase4 &frame) {
  ScissorAttrib *attrib = new ScissorAttrib(frame);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ScissorAttrib::
make_default() {
  return make_off();
  // return return_new(new ScissorAttrib(LVecBase4(0.0f, 1.0f, 0.0f, 1.0f)));
}

/**
 *
 */
void ScissorAttrib::
output(std::ostream &out) const {
  out << get_type() << ":[" << _frame << "]";
}

/**
 * Intended to be overridden by derived ScissorAttrib types to return a unique
 * number indicating whether this ScissorAttrib is equivalent to the other
 * one.
 *
 * This should return 0 if the two ScissorAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ScissorAttrib objects whose get_type()
 * functions return the same.
 */
int ScissorAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ScissorAttrib *ta = (const ScissorAttrib *)other;

  if (_off && ta->_off) {
    return 0;
  }

  if (_off && !ta->_off) {
    return -1;
  }

  if (!_off && ta->_off) {
    return 1;
  }

  return _frame.compare_to(ta->_frame);
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ScissorAttrib::
get_hash_impl() const {
  size_t hash = 0;
  if (!_off) {
    hash = _frame.add_hash(hash);
  }
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
CPT(RenderAttrib) ScissorAttrib::
compose_impl(const RenderAttrib *other) const {
  if (_off) {
    return other;
  }

  const ScissorAttrib *ta = (const ScissorAttrib *)other;

  if (ta->_off) {
    return this;
  }

  LVecBase4 new_frame(max(ta->_frame[0], _frame[0]),
                       min(ta->_frame[1], _frame[1]),
                       max(ta->_frame[2], _frame[2]),
                       min(ta->_frame[3], _frame[3]));

  ScissorAttrib *attrib = new ScissorAttrib(new_frame);
  return return_new(attrib);
}

/**
 * Tells the BamReader how to create objects of type ScissorAttrib.
 */
void ScissorAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ScissorAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  _frame.write_datagram(dg);
  if (manager->get_file_minor_ver() >= 34) {
    dg.add_bool(_off);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ScissorAttrib is encountered in the Bam file.  It should create the
 * ScissorAttrib and extract its information from the file.
 */
TypedWritable *ScissorAttrib::
make_from_bam(const FactoryParams &params) {
  ScissorAttrib *attrib = new ScissorAttrib(LVecBase4(0.0f, 1.0f, 0.0f, 1.0f));
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ScissorAttrib.
 */
void ScissorAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _frame.read_datagram(scan);
  _off = false;

  if (manager->get_file_minor_ver() >= 34) {
    _off = scan.get_bool();
  }
}
