// Filename: compassEffect.cxx
// Created by:  drose (16Jul02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "compassEffect.h"
#include "config_pgraph.h"
#include "nodePath.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CompassEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::make
//       Access: Published, Static
//  Description: Constructs a new CompassEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) CompassEffect::
make(const NodePath &reference) {
  CompassEffect *effect = new CompassEffect;
  effect->_reference = reference;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of RenderEffect by calling the
//               xform() method, false otherwise.
////////////////////////////////////////////////////////////////////
bool CompassEffect::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CompassEffect::
output(ostream &out) const {
  if (!_reference.is_empty()) {
    RenderEffect::output(out);
  } else {
    out << get_type() << ":";
    if (!_reference.is_empty()) {
      out << " reference " << _reference;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::do_compass
//       Access: Public
//  Description: Computes the appropriate transform to rotate the node
//               according to the reference node, or to the root
//               transform if there is no reference node.
////////////////////////////////////////////////////////////////////
CPT(TransformState) CompassEffect::
do_compass(const TransformState *net_transform,
           const TransformState *node_transform) const {
  if (!net_transform->has_components() || !node_transform->has_quat()) {
    // If we don't have a decomposable transform, we can't do anything here.
    pgraph_cat.warning()
      << "CompassEffect unable to adjust non-decomposable transform\n";
    return TransformState::make_identity();
  }

  // Compute just the rotation part of the transform we want.
  CPT(TransformState) want_rot = TransformState::make_identity();
  if (!_reference.is_empty()) {
    CPT(TransformState) rel_transform = _reference.get_net_transform();
    if (!rel_transform->has_quat()) {
      pgraph_cat.warning()
        << "CompassEffect unable to reference non-decomposable transform\n";
    } else {
      want_rot = TransformState::make_quat(rel_transform->get_quat());
    }
  }

  want_rot = 
    want_rot->compose(TransformState::make_quat(node_transform->get_quat()));

  // Now compute the net transform we want to achieve.  This is the
  // same as the net transform we were given, except the rotation
  // component is replaced by our desired rotation.
  CPT(TransformState) want_transform = 
    TransformState::make_pos_quat_scale(net_transform->get_pos(),
                                        want_rot->get_quat(),
                                        net_transform->get_scale());

  // Now compute the transform that will convert net_transform to
  // want_transform.  This is inv(net_transform) * want_transform.
  return net_transform->invert_compose(want_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CompassEffect
//               types to return a unique number indicating whether
//               this CompassEffect is equivalent to the other one.
//
//               This should return 0 if the two CompassEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two CompassEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int CompassEffect::
compare_to_impl(const RenderEffect *other) const {
  const CompassEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  int compare = _reference.compare_to(ta->_reference);
  if (compare != 0) {
    return compare;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CompassEffect
//               types to specify what the default property for a
//               CompassEffect of this type should be.
//
//               This should return a newly-allocated CompassEffect of
//               the same type that corresponds to whatever the
//               standard default for this kind of CompassEffect is.
////////////////////////////////////////////////////////////////////
RenderEffect *CompassEffect::
make_default_impl() const {
  return new CompassEffect;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CompassEffect.
////////////////////////////////////////////////////////////////////
void CompassEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CompassEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CompassEffect is encountered
//               in the Bam file.  It should create the CompassEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CompassEffect::
make_from_bam(const FactoryParams &params) {
  CompassEffect *effect = new CompassEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CompassEffect.
////////////////////////////////////////////////////////////////////
void CompassEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);
}
