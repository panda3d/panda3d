/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lodNode.cxx
 * @author drose
 * @date 2002-03-06
 */

#include "lodNode.h"
#include "fadeLodNode.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "config_pgraphnodes.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomVertexFormat.h"
#include "geomTristrips.h"
#include "mathNumbers.h"
#include "geom.h"
#include "geomNode.h"
#include "transformState.h"
#include "material.h"
#include "materialAttrib.h"
#include "materialPool.h"
#include "renderState.h"
#include "cullFaceAttrib.h"
#include "textureAttrib.h"
#include "boundingSphere.h"
#include "geometricBoundingVolume.h"
#include "look_at.h"
#include "nodePath.h"
#include "shaderAttrib.h"
#include "colorAttrib.h"
#include "clipPlaneAttrib.h"

TypeHandle LODNode::_type_handle;

/**
 * Creates a new LODNode of the type specified by the default-lod-type config
 * variable.
 */
PT(LODNode) LODNode::
make_default_lod(const std::string &name) {
  switch (default_lod_type.get_value()) {
  case LNT_pop:
    return new LODNode(name);

  case LNT_fade:
    return new FadeLODNode(name);

  default:
    pgraph_cat.error()
      << "Invalid LODNodeType value: " << (int)default_lod_type << "\n";
    return new LODNode(name);
  }
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *LODNode::
make_copy() const {
  return new LODNode(*this);
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool LODNode::
safe_to_combine() const {
  return false;
}

/**
 * Returns true if it is generally safe to combine the children of this
 * PandaNode with each other.  For instance, an LODNode's children should not
 * be combined with each other, because the set of children is meaningful.
 */
bool LODNode::
safe_to_combine_children() const {
  return false;
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void LODNode::
xform(const LMatrix4 &mat) {
  CDWriter cdata(_cycler);

  cdata->_center = cdata->_center * mat;

  // We'll take just the length of the y axis as the matrix's scale.
  LVector3 y;
  mat.get_row3(y, 1);
  PN_stdfloat factor = y.length();

  SwitchVector::iterator si;
  for (si = cdata->_switch_vector.begin();
       si != cdata->_switch_vector.end();
       ++si) {
    (*si).rescale(factor);
  }
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
bool LODNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (is_any_shown()) {
    return show_switches_cull_callback(trav, data);
  }

  consider_verify_lods(trav, data);

  CDReader cdata(_cycler);

  CPT(TransformState) rel_transform = get_rel_transform(trav, data);
  LPoint3 center = cdata->_center * rel_transform->get_mat();
  PN_stdfloat dist2 = center.dot(center);

  int num_children = std::min(get_num_children(), (int)cdata->_switch_vector.size());
  for (int index = 0; index < num_children; ++index) {
    const Switch &sw = cdata->_switch_vector[index];
    bool in_range;
    if (cdata->_got_force_switch) {
      in_range = (cdata->_force_switch == index);
    } else {
      in_range = sw.in_range_2(dist2 * cdata->_lod_scale
                   * trav->get_scene()->get_camera_node()->get_lod_scale());
    }

    if (in_range) {
      // This switch level is in range.  Draw its children.
      PandaNode *child = get_child(index);
      if (child != nullptr) {
        CullTraverserData next_data(data, child);
        trav->traverse(next_data);
      }
    }
  }

  // Now return false indicating that we have already taken care of the
  // traversal from here.
  return false;
}

/**
 *
 */
void LODNode::
output(std::ostream &out) const {
  PandaNode::output(out);
  CDReader cdata(_cycler);
  out << " center(" << cdata->_center << ") ";
  if (cdata->_switch_vector.empty()) {
    out << "no switches.";
  } else {
    SwitchVector::const_iterator si;
    si = cdata->_switch_vector.begin();
    out << "(" << (*si).get_in() << "/" << (*si).get_out() << ")";
    ++si;
    while (si != cdata->_switch_vector.end()) {
      out << " (" << (*si).get_in() << "/" << (*si).get_out() << ")";
      ++si;
    }
  }
}

/**
 * A simple downcast check.  Returns true if this kind of node happens to
 * inherit from LODNode, false otherwise.
 *
 * This is provided as a a faster alternative to calling
 * is_of_type(LODNode::get_class_type()).
 */
bool LODNode::
is_lod_node() const {
  return true;
}

/**
 * This is provided as a debugging aid.  show_switch() will put the LODNode
 * into a special mode where rather than computing and drawing the appropriate
 * level of the LOD, a ring is drawn around the LODNode center indicating the
 * switch distances from the camera for the indicated level, and the geometry
 * of the indicated level is drawn in wireframe.
 *
 * Multiple different levels can be visualized this way at once.  Call
 * hide_switch() or hide_all_switches() to undo this mode and restore the
 * LODNode to its normal behavior.
 */
void LODNode::
show_switch(int index) {
  CDWriter cdata(_cycler);
  do_show_switch(cdata, index, get_default_show_color(index));
  mark_internal_bounds_stale();
}

/**
 * This is provided as a debugging aid.  show_switch() will put the LODNode
 * into a special mode where rather than computing and drawing the appropriate
 * level of the LOD, a ring is drawn around the LODNode center indicating the
 * switch distances from the camera for the indicated level, and the geometry
 * of the indicated level is drawn in wireframe.
 *
 * Multiple different levels can be visualized this way at once.  Call
 * hide_switch() or hide_all_switches() to undo this mode and restore the
 * LODNode to its normal behavior.
 */
void LODNode::
show_switch(int index, const LColor &color) {
  CDWriter cdata(_cycler);
  do_show_switch(cdata, index, color);
  mark_internal_bounds_stale();
}

/**
 * Disables a previous call to show_switch().
 */
void LODNode::
hide_switch(int index) {
  CDWriter cdata(_cycler);
  do_hide_switch(cdata, index);
  mark_internal_bounds_stale();
}

/**
 * Shows all levels in their default colors.
 */
void LODNode::
show_all_switches() {
  CDWriter cdata(_cycler);
  for (int i = 0; i < (int)cdata->_switch_vector.size(); ++i) {
    do_show_switch(cdata, i, get_default_show_color(i));
  }
  mark_internal_bounds_stale();
}

/**
 * Hides all levels, restoring the LODNode to normal operation.
 */
void LODNode::
hide_all_switches() {
  CDWriter cdata(_cycler);
  for (int i = 0; i < (int)cdata->_switch_vector.size(); ++i) {
    do_hide_switch(cdata, i);
  }
  mark_internal_bounds_stale();
}

/**
 * Returns true if the bounding volumes for the geometry of each fhild node
 * entirely fits within the switch_in radius for that child, or false
 * otherwise.  It is almost always a mistake for the geometry of an LOD level
 * to be larger than its switch_in radius.
 */
bool LODNode::
verify_child_bounds() const {
  bool okflag = true;
  CDReader cdata(_cycler);

  for (int index = 0; index < (int)cdata->_switch_vector.size(); ++index) {
    PN_stdfloat suggested_radius;
    if (!do_verify_child_bounds(cdata, index, suggested_radius)) {
      const Switch &sw = cdata->_switch_vector[index];
      pgraph_cat.warning()
        << "Level " << index << " geometry of " << *this
        << " is larger than its switch radius; suggest radius of "
        << suggested_radius << " instead of " << sw.get_in() << "\n";
      okflag = false;
    }
  }

  return okflag;
}

/**
 * Determines which child should be visible according to the current camera
 * position.  If a child is visible, returns its index number; otherwise,
 * returns -1.
 */
int LODNode::
compute_child(CullTraverser *trav, CullTraverserData &data) {
  if (data.get_net_transform(trav)->is_singular()) {
    // If we're under a singular transform, we can't compute the LOD; select
    // none of them instead.
    return -1;
  }

  CDReader cdata(_cycler);

  if (cdata->_got_force_switch) {
    return cdata->_force_switch;
  }

  CPT(TransformState) rel_transform = get_rel_transform(trav, data);
  LPoint3 center = cdata->_center * rel_transform->get_mat();
  PN_stdfloat dist2 = center.dot(center);

  for (int index = 0; index < (int)cdata->_switch_vector.size(); ++index) {
    if (cdata->_switch_vector[index].in_range_2(dist2 * cdata->_lod_scale
         * trav->get_scene()->get_camera_node()->get_lod_scale())) {
      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << data.get_node_path() << " at distance " << sqrt(dist2)
          << ", selected child " << index << "\n";
      }

      return index;
    }
  }

  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << data.get_node_path() << " at distance " << sqrt(dist2)
      << ", no children in range.\n";
  }

  return -1;
}


/**
 * A special version of cull_callback() that is to be invoked when the LODNode
 * is in show_switch() mode.  This just draws the rings and the wireframe
 * geometry for the selected switches.
 */
bool LODNode::
show_switches_cull_callback(CullTraverser *trav, CullTraverserData &data) {
  CDReader cdata(_cycler);

  CPT(TransformState) rel_transform = get_rel_transform(trav, data);
  LPoint3 center = cdata->_center * rel_transform->get_mat();
  PN_stdfloat dist2 = center.dot(center);

  // Now orient the disk(s) in camera space such that their origin is at
  // center, and the (0, 0, 0) point in camera space is on the disk.
  LMatrix4 mat;
  look_at(mat, -center, LVector3(0.0f, 0.0f, 1.0f));
  mat.set_row(3, center);
  CPT(TransformState) viz_transform =
    rel_transform->invert_compose(TransformState::make_mat(mat));

  for (int index = 0; index < (int)cdata->_switch_vector.size(); ++index) {
    const Switch &sw = cdata->_switch_vector[index];
    if (sw.is_shown()) {
      bool in_range;
      if (cdata->_got_force_switch) {
        in_range = (cdata->_force_switch == index);
      } else {
        in_range = sw.in_range_2(dist2);
      }

      if (in_range) {
        // This switch level is in range.  Draw its children in the funny
        // wireframe mode.
        if (index < get_num_children()) {
          PandaNode *child = get_child(index);
          if (child != nullptr) {
            CullTraverserData next_data3(data, child);
            next_data3._state = next_data3._state->compose(sw.get_viz_model_state());
            trav->traverse(next_data3);
          }
        }

        // And draw the spindle in this color.
        CullTraverserData next_data2(data, sw.get_spindle_viz());
        next_data2.apply_transform(viz_transform);
        trav->traverse(next_data2);
      }

      // Draw the rings for this switch level.  We do this after we have drawn
      // the geometry and the spindle.
      CullTraverserData next_data(data, sw.get_ring_viz());
      next_data.apply_transform(viz_transform);
      trav->traverse(next_data);
    }
  }

  // Now return false indicating that we have already taken care of the
  // traversal from here.
  return false;
}

/**
 * Returns a newly-allocated BoundingVolume that represents the internal
 * contents of the node.  Should be overridden by PandaNode classes that
 * contain something internally.
 */
void LODNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;

  // If we have any visible rings, those count in the bounding volume.
  if (is_any_shown()) {
    // Now actually compute the bounding volume by putting it around all of
    // our geoms' bounding volumes.
    pvector<const BoundingVolume *> child_volumes;
    pvector<PT(BoundingVolume) > pt_volumes;

    CDStageReader cdata(_cycler, pipeline_stage, current_thread);

    SwitchVector::const_iterator si;
    for (si = cdata->_switch_vector.begin();
         si != cdata->_switch_vector.end();
         ++si) {
      const Switch &sw = (*si);
      if (sw.is_shown()) {
        PT(BoundingVolume) sphere = new BoundingSphere(cdata->_center, sw.get_in());
        child_volumes.push_back(sphere);
        pt_volumes.push_back(sphere);
      }
    }

    const BoundingVolume **child_begin = &child_volumes[0];
    const BoundingVolume **child_end = child_begin + child_volumes.size();

    bound->around(child_begin, child_end);
  }

  internal_bounds = bound;
  internal_vertices = 0;
}

/**
 * Returns the relative transform to convert from the LODNode space to the
 * camera space.
 */
CPT(TransformState) LODNode::
get_rel_transform(CullTraverser *trav, CullTraverserData &data) {
  // Get a pointer to the camera node.
  Camera *camera = trav->get_scene()->get_camera_node();

  // Get the camera space transform.
  CPT(TransformState) rel_transform;

  NodePath lod_center = camera->get_lod_center();
  if (!lod_center.is_empty()) {
    rel_transform =
      lod_center.get_net_transform()->invert_compose(data.get_net_transform(trav));
  } else {
    NodePath cull_center = camera->get_cull_center();
    if (!cull_center.is_empty()) {
      rel_transform =
        cull_center.get_net_transform()->invert_compose(data.get_net_transform(trav));
    } else {
      rel_transform = data.get_modelview_transform(trav);
    }
  }

  return rel_transform;
}

/**
 * The private implementation of show_switch().
 */
void LODNode::
do_show_switch(LODNode::CData *cdata, int index, const LColor &color) {
  nassertv(index >= 0 && index < (int)cdata->_switch_vector.size());

  if (!cdata->_switch_vector[index].is_shown()) {
    ++cdata->_num_shown;
  }
  cdata->_switch_vector[index].show(color);
}

/**
 * The private implementation of hide_switch().
 */
void LODNode::
do_hide_switch(LODNode::CData *cdata, int index) {
  nassertv(index >= 0 && index < (int)cdata->_switch_vector.size());

  if (cdata->_switch_vector[index].is_shown()) {
    --cdata->_num_shown;
  }
  cdata->_switch_vector[index].hide();
}

/**
 * The private implementation of verify_child_bounds(), this checks the
 * bounding volume of just one child.
 *
 * If the return value is false, suggested_radius is filled with a radius that
 * ought to be large enough to include the child.
 */
bool LODNode::
do_verify_child_bounds(const LODNode::CData *cdata, int index,
                       PN_stdfloat &suggested_radius) const {
  suggested_radius = 0.0f;

  if (index < get_num_children()) {
    const Switch &sw = cdata->_switch_vector[index];
    PandaNode *child = get_child(index);
    if (child != nullptr) {
      UpdateSeq seq;
      CPT(BoundingVolume) bv = child->get_bounds(seq);

      if (seq == sw._bounds_seq) {
        // We previously verified this child, and it hasn't changed since
        // then.
        return sw._verify_ok;
      }

      ((Switch &)sw)._bounds_seq = seq;
      ((Switch &)sw)._verify_ok = true;

      if (bv->is_empty()) {
        // This child has no geometry, so no one cares anyway.
        return true;
      }
      if (bv->is_infinite()) {
        // To be strict, we ought to look closer if the child has an infinite
        // bounding volume, but in practice this is probably just a special
        // case (e.g.  the child contains the camera) that we don't really
        // want to check.
        return true;
      }

      const Switch &sw = cdata->_switch_vector[index];

      const GeometricBoundingVolume *gbv;
      DCAST_INTO_R(gbv, bv, false);
      BoundingSphere sphere(cdata->_center, sw.get_in());
      sphere.local_object();

      int flags = sphere.contains(gbv);
      if ((flags & BoundingVolume::IF_all) != 0) {
        // This child's radius completely encloses its bounding volume.
        // Perfect.  (And this is the most common case.)
        return true;
      }

      if (flags == 0) {
        // This child's radius doesn't even come close to containing its
        // volume.
        nassertr(!gbv->is_infinite(), false);
        sphere.extend_by(gbv);
        suggested_radius = sphere.get_radius();
        ((Switch &)sw)._verify_ok = false;
        return false;
      }

      // This child's radius partially encloses its (loose) bounding volume.
      // We have to look closer to determine whether it, in fact, fully
      // encloses its geometry.
      LPoint3 min_point(0.0f, 0.0f, 0.0f);
      LPoint3 max_point(0.0f, 0.0f, 0.0f);

      bool found_any = false;
      child->calc_tight_bounds(min_point, max_point, found_any,
                               TransformState::make_identity(),
                               Thread::get_current_thread());
      if (!found_any) {
        // Hmm, the child has no geometry after all.
        return true;
      }

      // Now we have a bounding box.  Define the largest sphere we can that
      // fits within this box.  All we can say about this sphere is that it
      // should definitely fit entirely within a bounding sphere that contains
      // all the points of the child.
      LPoint3 box_center = (min_point + max_point) / 2.0f;
      PN_stdfloat box_radius = std::min(std::min(max_point[0] - box_center[0],
                                 max_point[1] - box_center[1]),
                             max_point[2] - box_center[2]);

      BoundingSphere box_sphere(box_center, box_radius);
      box_sphere.local_object();

      // So if any part of this inscribed sphere is outside of the radius,
      // then the radius is bad.
      flags = sphere.contains(&box_sphere);
      if ((flags & BoundingVolume::IF_all) == 0) {
        // No good.
        if (gbv->is_infinite()) {
          sphere.extend_by(&box_sphere);
        } else {
          sphere.extend_by(gbv);
        }
        suggested_radius = sphere.get_radius();
        ((Switch &)sw)._verify_ok = false;
        return false;
      }
    }
  }

  return true;
}

/**
 * Called internally by consider_verify_lods().
 */
void LODNode::
do_auto_verify_lods(CullTraverser *trav, CullTraverserData &data) {
  UpdateSeq seq;
  get_bounds(seq);
  CDLockedReader cdata(_cycler);

  if (cdata->_got_force_switch) {
    // If we're forcing a particular switch, don't verify the LOD sizes, since
    // they don't really apply anymore anyway.  Assume the user knows what
    // he's doing.
    return;
  }

  if (seq != cdata->_bounds_seq) {
    // Time to validate the children again.
    for (int index = 0; index < (int)cdata->_switch_vector.size(); ++index) {
      PN_stdfloat suggested_radius;
      if (!do_verify_child_bounds(cdata, index, suggested_radius)) {
        const Switch &sw = cdata->_switch_vector[index];
        std::ostringstream strm;
        strm
          << "Level " << index << " geometry of " << data.get_node_path()
          << " is larger than its switch radius; suggest radius of "
          << suggested_radius << " instead of " << sw.get_in()
          << " (configure verify-lods 0 to ignore this error)";
        nassert_raise(strm.str());
      }
    }
    CDWriter cdataw(_cycler, cdata);
    cdataw->_bounds_seq = seq;
  }
}

/**
 * Returns a default color appropriate for showing the indicated level.
 */
const LColor &LODNode::
get_default_show_color(int index) {
  static LColor default_colors[] = {
    LColor(1.0f, 0.0f, 0.0f, 0.7f),
    LColor(0.0f, 1.0f, 0.0f, 0.7f),
    LColor(0.0f, 0.0f, 1.0f, 0.7f),
    LColor(0.0f, 1.0f, 1.0f, 0.7f),
    LColor(1.0f, 0.0f, 1.0f, 0.7f),
    LColor(1.0f, 1.0f, 0.0f, 0.7f),
  };
  static const int num_default_colors = sizeof(default_colors) / sizeof(LColor);

  return default_colors[index % num_default_colors];
}


/**
 * Tells the BamReader how to create objects of type LODNode.
 */
void LODNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LODNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LODNode is encountered in the Bam file.  It should create the LODNode
 * and extract its information from the file.
 */
TypedWritable *LODNode::
make_from_bam(const FactoryParams &params) {
  LODNode *node = new LODNode("");

  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LODNode.
 */
void LODNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
CycleData *LODNode::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Ensures that the _lowest and _highest members are set appropriately after a
 * change to the set of switches.
 */
void LODNode::CData::
check_limits() {
  _lowest = 0;
  _highest = 0;
  for (size_t i = 1; i < _switch_vector.size(); ++i) {
    if (_switch_vector[i].get_out() > _switch_vector[_lowest].get_out()) {
      _lowest = i;
    }
    if (_switch_vector[i].get_in() < _switch_vector[_highest].get_in()) {
      _highest = i;
    }
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LODNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  _center.write_datagram(dg);

  dg.add_uint16(_switch_vector.size());

  SwitchVector::const_iterator si;
  for (si = _switch_vector.begin();
       si != _switch_vector.end();
       ++si) {
    (*si).write_datagram(dg);
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LODNode.
 */
void LODNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _center.read_datagram(scan);

  _switch_vector.clear();

  int num_switches = scan.get_uint16();
  _switch_vector.reserve(num_switches);
  for (int i = 0; i < num_switches; i++) {
    Switch sw(0, 0);
    sw.read_datagram(scan);

    _switch_vector.push_back(sw);
  }
  _lod_scale = 1;
}

/**
 * Computes a Geom suitable for rendering the ring associated with this
 * switch.
 */
void LODNode::Switch::
compute_ring_viz() {
  // We render the ring as a series of concentric ring-shaped triangle strips,
  // each of which has num_slices quads.
  static const int num_slices = 50;
  static const int num_rings = 1;

  // There are also two more triangle strips, one for the outer edge, and one
  // for the inner edge.
  static const PN_stdfloat edge_ratio = 0.1;  // ratio of edge height to diameter.

  const GeomVertexFormat *format = GeomVertexFormat::get_v3n3cp();
  PT(GeomVertexData) vdata = new GeomVertexData("LOD_ring", format, Geom::UH_static);

  // Fill up the vertex table with all of the vertices.
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  GeomVertexWriter color(vdata, InternalName::get_color());

  // First, the vertices for the flat ring.
  int ri, si;
  for (ri = 0; ri <= num_rings; ++ri) {
    // r is in the range [0.0, 1.0].
    PN_stdfloat r = (PN_stdfloat)ri / (PN_stdfloat)num_rings;

    // d is in the range [_out, _in].
    PN_stdfloat d = r * (_in - _out) + _out;

    for (si = 0; si < num_slices; ++si) {
      // s is in the range [0.0, 1.0).
      PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;

      // t is in the range [0.0, 2pi).
      PN_stdfloat t = MathNumbers::pi * 2.0f * s;

      PN_stdfloat x = ccos(t);
      PN_stdfloat y = csin(t);
      vertex.add_data3(x * d, y * d, 0.0f);
      normal.add_data3(0.0f, 0.0f, 1.0f);
      color.add_data4(_show_color);
    }
  }

  // Next, the vertices for the inner and outer edges.
  for (ri = 0; ri <= 1; ++ri) {
    PN_stdfloat r = (PN_stdfloat)ri;
    PN_stdfloat d = r * (_in - _out) + _out;

    for (si = 0; si < num_slices; ++si) {
      PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;
      PN_stdfloat t = MathNumbers::pi * 2.0f * s;

      PN_stdfloat x = ccos(t);
      PN_stdfloat y = csin(t);

      vertex.add_data3(x * d, y * d, 0.5f * edge_ratio * d);
      normal.add_data3(x, y, 0.0f);
      color.add_data4(_show_color);
    }

    for (si = 0; si < num_slices; ++si) {
      PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;
      PN_stdfloat t = MathNumbers::pi * 2.0f * s;

      PN_stdfloat x = ccos(t);
      PN_stdfloat y = csin(t);

      vertex.add_data3(x * d, y * d, -0.5f * edge_ratio * d);
      normal.add_data3(x, y, 0.0f);
      color.add_data4(_show_color);
    }
  }

  // Now create the triangle strips.  One tristrip for each ring.
  PT(GeomTristrips) strips = new GeomTristrips(Geom::UH_static);
  for (ri = 0; ri < num_rings; ++ri) {
    for (si = 0; si < num_slices; ++si) {
      strips->add_vertex(ri * num_slices + si);
      strips->add_vertex((ri + 1) * num_slices + si);
    }
    strips->add_vertex(ri * num_slices);
    strips->add_vertex((ri + 1) * num_slices);
    strips->close_primitive();
  }

  // And then one triangle strip for each of the inner and outer edges.
  for (ri = 0; ri <= 1; ++ri) {
    for (si = 0; si < num_slices; ++si) {
      strips->add_vertex((num_rings + 1 + ri * 2) * num_slices + si);
      strips->add_vertex((num_rings + 1 + ri * 2 + 1) * num_slices + si);
    }
    strips->add_vertex((num_rings + 1 + ri * 2) * num_slices);
    strips->add_vertex((num_rings + 1 + ri * 2 + 1) * num_slices);
    strips->close_primitive();
  }

  PT(Geom) ring_geom = new Geom(vdata);
  ring_geom->add_primitive(strips);

  PT(GeomNode) geom_node = new GeomNode("ring");
  geom_node->add_geom(ring_geom);

  // Get a material for two-sided lighting.
  PT(Material) material = new Material();
  material->set_twoside(true);
  material = MaterialPool::get_material(material);

  CPT(RenderState) viz_state =
    RenderState::make(CullFaceAttrib::make(CullFaceAttrib::M_cull_none),
                      TextureAttrib::make_off(),
                      ShaderAttrib::make_off(),
                      MaterialAttrib::make(material),
                      RenderState::get_max_priority());
  if (_show_color[3] != 1.0f) {
    viz_state = viz_state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha),
                                        RenderState::get_max_priority());
  }

  geom_node->set_state(viz_state);

  _ring_viz = geom_node.p();
}

/**
 * Computes a Geom suitable for rendering the LODNode spindle in the color of
 * this switch.
 */
void LODNode::Switch::
compute_spindle_viz() {
  // We render the spindle as a cylinder, which consists of num_rings rings
  // stacked vertically, each of which is a triangle strip of num_slices
  // quads.  The scale is -10 .. 10 vertically, with a radius of 1.0.
  static const int num_slices = 10;
  static const int num_rings = 10;

  const GeomVertexFormat *format = GeomVertexFormat::get_v3n3cp();
  PT(GeomVertexData) vdata = new GeomVertexData("LOD_spindle", format, Geom::UH_static);

  // Fill up the vertex table with all of the vertices.
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  GeomVertexWriter color(vdata, InternalName::get_color());

  int ri, si;
  for (ri = 0; ri <= num_rings; ++ri) {
    // r is in the range [0.0, 1.0].
    PN_stdfloat r = (PN_stdfloat)ri / (PN_stdfloat)num_rings;

    // z is in the range [100.0, -100.0]
    PN_stdfloat z = 100.0f - r * 200.0f;

    for (si = 0; si < num_slices; ++si) {
      // s is in the range [0.0, 1.0).
      PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;

      // t is in the range [0.0, 2pi).
      PN_stdfloat t = MathNumbers::pi * 2.0f * s;

      PN_stdfloat x = ccos(t);
      PN_stdfloat y = csin(t);
      vertex.add_data3(x, y, z);
      normal.add_data3(x, y, 0.0f);
      color.add_data4(_show_color);
    }
  }

  // Now create the triangle strips.  One tristrip for each ring.
  PT(GeomTristrips) strips = new GeomTristrips(Geom::UH_static);
  for (ri = 0; ri < num_rings; ++ri) {
    for (si = 0; si < num_slices; ++si) {
      strips->add_vertex(ri * num_slices + si);
      strips->add_vertex((ri + 1) * num_slices + si);
    }
    strips->add_vertex(ri * num_slices);
    strips->add_vertex((ri + 1) * num_slices);
    strips->close_primitive();
  }

  PT(Geom) spindle_geom = new Geom(vdata);
  spindle_geom->add_primitive(strips);

  PT(GeomNode) geom_node = new GeomNode("spindle");
  geom_node->add_geom(spindle_geom);

  CPT(RenderState) viz_state =
    RenderState::make(CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
                      TextureAttrib::make_off(),
                      ShaderAttrib::make_off(),
                      RenderState::get_max_priority());
  if (_show_color[3] != 1.0f) {
    viz_state = viz_state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha),
                                      RenderState::get_max_priority());
  }

  geom_node->set_state(viz_state);

  _spindle_viz = geom_node.p();
}

/**
 * Computes a RenderState for rendering the children of this switch in colored
 * wireframe mode.
 */
void LODNode::Switch::
compute_viz_model_state() {
  // The RenderState::make() function only takes up to four attribs at once.
  // Since we need more attribs than that, we have to make up our state in two
  // steps.
  _viz_model_state = RenderState::make(RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
                                       TextureAttrib::make_off(),
                                       ShaderAttrib::make_off(),
                                       ColorAttrib::make_flat(_show_color),
                                       RenderState::get_max_priority());
  CPT(RenderState) st2 = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_none),
                                           RenderState::get_max_priority());
  _viz_model_state = _viz_model_state->compose(st2);
}
