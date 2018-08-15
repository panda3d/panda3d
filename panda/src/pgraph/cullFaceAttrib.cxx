/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullFaceAttrib.cxx
 * @author drose
 * @date 2002-02-27
 */

#include "cullFaceAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CullFaceAttrib::_type_handle;
int CullFaceAttrib::_attrib_slot;

/**
 * Constructs a new CullFaceAttrib object that specifies how to cull geometry.
 * By Panda convention, vertices are ordered counterclockwise when seen from
 * the front, so the M_cull_clockwise will cull backfacing polygons.
 *
 * M_cull_unchanged is an identity attrib; if this is applied to vertices
 * without any other intervening attrib, it is the same as applying the
 * default attrib.
 */
CPT(RenderAttrib) CullFaceAttrib::
make(CullFaceAttrib::Mode mode) {
  CullFaceAttrib *attrib = new CullFaceAttrib(mode, false);
  return return_new(attrib);
}

/**
 * Constructs a new CullFaceAttrib object that reverses the effects of any
 * other CullFaceAttrib objects in the scene graph.  M_cull_clockwise will be
 * treated as M_cull_counter_clockwise, and vice-versa.  M_cull_none is
 * unchanged.
 */
CPT(RenderAttrib) CullFaceAttrib::
make_reverse() {
  CullFaceAttrib *attrib = new CullFaceAttrib(M_cull_unchanged, true);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) CullFaceAttrib::
make_default() {
  return return_new(new CullFaceAttrib(M_cull_clockwise, false));
}

/**
 * Returns the effective culling mode.  This is the same as the actual culling
 * mode, unless the reverse flag is set, which swaps CW for CCW and vice-
 * versa.  Also, M_cull_unchanged is mapped to M_cull_none.
 */
CullFaceAttrib::Mode CullFaceAttrib::
get_effective_mode() const {
  if (_reverse) {
    switch (_mode) {
    case M_cull_clockwise:
    case M_cull_unchanged:
      return M_cull_counter_clockwise;

    case M_cull_counter_clockwise:
      return M_cull_clockwise;

    default:
      break;
    }

  } else {
    switch (_mode) {
    case M_cull_clockwise:
    case M_cull_unchanged:
      return M_cull_clockwise;

    case M_cull_counter_clockwise:
      return M_cull_counter_clockwise;

    default:
      break;
    }
  }

  return M_cull_none;
}

/**
 *
 */
void CullFaceAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_actual_mode()) {
  case M_cull_none:
    out << "cull_none";
    break;
  case M_cull_clockwise:
    out << "cull_clockwise";
    break;
  case M_cull_counter_clockwise:
    out << "cull_counter_clockwise";
    break;
  case M_cull_unchanged:
    out << "cull_unchanged";
    break;
  }
  if (get_reverse()) {
    out << "(reverse)";
  }
}

/**
 * Intended to be overridden by derived CullFaceAttrib types to return a
 * unique number indicating whether this CullFaceAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two CullFaceAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two CullFaceAttrib objects whose get_type()
 * functions return the same.
 */
int CullFaceAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const CullFaceAttrib *ta = (const CullFaceAttrib *)other;

  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }
  return (int)_reverse - (int)ta->_reverse;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t CullFaceAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  hash = int_hash::add_hash(hash, (int)_reverse);
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
CPT(RenderAttrib) CullFaceAttrib::
compose_impl(const RenderAttrib *other) const {
  const CullFaceAttrib *ta = (const CullFaceAttrib *)other;

  if (!_reverse && ta->_mode != M_cull_unchanged) {
    // The normal case (there is nothing funny going on): the second attrib
    // completely replaces this attrib.
    return other;
  }

  // In the more complex case, the two attribs affect each other in some way,
  // and we must generate a new attrib from the result.
  Mode mode = _mode;
  if (ta->_mode != M_cull_unchanged) {
    mode = ta->_mode;
  }
  bool reverse = (_reverse && !ta->_reverse) || (!_reverse && ta->_reverse);

  CullFaceAttrib *attrib = new CullFaceAttrib(mode, reverse);
  return return_new(attrib);
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) CullFaceAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const CullFaceAttrib *ta = (const CullFaceAttrib *)other;

  // The invert case is the same as the normal case, except that the meaning
  // of _reverse is inverted.  See compose_impl(), above.

  if (_reverse && ta->_mode != M_cull_unchanged) {
    return other;
  }

  Mode mode = _mode;
  if (ta->_mode != M_cull_unchanged) {
    mode = ta->_mode;
  }
  bool reverse = (!_reverse && !ta->_reverse) || (_reverse && ta->_reverse);

  CullFaceAttrib *attrib = new CullFaceAttrib(mode, reverse);
  return return_new(attrib);
}

/**
 * Tells the BamReader how to create objects of type CullFaceAttrib.
 */
void CullFaceAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CullFaceAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  dg.add_bool(_reverse);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CullFaceAttrib is encountered in the Bam file.  It should create the
 * CullFaceAttrib and extract its information from the file.
 */
TypedWritable *CullFaceAttrib::
make_from_bam(const FactoryParams &params) {
  CullFaceAttrib *attrib = new CullFaceAttrib(M_cull_none, false);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CullFaceAttrib.
 */
void CullFaceAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
  _reverse = scan.get_bool();
}
