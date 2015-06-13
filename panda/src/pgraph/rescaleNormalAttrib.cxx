// Filename: rescaleNormalAttrib.cxx
// Created by:  drose (30Dec04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

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

TypeHandle RescaleNormalAttrib::_type_handle;
int RescaleNormalAttrib::_attrib_slot;

// This variable is defined here instead of in config_pgraph.cxx,
// because it depends on rescaleNormalAttrib.h having already been
// included.
static ConfigVariableEnum<RescaleNormalAttrib::Mode> rescale_normals
("rescale-normals", RescaleNormalAttrib::M_auto,
 PRC_DESC("Specifies the kind of RescaleNormalAttrib that should be "
          "created for the top of the scene graph.  This can automatically "
          "ensure that your lighting normals are unit-length, which may be "
          "particularly necessary in the presence of scales in the scene "
          "graph.  Turning it off ('none') may produce a small performance "
          "benefit."));

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::make
//       Access: Published, Static
//  Description: Constructs a new RescaleNormalAttrib object that
//               specifies whether to rescale normals to compensate
//               for transform scales or incorrectly defined normals.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RescaleNormalAttrib::
make(RescaleNormalAttrib::Mode mode) {
  RescaleNormalAttrib *attrib = new RescaleNormalAttrib(mode);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::make_default
//       Access: Published, Static
//  Description: Constructs a RescaleNoramlAttrib object that's
//               suitable for putting at the top of a scene graph.
//               This will contain whatever attrib was suggested by
//               the user's rescale-normals Config variable.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RescaleNormalAttrib::
make_default() {
  RescaleNormalAttrib *attrib = new RescaleNormalAttrib(rescale_normals);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RescaleNormalAttrib::
output(ostream &out) const {
  out << get_type() << ":" << get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RescaleNormalAttrib
//               types to return a unique number indicating whether
//               this RescaleNormalAttrib is equivalent to the other one.
//
//               This should return 0 if the two RescaleNormalAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two RescaleNormalAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int RescaleNormalAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const RescaleNormalAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return (int)_mode - (int)ta->_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t RescaleNormalAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::get_auto_shader_attrib_impl
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RescaleNormalAttrib::
get_auto_shader_attrib_impl(const RenderState *state) const {
  // We currently only support M_normalize in the ShaderGenerator.
  if (_mode == M_none || _mode == M_normalize) {
    return this;
  } else {
    return RescaleNormalAttrib::make(M_normalize);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RescaleNormalAttrib.
////////////////////////////////////////////////////////////////////
void RescaleNormalAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RescaleNormalAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RescaleNormalAttrib is encountered
//               in the Bam file.  It should create the RescaleNormalAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RescaleNormalAttrib::
make_from_bam(const FactoryParams &params) {
  RescaleNormalAttrib *attrib = new RescaleNormalAttrib(M_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RescaleNormalAttrib.
////////////////////////////////////////////////////////////////////
void RescaleNormalAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::Mode output operator
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::Mode input operator
//  Description:
////////////////////////////////////////////////////////////////////
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
