/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file billboardEffect.cxx
 * @author drose
 * @date 2002-03-14
 */

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

/**
 * Constructs a new BillboardEffect object with the indicated properties.
 */
CPT(RenderEffect) BillboardEffect::
make(const LVector3 &up_vector, bool eye_relative,
     bool axial_rotate, PN_stdfloat offset, const NodePath &look_at,
     const LPoint3 &look_at_point, bool fixed_depth) {
  BillboardEffect *effect = new BillboardEffect;
  effect->_up_vector = up_vector;
  effect->_eye_relative = eye_relative;
  effect->_axial_rotate = axial_rotate;
  effect->_offset = offset;
  effect->_look_at = look_at;
  effect->_look_at_point = look_at_point;
  effect->_fixed_depth = fixed_depth;
  effect->_off = false;
  return return_new(effect);
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * RenderEffect by calling the xform() method, false otherwise.
 */
bool BillboardEffect::
safe_to_transform() const {
  return false;
}

/**
 * Preprocesses the accumulated transform that is about to be applied to (or
 * through) this node due to a flatten operation.  The returned value will be
 * used instead.
 */
CPT(TransformState) BillboardEffect::
prepare_flatten_transform(const TransformState *net_transform) const {
  // We don't want any flatten operation to rotate the billboarded node, since
  // the billboard effect should eat any rotation that comes in from above.
  return net_transform->set_hpr(LVecBase3(0, 0, 0));
}

/**
 *
 */
void BillboardEffect::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    if (_axial_rotate) {
      out << "(axis";
    } else {
      out << "(point";
    }
    if (!_up_vector.almost_equal(LVector3::up())) {
      out << " up " << _up_vector;
    }
    if (_eye_relative) {
      out << " eye";
    }
    if (_fixed_depth) {
      out << " depth " << -_offset;
    } else if (_offset != 0.0f) {
      out << " offset " << _offset;
    }
    if (!_look_at.is_empty()) {
      out << " look at " << _look_at;
    }
    if (!_look_at_point.almost_equal(LPoint3(0.0f, 0.0f, 0.0f))) {
      out << " look at point " << _look_at_point;
    }
    out << ")";
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool BillboardEffect::
has_cull_callback() const {
  return true;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.  This may include additional manipulation of render
 * state or additional visible/invisible decisions, or any other arbitrary
 * operation.
 *
 * At the time this function is called, the current node's transform and state
 * have not yet been applied to the net_transform and net_state.  This
 * callback may modify the node_transform and node_state to apply an effective
 * change to the render state at this level.
 */
void BillboardEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &) const {
  CPT(TransformState) modelview_transform = data.get_modelview_transform(trav);
  if (modelview_transform->is_singular()) {
    // If we're under a singular transform, never mind.
    return;
  }

  // Since the "modelview" transform from the cull traverser already includes
  // the inverse camera transform, the camera transform is identity.
  CPT(TransformState) camera_transform = TransformState::make_identity();

  // But if we're rotating to face something other than the camera, we have to
  // compute the "camera" transform to compensate for that.
  if (!_look_at.is_empty()) {
    camera_transform = trav->get_camera_transform()->invert_compose(_look_at.get_net_transform());
  }

  compute_billboard(node_transform, modelview_transform, camera_transform);
}

/**
 * Should be overridden by derived classes to return true if
 * adjust_transform() has been defined, and therefore the RenderEffect has
 * some effect on the node's apparent local and net transforms.
 */
bool BillboardEffect::
has_adjust_transform() const {
  // A BillboardEffect can only affect the net transform when it is to a
  // particular node.  A billboard to a camera is camera-dependent, of course,
  // so it has no effect in the absence of any particular camera viewing it.
  return !_look_at.is_empty();
}

/**
 * Performs some operation on the node's apparent net and/or local transforms.
 * This will only be called if has_adjust_transform() is redefined to return
 * true.
 *
 * Both parameters are in/out.  The original transforms will be passed in, and
 * they may (or may not) be modified in-place by the RenderEffect.
 */
void BillboardEffect::
adjust_transform(CPT(TransformState) &net_transform,
                 CPT(TransformState) &node_transform,
                 const PandaNode *) const {
  // A BillboardEffect can only affect the net transform when it is to a
  // particular node.  A billboard to a camera is camera-dependent, of course,
  // so it has no effect in the absence of any particular camera viewing it.
  if (_look_at.is_empty()) {
    return;
  }

  CPT(TransformState) camera_transform = _look_at.get_net_transform();

  compute_billboard(node_transform, net_transform, camera_transform);
}


/**
 * Intended to be overridden by derived BillboardEffect types to return a
 * unique number indicating whether this BillboardEffect is equivalent to the
 * other one.
 *
 * This should return 0 if the two BillboardEffect objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two BillboardEffect objects whose get_type()
 * functions return the same.
 */
int BillboardEffect::
compare_to_impl(const RenderEffect *other) const {
  const BillboardEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_axial_rotate != ta->_axial_rotate) {
    return (int)_axial_rotate - (int)ta->_axial_rotate;
  }
  if (_eye_relative != ta->_eye_relative) {
    return (int)_eye_relative - (int)ta->_eye_relative;
  }
  if (_fixed_depth != ta->_fixed_depth) {
    return (int)_fixed_depth - (int)ta->_fixed_depth;
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

/**
 * Computes the billboard operation given the parent's net transform and the
 * camera transform.
 *
 * The result is applied to node_transform, which is modified in-place.
 */
void BillboardEffect::
compute_billboard(CPT(TransformState) &node_transform,
                  const TransformState *net_transform,
                  const TransformState *camera_transform) const {
  // First, extract out just the translation component of the node's local
  // transform.  This gets applied to the net transform, to compute the look-
  // at direction properly.
  CPT(TransformState) translate = TransformState::make_pos(node_transform->get_pos());

  // And then the translation gets removed from the node, but we keep its
  // rotation etc., which gets applied after the billboard operation.
  node_transform = node_transform->set_pos(LPoint3(0.0f, 0.0f, 0.0f));

  CPT(TransformState) rel_transform =
    net_transform->compose(translate)->invert_compose(camera_transform);
  if (!rel_transform->has_mat()) {
    // Never mind.
    return;
  }

  const LMatrix4 &rel_mat = rel_transform->get_mat();

  // Determine the look_at point in the camera space.
  LVector3 camera_pos, up;

  // If this is an eye-relative Billboard, then (a) the up vector is relative
  // to the camera, not to the world, and (b) the look direction is towards
  // the plane that contains the camera, perpendicular to the forward
  // direction, not directly to the camera.

  if (_eye_relative) {
    up = _up_vector * rel_mat;
    camera_pos = LVector3::forward() * rel_mat;

  } else {
    up = _up_vector;
    camera_pos = -(_look_at_point * rel_mat);
  }

  // Now determine the rotation matrix for the Billboard.
  LMatrix4 rotate;
  if (_axial_rotate) {
    heads_up(rotate, camera_pos, up);
  } else {
    look_at(rotate, camera_pos, up);
  }

  // Also slide the billboard geometry towards the camera according to the
  // offset factor.
  if (_offset != 0.0f || _fixed_depth) {
    LVector3 translate(rel_mat(3, 0), rel_mat(3, 1), rel_mat(3, 2));
    LPoint3 pos;
    if (_fixed_depth) {
      pos = translate / rel_mat(3, 3);
    } else {
      pos.fill(0.0f);
    }
    translate.normalize();
    translate *= _offset;
    rotate.set_row(3, pos + translate);
  }

  node_transform = translate->compose(TransformState::make_mat(rotate))->compose(node_transform);
}

/**
 * Tells the BamReader how to create objects of type BillboardEffect.
 */
void BillboardEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BillboardEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  dg.add_bool(_off);
  _up_vector.write_datagram(dg);
  dg.add_bool(_eye_relative);
  dg.add_bool(_axial_rotate);
  dg.add_stdfloat(_offset);
  _look_at_point.write_datagram(dg);

  if (manager->get_file_minor_ver() >= 43) {
    _look_at.write_datagram(manager, dg);
    dg.add_bool(_fixed_depth);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int BillboardEffect::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderEffect::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 43) {
    pi += _look_at.complete_pointers(p_list + pi, manager);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BillboardEffect is encountered in the Bam file.  It should create the
 * BillboardEffect and extract its information from the file.
 */
TypedWritable *BillboardEffect::
make_from_bam(const FactoryParams &params) {
  BillboardEffect *effect = new BillboardEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BillboardEffect.
 */
void BillboardEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  _off = scan.get_bool();
  _up_vector.read_datagram(scan);
  _eye_relative = scan.get_bool();
  _axial_rotate = scan.get_bool();
  _offset = scan.get_stdfloat();
  _look_at_point.read_datagram(scan);

  if (manager->get_file_minor_ver() >= 43) {
    _look_at.fillin(scan, manager);
    _fixed_depth = scan.get_bool();
  } else {
    _fixed_depth = false;
  }
}
