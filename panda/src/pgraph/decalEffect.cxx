// Filename: decalEffect.cxx
// Created by:  drose (14Mar02)
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

#include "decalEffect.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DecalEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::make
//       Access: Published, Static
//  Description: Constructs a new DecalEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) DecalEffect::
make() {
  DecalEffect *effect = new DecalEffect;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if this kind of effect can safely be
//               combined with sibling nodes that share the exact same
//               effect, or false if this is not a good idea.
////////////////////////////////////////////////////////////////////
bool DecalEffect::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DecalEffect
//               types to return a unique number indicating whether
//               this DecalEffect is equivalent to the other one.
//
//               This should return 0 if the two DecalEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two DecalEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int DecalEffect::
compare_to_impl(const RenderEffect *other) const {
  // All DecalEffects are equivalent--there are no properties to
  // store.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               DecalEffect.
////////////////////////////////////////////////////////////////////
void DecalEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DecalEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type DecalEffect is encountered
//               in the Bam file.  It should create the DecalEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *DecalEffect::
make_from_bam(const FactoryParams &params) {
  DecalEffect *effect = new DecalEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new DecalEffect.
////////////////////////////////////////////////////////////////////
void DecalEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);
}
