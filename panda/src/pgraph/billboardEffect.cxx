// Filename: billboardEffect.cxx
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "billboardEffect.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "nodePath.h"
#include "look_at.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle BillboardEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make
//       Access: Published, Static
//  Description: Constructs a new BillboardEffect object with the
//               indicated properties.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) BillboardEffect::
make(const LVector3f &up_vector, bool eye_relative,
     bool axial_rotate, float offset, const NodePath &look_at,
     const LPoint3f &look_at_point) {
  BillboardEffect *effect = new BillboardEffect;
  effect->_up_vector = up_vector;
  effect->_eye_relative = eye_relative;
  effect->_axial_rotate = axial_rotate;
  effect->_offset = offset;
  effect->_look_at = look_at;
  effect->_look_at_point = look_at_point;
  effect->_off = false;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of RenderEffect by calling the
//               xform() method, false otherwise.
////////////////////////////////////////////////////////////////////
bool BillboardEffect::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BillboardEffect::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    if (_axial_rotate) {
      out << "(axis";
    } else {
      out << "(point";
    }
    if (!_up_vector.almost_equal(LVector3f::up())) {
      out << " up " << _up_vector;
    }
    if (_eye_relative) {
      out << " eye";
    }
    if (_offset != 0.0f) {
      out << " offset " << _offset;
    }
    if (!_look_at.is_empty()) {
      out << " look at " << _look_at;
    }
    if (!_look_at_point.almost_equal(LPoint3f(0.0f, 0.0f, 0.0f))) {
      out << " look at point " << _look_at_point;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this effect during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool BillboardEffect::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               At the time this function is called, the current
//               node's transform and state have not yet been applied
//               to the net_transform and net_state.  This callback
//               may modify the node_transform and node_state to apply
//               an effective change to the render state at this
//               level.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &) const {
  CPT(TransformState) net_transform = data._net_transform;
  if (net_transform->is_singular()) {
    // If we're under a singular transform, never mind.
    return;
  }

  CPT(TransformState) camera_transform = trav->get_camera_transform();

  // Determine the relative transform to our camera (or other look_at
  // coordinate space).
  if (!_look_at.is_empty()) {
    camera_transform = _look_at.get_net_transform();
  }

  CPT(TransformState) billboard_transform =
    compute_billboard(net_transform, camera_transform);

  node_transform = billboard_transform->compose(node_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::has_net_transform
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if net_transform() has been defined, and
//               therefore the RenderEffect has some effect on the
//               node's apparent net transform.
////////////////////////////////////////////////////////////////////
bool BillboardEffect::
has_net_transform() const {
  // A BillboardEffect can only affect the net transform when it is to
  // a particular node.  A billboard to a camera is camera-dependent,
  // of course, so it has no effect in the absence of any particular
  // camera viewing it.
  return !_look_at.is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::net_transform
//       Access: Public, Virtual
//  Description: Given the node's parent's net transform, compute its
//               parent's new net transform after application of the
//               RenderEffect.  Presumably this interposes some
//               special transform derived from the RenderEffect.
//               This may only be called if has_net_transform(),
//               above, has been defined to return true.
////////////////////////////////////////////////////////////////////
CPT(TransformState) BillboardEffect::
net_transform(const TransformState *orig_net_transform) const {
  // A BillboardEffect can only affect the net transform when it is to
  // a particular node.  A billboard to a camera is camera-dependent,
  // of course, so it has no effect in the absence of any particular
  // camera viewing it.
  if (_look_at.is_empty()) {
    return orig_net_transform;
  }

  CPT(TransformState) camera_transform = _look_at.get_net_transform();

  CPT(TransformState) billboard_transform =
    compute_billboard(orig_net_transform, camera_transform);

  return orig_net_transform->compose(billboard_transform);
}


////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived BillboardEffect
//               types to return a unique number indicating whether
//               this BillboardEffect is equivalent to the other one.
//
//               This should return 0 if the two BillboardEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two BillboardEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int BillboardEffect::
compare_to_impl(const RenderEffect *other) const {
  const BillboardEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_axial_rotate != ta->_axial_rotate) {
    return _axial_rotate - ta->_axial_rotate;
  }
  if (_eye_relative != ta->_eye_relative) {
    return _eye_relative - ta->_eye_relative;
  }
  if (_offset != ta->_offset) {
    return _offset < ta->_offset ? -1 : 1;
  }
  int compare = _up_vector.compare_to(ta->_up_vector);
  if (compare != 0) {
    return compare;
  }
  compare = _look_at.compare_to(ta->_look_at);
  if (compare != 0) {
    return compare;
  }
  compare = _look_at_point.compare_to(ta->_look_at_point);
  if (compare != 0) {
    return compare;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::compute_billboard
//       Access: Private
//  Description: Computes the billboard operation given the parent's
//               net transform and the camera transform.
////////////////////////////////////////////////////////////////////
CPT(TransformState) BillboardEffect::
compute_billboard(const TransformState *net_transform, 
                  const TransformState *camera_transform) const {
  CPT(TransformState) rel_transform =
    net_transform->invert_compose(camera_transform);
  const LMatrix4f &rel_mat = rel_transform->get_mat();

  // Determine the look_at point in the camera space.
  LVector3f camera_pos, up;

  // If this is an eye-relative Billboard, then (a) the up vector is
  // relative to the camera, not to the world, and (b) the look
  // direction is towards the plane that contains the camera,
  // perpendicular to the forward direction, not directly to the
  // camera.

  if (_eye_relative) {
    up = _up_vector * rel_mat;
    camera_pos = LVector3f::forward() * rel_mat;

  } else {
    up = _up_vector;
    camera_pos = -(_look_at_point * rel_mat);
  }

  // Now determine the rotation matrix for the Billboard.
  LMatrix4f rotate;
  if (_axial_rotate) {
    heads_up(rotate, camera_pos, up);
  } else {
    look_at(rotate, camera_pos, up);
  }

  // Also slide the billboard geometry towards the camera according to
  // the offset factor.
  if (_offset != 0.0f) {
    LVector3f translate(rel_mat(3, 0), rel_mat(3, 1), rel_mat(3, 2));
    translate.normalize();
    translate *= _offset;
    rotate.set_row(3, translate);
  }

  return TransformState::make_mat(rotate);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BillboardEffect.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  dg.add_bool(_off);
  _up_vector.write_datagram(dg);
  dg.add_bool(_eye_relative);
  dg.add_bool(_axial_rotate);
  dg.add_float32(_offset);
  _look_at_point.write_datagram(dg);

  // *** We don't write out the _look_at NodePath right now.  Maybe
  // we should.
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BillboardEffect is encountered
//               in the Bam file.  It should create the BillboardEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BillboardEffect::
make_from_bam(const FactoryParams &params) {
  BillboardEffect *effect = new BillboardEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BillboardEffect.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  _off = scan.get_bool();
  _up_vector.read_datagram(scan);
  _eye_relative = scan.get_bool();
  _axial_rotate = scan.get_bool();
  _offset = scan.get_float32();
  _look_at_point.read_datagram(scan);
}
