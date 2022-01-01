/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file instancedNode.cxx
 * @author rdb
 * @date 2019-03-10
 */

#include "instancedNode.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "cullTraverserData.h"
#include "cullPlanes.h"

TypeHandle InstancedNode::_type_handle;
TypeHandle InstancedNode::CData::_type_handle;

/**
 *
 */
InstancedNode::
InstancedNode(const std::string &name) :
  PandaNode(name)
{
  set_cull_callback();
}

/**
 *
 */
InstancedNode::
InstancedNode(const InstancedNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
  set_cull_callback();
}

/**
 *
 */
InstancedNode::
~InstancedNode() {
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *InstancedNode::
make_copy() const {
  return new InstancedNode(*this);
}

/**
 * Returns the list of instances.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
PT(InstanceList) InstancedNode::
modify_instances() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  PT(InstanceList) instances = cdata->_instances.get_write_pointer();
  mark_bounds_stale(current_thread->get_pipeline_stage(), current_thread);
  mark_bam_modified();
  return instances;
}

/**
 * Entirely replaces the list of instances with the given list.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
void InstancedNode::
set_instances(PT(InstanceList) instances) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true);
  cdata->_instances = std::move(instances);
  mark_bounds_stale(current_thread->get_pipeline_stage(), current_thread);
  mark_bam_modified();
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * PandaNode by duplicating instances (by calling dupe_for_flatten()), false
 * otherwise (for instance, a Camera cannot be safely flattened, because the
 * Camera pointer itself is meaningful).
 */
bool InstancedNode::
safe_to_flatten() const {
  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool InstancedNode::
safe_to_combine() const {
  // This can happen iff the instance list is identical; see combine_with().
  return true;
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void InstancedNode::
xform(const LMatrix4 &mat) {
}

/**
 * Collapses this node with the other node, if possible, and returns a pointer
 * to the combined node, or NULL if the two nodes cannot safely be combined.
 *
 * The return value may be this, other, or a new node altogether.
 *
 * This function is called from GraphReducer::flatten(), and need not deal
 * with children; its job is just to decide whether to collapse the two nodes
 * and what the collapsed node should look like.
 */
PandaNode *InstancedNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) && other->is_exact_type(get_class_type())) {
    InstancedNode *iother = DCAST(InstancedNode, other);

    // Only combine them if the instance lists for both are identical.
    Thread *current_thread = Thread::get_current_thread();
    CDReader this_cdata(_cycler, current_thread);
    CDReader other_cdata(iother->_cycler, current_thread);
    CPT(InstanceList) this_instances = this_cdata->_instances.get_read_pointer(current_thread);
    CPT(InstanceList) other_instances = other_cdata->_instances.get_read_pointer(current_thread);
    if (this_instances == other_instances) {
      return this;
    }
  }

  return nullptr;
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 */
CPT(TransformState) InstancedNode::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {

  CPT(InstanceList) instances = get_instances(current_thread);
  CPT(TransformState) next_transform = transform->compose(get_transform(current_thread));

  for (size_t ii = 0; ii < instances->size(); ++ii) {
    CPT(TransformState) instance_transform = next_transform->compose((*instances)[ii].get_transform());

    Children cr = get_children(current_thread);
    size_t num_children = cr.get_num_children();
    for (size_t ci = 0; ci < num_children; ++ci) {
      cr.get_child(ci)->calc_tight_bounds(min_point, max_point,
                                          found_any, instance_transform,
                                          current_thread);
    }
  }

  return next_transform;
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool InstancedNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  Thread *current_thread = trav->get_current_thread();

  CPT(InstanceList) instances = get_instances(current_thread);

  if (data._instances != nullptr) {
    // We are already under an instanced node.  Create a new combined list.
    InstanceList *new_list = new InstanceList();
    new_list->reserve(data._instances->size() * instances->size());
    for (const InstanceList::Instance &parent_instance : *data._instances) {
      for (const InstanceList::Instance &this_instance : *instances) {
        new_list->append(parent_instance.get_transform()->compose(this_instance.get_transform()));
      }
    }
    instances = new_list;
  }

  if (data._view_frustum != nullptr || data._cull_planes != nullptr) {
    // Culling is on, so we need to figure out which instances should be culled.
    BitArray culled_instances;
    culled_instances.set_range(0, instances->size());

    for (size_t ii = 0; ii < instances->size(); ++ii) {
      if (data.is_instance_in_view((*instances)[ii].get_transform(), trav->get_camera_mask())) {
        culled_instances.clear_bit(ii);
      }
    }

    if (!culled_instances.is_zero() && trav->get_fake_view_frustum_cull()) {
      // The culled instances are drawn with the fake-view-frustum-cull effect.
      data._instances = instances->without(culled_instances ^ BitArray::range(0, instances->size()));

      Children children = data.node_reader()->get_children();
      int num_children = children.get_num_children();
      for (int i = 0; i < num_children; ++i) {
        trav->do_fake_cull(data, children.get_child(i), data._net_transform, data._state);
      }
    }

    instances = instances->without(culled_instances);
  }

  if (instances->empty()) {
    // There are no instances, or they are all culled away.
    return false;
  }

  data._instances = std::move(instances);

  // Disable culling from this point on, for now.  It's probably not worth it
  // to keep lists of transformed bounding volumes for each instance.
  data._view_frustum = nullptr;
  data._cull_planes = CullPlanes::make_empty();

  return true;
}

/**
 *
 */
void InstancedNode::
output(std::ostream &out) const {
  PandaNode::output(out);
  out << " (" << get_num_instances() << " instances)";
}

/**
 * Returns a BoundingVolume that represents the external contents of the node.
 * This should encompass the internal bounds, but also the bounding volumes of
 * of all this node's children, which are passed in.
 */
void InstancedNode::
compute_external_bounds(CPT(BoundingVolume) &external_bounds,
                        BoundingVolume::BoundsType btype,
                        const BoundingVolume **volumes, size_t num_volumes,
                        int pipeline_stage, Thread *current_thread) const {

  CPT(InstanceList) instances = get_instances(current_thread);

  PT(GeometricBoundingVolume) gbv;
  if (btype == BoundingVolume::BT_sphere) {
    gbv = new BoundingSphere;
  } else {
    gbv = new BoundingBox;
  }

  if (num_volumes == 0 || instances->empty()) {
    external_bounds = gbv;
    return;
  }

  // Compute a sphere at the origin, encompassing the children.  This may not
  // be the most optimal shape, but it allows us to easily estimate a bounding
  // volume without having to take each instance transform into account.
  PN_stdfloat max_radius = 0;
  LVector3 max_abs_box(0);

  for (size_t i = 0; i < num_volumes; ++i) {
    const BoundingVolume *child_volume = volumes[i];
    if (child_volume->is_empty()) {
      continue;
    }
    if (child_volume->is_infinite()) {
      gbv->set_infinite();
      break;
    }
    if (const BoundingSphere *child_sphere = child_volume->as_bounding_sphere()) {
      max_radius = child_sphere->get_center().length() + child_sphere->get_radius();
    }
    else if (const FiniteBoundingVolume *child_finite = child_volume->as_finite_bounding_volume()) {
      LPoint3 min1 = child_finite->get_min();
      LPoint3 max1 = child_finite->get_max();
      max_abs_box.set(
        std::max(max_abs_box[0], std::max(std::fabs(min1[0]), std::fabs(max1[0]))),
        std::max(max_abs_box[1], std::max(std::fabs(min1[1]), std::fabs(max1[1]))),
        std::max(max_abs_box[2], std::max(std::fabs(min1[2]), std::fabs(max1[2]))));
    }
    else {
      gbv->set_infinite();
      break;
    }
  }

  max_radius = std::max(max_radius, max_abs_box.length());
  if (max_radius == 0 || gbv->is_infinite()) {
    external_bounds = gbv;
    return;
  }

  // Now that we have a sphere encompassing the children, we will make a box
  // surrounding all the instances, extended by the computed radius.
  LPoint3 min_point = (*instances)[0].get_pos();
  LPoint3 max_point(min_point);

  for (const InstanceList::Instance &instance : *instances) {
    // To make the math easier and not have to take rotations into account, we
    // take the highest scale component and multiply it by the radius of the
    // bounding sphere on the origin we just calculated.
    LVecBase3 scale = instance.get_scale();
    PN_stdfloat max_scale = std::max(std::fabs(scale[0]), std::max(std::fabs(scale[1]), std::fabs(scale[2])));
    PN_stdfloat inst_radius = max_scale * max_radius;
    LVector3 extends_by(inst_radius);
    LPoint3 pos = instance.get_pos();
    min_point = min_point.fmin(pos - extends_by);
    max_point = max_point.fmax(pos + extends_by);
  }

  if (min_point == max_point) {
    external_bounds = gbv;
    return;
  }

  // If we really need to make a sphere, we use the center of the bounding box
  // as our sphere center, and iterate again to find the furthest instance.
  if (btype == BoundingVolume::BT_sphere) {
    LPoint3 center = (min_point + max_point) * 0.5;

    PN_stdfloat max_distance = 0;
    for (const InstanceList::Instance &instance : *instances) {
      LVecBase3 scale = instance.get_scale();
      PN_stdfloat max_scale = std::max(std::fabs(scale[0]), std::max(std::fabs(scale[1]), std::fabs(scale[2])));
      PN_stdfloat inst_radius = max_scale * max_radius;
      PN_stdfloat distance = (instance.get_pos() - center).length() + inst_radius;
      max_distance = std::max(max_distance, distance);
    }

    if (max_distance == 0) {
      external_bounds = gbv;
      return;
    }
    ((BoundingSphere *)gbv.p())->set_center(center);
    ((BoundingSphere *)gbv.p())->set_radius(max_distance);
  } else {
    ((BoundingBox *)gbv.p())->set_min_max(min_point, max_point);
  }

  // If we have a transform, apply it to the bounding volume we just
  // computed.
  CPT(TransformState) transform = get_transform(current_thread);
  if (!transform->is_identity()) {
    gbv->xform(transform->get_mat());
  }

  external_bounds = gbv;
}

/**
 * Tells the BamReader how to create objects of type GeomNode.
 */
void InstancedNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void InstancedNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type InstancedNode is encountered in the Bam file.  It should create the
 * InstancedNode and extract its information from the file.
 */
TypedWritable *InstancedNode::
make_from_bam(const FactoryParams &params) {
  InstancedNode *node = new InstancedNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new InstancedNode.
 */
void InstancedNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
InstancedNode::CData::
CData(const InstancedNode::CData &copy) :
  _instances(copy._instances)
{
}

/**
 *
 */
CycleData *InstancedNode::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void InstancedNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  CPT(InstanceList) instances = _instances.get_read_pointer();
  manager->write_pointer(dg, instances.p());
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int InstancedNode::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _instances = DCAST(InstanceList, p_list[pi++]);
  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomNode.
 */
void InstancedNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);
}
