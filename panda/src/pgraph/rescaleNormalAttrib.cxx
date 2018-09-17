/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rescaleNormalAttrib.cxx
 * @author drose
 * @date 2004-12-30
 */

#include "rescaleNormalAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "string_utils.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "configVariableEnum.h"
#include "config_pgraph.h"

using std::istream;
using std::ostream;
using std::string;

TypeHandle RescaleNormalAttrib::_type_handle;
int RescaleNormalAttrib::_attrib_slot;
CPT(RenderAttrib) RescaleNormalAttrib::_attribs[RescaleNormalAttrib::M_auto + 1];

/**
 * Constructs a new RescaleNormalAttrib object that specifies whether to
 * rescale normals to compensate for transform scales or incorrectly defined
 * normals.
 */
CPT(RenderAttrib) RescaleNormalAttrib::
make(RescaleNormalAttrib::Mode mode) {
  if (_attribs[mode].is_null()) {
    // Don't bother with return_new, since this is the only way a
    // RescaleNormalAttrib can be made anyway.
    _attribs[mode] = new RescaleNormalAttrib(mode);
  }
  return _attribs[mode];
}

/**
 *
 */
void RescaleNormalAttrib::
output(ostream &out) const {
  out << get_type() << ":" << get_mode();
}

/**
 * Intended to be overridden by derived RescaleNormalAttrib types to return a
 * unique number indicating whether this RescaleNormalAttrib is equivalent to
 * the other one.
 *
 * This should return 0 if the two RescaleNormalAttrib objects are equivalent,
 * a number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two RescaleNormalAttrib objects whose
 * get_type() functions return the same.
 */
int RescaleNormalAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const RescaleNormalAttrib *ta = (const RescaleNormalAttrib *)other;
  return (int)_mode - (int)ta->_mode;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t RescaleNormalAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  return hash;
}

/**
 * Tells the BamReader how to create objects of type RescaleNormalAttrib.
 */
void RescaleNormalAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RescaleNormalAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type RescaleNormalAttrib is encountered in the Bam file.  It should create
 * the RescaleNormalAttrib and extract its information from the file.
 */
TypedWritable *RescaleNormalAttrib::
make_from_bam(const FactoryParams &params) {
  RescaleNormalAttrib *attrib = new RescaleNormalAttrib(M_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RescaleNormalAttrib.
 */
void RescaleNormalAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}

/**
 *
 */
void RescaleNormalAttrib::
init_type() {
  RenderAttrib::init_type();
  register_type(_type_handle, "RescaleNormalAttrib",
                RenderAttrib::get_class_type());

  // This is defined here, since we have otherwise no guarantee that the
  // config var has already been constructed by the time we call init_type()
  // at static init time.
  static ConfigVariableEnum<RescaleNormalAttrib::Mode> rescale_normals
  ("rescale-normals", RescaleNormalAttrib::M_auto,
   PRC_DESC("Specifies the kind of RescaleNormalAttrib that should be "
            "created for the top of the scene graph.  This can automatically "
            "ensure that your lighting normals are unit-length, which may be "
            "particularly necessary in the presence of scales in the scene "
            "graph.  Turning it off ('none') may produce a small performance "
            "benefit."));

  Mode mode = rescale_normals;
  RescaleNormalAttrib *attrib = new RescaleNormalAttrib(mode);
  _attrib_slot = register_slot(_type_handle, 100, attrib);
  _attribs[mode] = attrib;
}

/**
 *
 */
ostream &
operator << (ostream &out, RescaleNormalAttrib::Mode mode) {
  switch (mode) {
  case RescaleNormalAttrib::M_none:
    return out << "none";

  case RescaleNormalAttrib::M_rescale:
    return out << "rescale";

  case RescaleNormalAttrib::M_normalize:
    return out << "normalize";

  case RescaleNormalAttrib::M_auto:
    return out << "auto";
  }

  return out << "(**invalid RescaleNormalAttrib::Mode(" << (int)mode << ")**)";
}

/**
 *
 */
istream &
operator >> (istream &in, RescaleNormalAttrib::Mode &mode) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0) {
    mode = RescaleNormalAttrib::M_none;

  } else if (cmp_nocase(word, "rescale") == 0) {
    mode = RescaleNormalAttrib::M_rescale;

  } else if (cmp_nocase(word, "normalize") == 0) {
    mode = RescaleNormalAttrib::M_normalize;

  } else if (cmp_nocase(word, "auto") == 0) {
    mode = RescaleNormalAttrib::M_auto;

  } else {
    pgraph_cat.error()
      << "Invalid RescaleNormalAttrib::Mode value: " << word << "\n";
    mode = RescaleNormalAttrib::M_none;
  }

  return in;
}
