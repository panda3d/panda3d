/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorAttrib.cxx
 * @author drose
 * @date 2002-02-22
 */

#include "colorAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ColorAttrib::_type_handle;
int ColorAttrib::_attrib_slot;
CPT(RenderAttrib) ColorAttrib::_off;
CPT(RenderAttrib) ColorAttrib::_vertex;

/**
 * Constructs a new ColorAttrib object that indicates geometry should be
 * rendered according to its own vertex color.
 */
CPT(RenderAttrib) ColorAttrib::
make_vertex() {
  if (_vertex != nullptr) {
    return _vertex;
  }
  ColorAttrib *attrib = new ColorAttrib(T_vertex, LColor::zero());
  _vertex = return_new(attrib);
  return _vertex;
}

/**
 * Constructs a new ColorAttrib object that indicates geometry should be
 * rendered in the indicated color.
 */
CPT(RenderAttrib) ColorAttrib::
make_flat(const LColor &color) {
  ColorAttrib *attrib = new ColorAttrib(T_flat, color);
  return return_new(attrib);
}

/**
 * Constructs a new ColorAttrib object that indicates geometry should be
 * rendered in white.
 */
CPT(RenderAttrib) ColorAttrib::
make_off() {
  if (_off != nullptr) {
    return _off;
  }
  ColorAttrib *attrib = new ColorAttrib(T_off, LColor(1.0f, 1.0f, 1.0f, 1.0f));
  _off = return_new(attrib);
  return _off;
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ColorAttrib::
make_default() {
  return make_vertex();
}

/**
 *
 */
void ColorAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_color_type()) {
  case T_vertex:
    out << "vertex";
    break;

  case T_flat:
    out << "(" << get_color() << ")";
    break;

  case T_off:
    out << "off";
    break;
  }
}

/**
 * Intended to be overridden by derived ColorAttrib types to return a unique
 * number indicating whether this ColorAttrib is equivalent to the other one.
 *
 * This should return 0 if the two ColorAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ColorAttrib objects whose get_type()
 * functions return the same.
 */
int ColorAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorAttrib *ta = (const ColorAttrib *)other;

  if (_type != ta->_type) {
    return (int)_type - (int)ta->_type;
  }
  if (_type == T_flat) {
    return _color.compare_to(ta->_color);
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ColorAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_type);
  if (_type == T_flat) {
    hash = _color.add_hash(hash);
  }
  return hash;
}

/**
 * Quantizes the flat color to the nearest multiple of 1024, just to prevent
 * runaway accumulation of only slightly-different ColorAttribs.
 */
void ColorAttrib::
quantize_color() {
  switch (_type) {
  case T_flat:
    _color[0] = cfloor(_color[0] * 1024.0f + 0.5f) / 1024.0f;
    _color[1] = cfloor(_color[1] * 1024.0f + 0.5f) / 1024.0f;
    _color[2] = cfloor(_color[2] * 1024.0f + 0.5f) / 1024.0f;
    _color[3] = cfloor(_color[3] * 1024.0f + 0.5f) / 1024.0f;
    break;

  case T_off:
    _color.set(1.0f, 1.0f, 1.0f, 1.0f);
    break;

  case T_vertex:
    _color.set(0.0f, 0.0f, 0.0f, 0.0f);
    break;
  }
}

/**
 * Tells the BamReader how to create objects of type ColorAttrib.
 */
void ColorAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ColorAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_type);
  _color.write_datagram(dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ColorAttrib is encountered in the Bam file.  It should create the
 * ColorAttrib and extract its information from the file.
 */
TypedWritable *ColorAttrib::
make_from_bam(const FactoryParams &params) {
  ColorAttrib *attrib = new ColorAttrib(T_off, LColor::zero());
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ColorAttrib.
 */
void ColorAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _type = (Type)scan.get_int8();
  _color.read_datagram(scan);
  quantize_color();
}
