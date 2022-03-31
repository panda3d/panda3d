/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorBlendAttrib.cxx
 * @author drose
 * @date 2002-03-29
 */

#include "colorBlendAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

using std::ostream;

TypeHandle ColorBlendAttrib::_type_handle;
int ColorBlendAttrib::_attrib_slot;

/**
 * Constructs a new ColorBlendAttrib object that disables special-effect
 * blending, allowing normal transparency to be used instead.
 */
CPT(RenderAttrib) ColorBlendAttrib::
make_off() {
  ColorBlendAttrib *attrib = new ColorBlendAttrib;
  return return_new(attrib);
}

/**
 * Constructs a new ColorBlendAttrib object.
 *
 * @deprecated Use the three- or four-parameter constructor instead.
 */
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode, O_one, O_one,
                                                  mode, O_one, O_one,
                                                  LColor::zero());
  return return_new(attrib);
}

/**
 * Constructs a new ColorBlendAttrib object that enables special-effect
 * blending.  This supercedes transparency.  The given mode and operands are
 * used for both the RGB and alpha channels.
 */
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode,
     ColorBlendAttrib::Operand a, ColorBlendAttrib::Operand b,
     const LColor &color) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode, a, b, mode, a, b, color);
  return return_new(attrib);
}

/**
 * Constructs a new ColorBlendAttrib object that enables special-effect
 * blending.  This supercedes transparency.  This form is used to specify
 * separate blending parameters for the RGB and alpha channels.
 */
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode,
     ColorBlendAttrib::Operand a, ColorBlendAttrib::Operand b,
     ColorBlendAttrib::Mode alpha_mode,
     ColorBlendAttrib::Operand alpha_a, ColorBlendAttrib::Operand alpha_b,
     const LColor &color) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode, a, b,
                                                  alpha_mode, alpha_a, alpha_b,
                                                  color);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ColorBlendAttrib::
make_default() {
  return return_new(new ColorBlendAttrib);
}

/**
 *
 */
void ColorBlendAttrib::
output(ostream &out) const {
  out << get_type() << ":" << get_mode();

  if (get_mode() != M_none) {
    out << "(" << get_operand_a()
        << "," << get_operand_b();
    if (involves_constant_color()) {
      out << "," << get_color();
    }
    out << ")";
  }
}

/**
 * Intended to be overridden by derived ColorBlendAttrib types to return a
 * unique number indicating whether this ColorBlendAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two ColorBlendAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ColorBlendAttrib objects whose get_type()
 * functions return the same.
 */
int ColorBlendAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorBlendAttrib *ta = (const ColorBlendAttrib *)other;

  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }

  if (_a != ta->_a) {
    return (int)_a - (int)ta->_a;
  }

  if (_b != ta->_b) {
    return (int)_b - (int)ta->_b;
  }

  return _color.compare_to(ta->_color);
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ColorBlendAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  hash = int_hash::add_hash(hash, (int)_a);
  hash = int_hash::add_hash(hash, (int)_b);
  hash = _color.add_hash(hash);

  return hash;
}

/**
 * Tells the BamReader how to create objects of type ColorBlendAttrib.
 */
void ColorBlendAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ColorBlendAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint8(_mode);
  dg.add_uint8(_a);
  dg.add_uint8(_b);

  if (manager->get_file_minor_ver() >= 42) {
    dg.add_uint8(_alpha_mode);
    dg.add_uint8(_alpha_a);
    dg.add_uint8(_alpha_b);
  }

  _color.write_datagram(dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ColorBlendAttrib is encountered in the Bam file.  It should create the
 * ColorBlendAttrib and extract its information from the file.
 */
TypedWritable *ColorBlendAttrib::
make_from_bam(const FactoryParams &params) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ColorBlendAttrib.
 */
void ColorBlendAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_uint8();
  _a = (Operand)scan.get_uint8();
  _b = (Operand)scan.get_uint8();

  if (manager->get_file_minor_ver() >= 42) {
    _alpha_mode = (Mode)scan.get_uint8();
    _alpha_a = (Operand)scan.get_uint8();
    _alpha_b = (Operand)scan.get_uint8();
  } else {
    // Before bam 6.42, these were shifted by four.
    if (_a >= O_incoming1_color) {
      _a = (Operand)(_a + 4);
    }
    if (_b >= O_incoming1_color) {
      _b = (Operand)(_b + 4);
    }

    // And there was only one set of blend constants for both RGB and alpha.
    _alpha_mode = _mode;
    _alpha_a = _a;
    _alpha_b = _b;
  }

  _color.read_datagram(scan);

  _involves_constant_color =
    involves_constant_color(_a) || involves_constant_color(_alpha_a) ||
    involves_constant_color(_b) || involves_constant_color(_alpha_b);
  _involves_color_scale =
    involves_color_scale(_a) || involves_color_scale(_alpha_a) ||
    involves_color_scale(_b) || involves_color_scale(_alpha_b);
}

/**
 *
 */
ostream &
operator << (ostream &out, ColorBlendAttrib::Mode mode) {
  switch (mode) {
  case ColorBlendAttrib::M_none:
    return out << "none";

  case ColorBlendAttrib::M_add:
    return out << "add";

  case ColorBlendAttrib::M_subtract:
    return out << "subtract";

  case ColorBlendAttrib::M_inv_subtract:
    return out << "inv_subtract";

  case ColorBlendAttrib::M_min:
    return out << "min";

  case ColorBlendAttrib::M_max:
    return out << "max";
  }

  return out << "**invalid ColorBlendAttrib::Mode(" << (int)mode << ")**";
}

/**
 *
 */
ostream &
operator << (ostream &out, ColorBlendAttrib::Operand operand) {
  switch (operand) {
  case ColorBlendAttrib::O_zero:
    return out << "zero";

  case ColorBlendAttrib::O_one:
    return out << "one";

  case ColorBlendAttrib::O_incoming_color:
    return out << "incoming_color";

  case ColorBlendAttrib::O_one_minus_incoming_color:
    return out << "one_minus_incoming_color";

  case ColorBlendAttrib::O_fbuffer_color:
    return out << "fbuffer_color";

  case ColorBlendAttrib::O_one_minus_fbuffer_color:
    return out << "one_minus_fbuffer_color";

  case ColorBlendAttrib::O_incoming_alpha:
    return out << "incoming_alpha";

  case ColorBlendAttrib::O_one_minus_incoming_alpha:
    return out << "one_minus_incoming_alpha";

  case ColorBlendAttrib::O_fbuffer_alpha:
    return out << "fbuffer_alpha";

  case ColorBlendAttrib::O_one_minus_fbuffer_alpha:
    return out << "one_minus_fbuffer_alpha";

  case ColorBlendAttrib::O_constant_color:
    return out << "constant_color";

  case ColorBlendAttrib::O_one_minus_constant_color:
    return out << "one_minus_constant_color";

  case ColorBlendAttrib::O_constant_alpha:
    return out << "constant_alpha";

  case ColorBlendAttrib::O_one_minus_constant_alpha:
    return out << "one_minus_constant_alpha";

  case ColorBlendAttrib::O_incoming_color_saturate:
    return out << "incoming_color_saturate";

  case ColorBlendAttrib::O_color_scale:
    return out << "color_scale";

  case ColorBlendAttrib::O_one_minus_color_scale:
    return out << "one_minus_color_scale";

  case ColorBlendAttrib::O_alpha_scale:
    return out << "alpha_scale";

  case ColorBlendAttrib::O_one_minus_alpha_scale:
    return out << "one_minus_alpha_scale";

  case ColorBlendAttrib::O_incoming1_color:
    return out << "incoming1_color";

  case ColorBlendAttrib::O_one_minus_incoming1_color:
    return out << "one_minus_incoming1_color";

  case ColorBlendAttrib::O_incoming1_alpha:
    return out << "incoming1_alpha";

  case ColorBlendAttrib::O_one_minus_incoming1_alpha:
    return out << "one_minus_incoming1_alpha";
  }

  return out << "**invalid ColorBlendAttrib::Operand(" << (int)operand << ")**";
}
