/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightRampAttrib.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "lightRampAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightRampAttrib::_type_handle;
int LightRampAttrib::_attrib_slot;
CPT(RenderAttrib) LightRampAttrib::_default;

/**
 * Constructs a new LightRampAttrib object.  This is the standard OpenGL
 * lighting ramp, which clamps the final light total to the 0-1 range.
 */
CPT(RenderAttrib) LightRampAttrib::
make_default() {
  if (_default == nullptr) {
    LightRampAttrib *attrib = new LightRampAttrib();
    _default = return_new(attrib);
  }
  return _default;
}

/**
 * Constructs a new LightRampAttrib object.  This differs from the usual
 * OpenGL lighting model in that it does not clamp the final lighting total to
 * (0,1).
 */
CPT(RenderAttrib) LightRampAttrib::
make_identity() {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_identity;
  return return_new(attrib);
}

/**
 * Constructs a new LightRampAttrib object.  This causes the luminance of the
 * diffuse lighting contribution to be quantized using a single threshold:
 *
 * @code
 * if (original_luminance > threshold0) {
 *   luminance = level0;
 * } else {
 *   luminance = 0.0;
 * }
 * @endcode
 */
CPT(RenderAttrib) LightRampAttrib::
make_single_threshold(PN_stdfloat thresh0, PN_stdfloat val0) {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_single_threshold;
  attrib->_threshold[0] = thresh0;
  attrib->_level[0] = val0;
  return return_new(attrib);
}

/**
 * Constructs a new LightRampAttrib object.  This causes the luminance of the
 * diffuse lighting contribution to be quantized using two thresholds:
 *
 * @code
 * if (original_luminance > threshold1) {
 *   luminance = level1;
 * } else if (original_luminance > threshold0) {
 *   luminance = level0;
 * } else {
 *   luminance = 0.0;
 * }
 * @endcode
 */
CPT(RenderAttrib) LightRampAttrib::
make_double_threshold(PN_stdfloat thresh0, PN_stdfloat val0, PN_stdfloat thresh1, PN_stdfloat val1) {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_double_threshold;
  attrib->_threshold[0] = thresh0;
  attrib->_level[0] = val0;
  attrib->_threshold[1] = thresh1;
  attrib->_level[1] = val1;
  return return_new(attrib);
}

/**
 * Constructs a new LightRampAttrib object.  This causes an HDR tone mapping
 * operation to be applied.
 *
 * Normally, brightness values greater than 1 cannot be distinguished from
 * each other, causing very brightly lit objects to wash out white and all
 * detail to be erased.  HDR tone mapping remaps brightness values in the
 * range 0-infinity into the range (0,1), making it possible to distinguish
 * detail in scenes whose brightness exceeds 1.
 *
 * However, the monitor has finite contrast.  Normally, all of that contrast
 * is used to represent brightnesses in the range 0-1.  The HDR0 tone mapping
 * operator 'steals' one quarter of that contrast to represent brightnesses in
 * the range 1-infinity.
 *
 * @code
 * FINAL_RGB = (RGB^3 + RGB^2 + RGB) / (RGB^3 + RGB^2 + RGB + 1)
 * @endcode
 */
CPT(RenderAttrib) LightRampAttrib::
make_hdr0() {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_hdr0;
  return return_new(attrib);
}

/**
 * Constructs a new LightRampAttrib object.  This causes an HDR tone mapping
 * operation to be applied.
 *
 * Normally, brightness values greater than 1 cannot be distinguished from
 * each other, causing very brightly lit objects to wash out white and all
 * detail to be erased.  HDR tone mapping remaps brightness values in the
 * range 0-infinity into the range (0,1), making it possible to distinguish
 * detail in scenes whose brightness exceeds 1.
 *
 * However, the monitor has finite contrast.  Normally, all of that contrast
 * is used to represent brightnesses in the range 0-1.  The HDR1 tone mapping
 * operator 'steals' one third of that contrast to represent brightnesses in
 * the range 1-infinity.
 *
 * @code
 * FINAL_RGB = (RGB^2 + RGB) / (RGB^2 + RGB + 1)
 * @endcode
 */
CPT(RenderAttrib) LightRampAttrib::
make_hdr1() {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_hdr1;
  return return_new(attrib);
}

/**
 * Constructs a new LightRampAttrib object.  This causes an HDR tone mapping
 * operation to be applied.
 *
 * Normally, brightness values greater than 1 cannot be distinguished from
 * each other, causing very brightly lit objects to wash out white and all
 * detail to be erased.  HDR tone mapping remaps brightness values in the
 * range 0-infinity into the range (0,1), making it possible to distinguish
 * detail in scenes whose brightness exceeds 1.
 *
 * However, the monitor has finite contrast.  Normally, all of that contrast
 * is used to represent brightnesses in the range 0-1.  The HDR2 tone mapping
 * operator 'steals' one half of that contrast to represent brightnesses in
 * the range 1-infinity.
 *
 * @code
 * FINAL_RGB = (RGB) / (RGB + 1)
 * @endcode
 */
CPT(RenderAttrib) LightRampAttrib::
make_hdr2() {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_hdr2;
  return return_new(attrib);
}

/**
 *
 */
void LightRampAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (_mode) {
  case LRT_default:
    out << "default()";
    break;
  case LRT_identity:
    out << "identity()";
    break;
  case LRT_single_threshold:
    out << "single_threshold(" << _level[0] << "," << _level[1] << "," << _threshold[0] << ")";
    break;
  case LRT_double_threshold:
    out << "double_threshold(" << _level[0] << "," << _level[1] << "," << _threshold[0] << "," << _threshold[1] << ")";
    break;
  case LRT_hdr0:
    out << "hdr0()";
    break;
  case LRT_hdr1:
    out << "hdr1()";
    break;
  case LRT_hdr2:
    out << "hdr2()";
    break;
  }
}

/**
 * Intended to be overridden by derived LightRampAttrib types to return a
 * unique number indicating whether this LightRampAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two LightRampAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two LightRampAttrib objects whose get_type()
 * functions return the same.
 */
int LightRampAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LightRampAttrib *ta = (const LightRampAttrib *)other;

  int compare_result = ((int)_mode - (int)ta->_mode) ;
  if (compare_result != 0) {
    return compare_result;
  }
  for (int i = 0; i < 2; i++) {
    if (_level[i] != ta->_level[i]) {
      return (_level[i] < ta->_level[i]) ? -1 : 1;
    }
  }
  for (int i = 0; i < 2; i++) {
    if (_threshold[i] != ta->_threshold[i]) {
      return (_threshold[i] < ta->_threshold[i]) ? -1 : 1;
    }
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t LightRampAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  float_hash fh;
  for (int i = 0; i < 2; i++) {
    hash = fh.add_hash(hash, _level[i]);
    hash = fh.add_hash(hash, _threshold[i]);
  }
  return hash;
}

/**
 * Tells the BamReader how to create objects of type LightRampAttrib.
 */
void LightRampAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LightRampAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  for (int i=0; i<2; i++) {
    dg.add_stdfloat(_level[i]);
  }
  for (int i=0; i<2; i++) {
    dg.add_stdfloat(_threshold[i]);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LightRampAttrib is encountered in the Bam file.  It should create the
 * LightRampAttrib and extract its information from the file.
 */
TypedWritable *LightRampAttrib::
make_from_bam(const FactoryParams &params) {
  LightRampAttrib *attrib = new LightRampAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LightRampAttrib.
 */
void LightRampAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (LightRampMode)scan.get_int8();
  for (int i=0; i<2; i++) {
    _level[i] = scan.get_stdfloat();
  }
  for (int i=0; i<2; i++) {
    _threshold[i] = scan.get_stdfloat();
  }
}
