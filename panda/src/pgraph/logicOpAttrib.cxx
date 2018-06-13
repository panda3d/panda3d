/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file logicOpAttrib.I
 * @author rdb
 * @date 2016-03-24
 */

#include "logicOpAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LogicOpAttrib::_type_handle;
int LogicOpAttrib::_attrib_slot;

/**
 * Constructs a new LogicOpAttrib object that disables special-effect
 * blending, allowing normal transparency to be used instead.
 */
CPT(RenderAttrib) LogicOpAttrib::
make_off() {
  return RenderAttribRegistry::quick_get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 * Constructs a new LogicOpAttrib object with the given logic operation.
 */
CPT(RenderAttrib) LogicOpAttrib::
make(LogicOpAttrib::Operation op) {
  LogicOpAttrib *attrib = new LogicOpAttrib(op);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) LogicOpAttrib::
make_default() {
  return RenderAttribRegistry::quick_get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 *
 */
void LogicOpAttrib::
output(std::ostream &out) const {
  out << get_type() << ":" << get_operation();
}

/**
 * Intended to be overridden by derived LogicOpAttrib types to return a
 * unique number indicating whether this LogicOpAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two LogicOpAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two LogicOpAttrib objects whose get_type()
 * functions return the same.
 */
int LogicOpAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LogicOpAttrib *la = (const LogicOpAttrib *)other;
  return (int)_op - (int)la->_op;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t LogicOpAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_op);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type LogicOpAttrib.
 */
void LogicOpAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LogicOpAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint8(_op);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LogicOpAttrib is encountered in the Bam file.  It should create the
 * LogicOpAttrib and extract its information from the file.
 */
TypedWritable *LogicOpAttrib::
make_from_bam(const FactoryParams &params) {
  LogicOpAttrib *attrib = new LogicOpAttrib(O_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LogicOpAttrib.
 */
void LogicOpAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _op = (Operation)scan.get_uint8();
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, LogicOpAttrib::Operation op) {
  switch (op) {
  case LogicOpAttrib::O_none:
    return out << "none";

  case LogicOpAttrib::O_clear:
    return out << "clear";

  case LogicOpAttrib::O_and:
    return out << "and";

  case LogicOpAttrib::O_and_reverse:
    return out << "and_reverse";

  case LogicOpAttrib::O_copy:
    return out << "copy";

  case LogicOpAttrib::O_and_inverted:
    return out << "and_inverted";

  case LogicOpAttrib::O_noop:
    return out << "noop";

  case LogicOpAttrib::O_xor:
    return out << "xor";

  case LogicOpAttrib::O_or:
    return out << "or";

  case LogicOpAttrib::O_nor:
    return out << "nor";

  case LogicOpAttrib::O_equivalent:
    return out << "equivalent";

  case LogicOpAttrib::O_invert:
    return out << "invert";

  case LogicOpAttrib::O_or_reverse:
    return out << "or_reverse";

  case LogicOpAttrib::O_copy_inverted:
    return out << "copy_inverted";

  case LogicOpAttrib::O_or_inverted:
    return out << "or_inverted";

  case LogicOpAttrib::O_nand:
    return out << "nand";

  case LogicOpAttrib::O_set:
    return out << "set";
  }

  return out << "**invalid LogicOpAttrib::Operation(" << (int)op << ")**";
}
