/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file planeNode.cxx
 * @author drose
 * @date 2002-07-11
 */

#include "planeNode.h"
#include "geometricBoundingVolume.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "geomVertexWriter.h"
#include "geomVertexData.h"
#include "geomLines.h"
#include "geom.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "boundingPlane.h"

UpdateSeq PlaneNode::_sort_seq;

TypeHandle PlaneNode::_type_handle;

/**
 *
 */
CycleData *PlaneNode::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PlaneNode::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _plane.write_datagram(dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void PlaneNode::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _plane.read_datagram(scan);
}

/**
 *
 */
PlaneNode::
PlaneNode(const std::string &name, const LPlane &plane) :
  PandaNode(name),
  _priority(0),
  _clip_effect(~0)
{
  set_cull_callback();

  // PlaneNodes are hidden by default.
  set_overall_hidden(true);

  set_plane(plane);
}

/**
 *
 */
PlaneNode::
PlaneNode(const PlaneNode &copy) :
  PandaNode(copy),
  _priority(copy._priority),
  _clip_effect(copy._clip_effect),
  _cycler(copy._cycler)
{
}

/**
 *
 */
void PlaneNode::
output(std::ostream &out) const {
  PandaNode::output(out);
  out << " " << get_plane();
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PlaneNode::
make_copy() const {
  return new PlaneNode(*this);
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void PlaneNode::
xform(const LMatrix4 &mat) {
  PandaNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_plane = cdata->_plane * mat;
  cdata->_front_viz = nullptr;
  cdata->_back_viz = nullptr;
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
bool PlaneNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Normally, a PlaneNode is invisible.  But if someone shows it, we will
  // draw a visualization, a nice yellow wireframe.

  CullableObject *plane_viz =
    new CullableObject(get_viz(trav, data), data._state,
                       data.get_internal_transform(trav));
  trav->get_cull_handler()->record_object(plane_viz, trav);

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool PlaneNode::
is_renderable() const {
  return true;
}

/**
 * Returns a newly-allocated BoundingVolume that represents the internal
 * contents of the node.  Should be overridden by PandaNode classes that
 * contain something internally.
 */
void PlaneNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  CDStageReader cdata(_cycler, pipeline_stage, current_thread);
  internal_bounds = new BoundingPlane(cdata->_plane);
  internal_vertices = 0;
}

/**
 * Returns a Geom that represents the visualization of the PlaneNode.
 */
PT(Geom) PlaneNode::
get_viz(CullTraverser *trav, CullTraverserData &data) {
  CDLockedReader cdata(_cycler);

  // Figure out whether we are looking at the front or the back of the plane.
  const Lens *lens = trav->get_scene()->get_lens();
  LPlane eye_plane = cdata->_plane * data.get_modelview_transform(trav)->get_mat();
  bool front = (eye_plane.dist_to_plane(lens->get_nodal_point()) >= 0.0f);

  if (cdata->_front_viz != nullptr) {
    return front ? cdata->_front_viz : cdata->_back_viz;
  }

  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  CDWriter cdataw(_cycler, cdata, false);
  const LPlane &plane = cdataw->_plane;

  PT(GeomVertexData) vdata = new GeomVertexData
    (get_name(), GeomVertexFormat::get_v3cp(), Geom::UH_static);

  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  PT(GeomLines) lines = new GeomLines(Geom::UH_static);

  LVector3 a, b;

  if (fabs(plane[0]) > fabs(plane[1])) {
    // X > Y
    if (fabs(plane[0]) > fabs(plane[2])) {
      // X > Y && X > Z.  X is the largest.
      a.set(0, 1, 0);
      b.set(0, 0, 1);
    } else {
      // X > Y && Z > X.  Z is the largest.
      a.set(1, 0, 0);
      b.set(0, 1, 0);
    }
  } else {
    // Y > X
    if (fabs(plane[1]) > fabs(plane[2])) {
      // Y > X && Y > Z.  Y is the largest.
      a.set(1, 0, 0);
      b.set(0, 0, 1);
    } else {
      // Y > X && Z > Y.  Z is the largest.
      a.set(1, 0, 0);
      b.set(0, 1, 0);
    }
  }

  static const int num_segs = 10;
  a *= cdataw->_viz_scale / (num_segs * 2);
  b *= cdataw->_viz_scale / (num_segs * 2);

  for (int x = -num_segs; x <= num_segs; ++x) {
    vertex.add_data3(plane.project(a * x - b * num_segs));
    vertex.add_data3(plane.project(a * x + b * num_segs));
    lines->add_next_vertices(2);
    lines->close_primitive();
  }
  for (int y = -num_segs; y <= num_segs; ++y) {
    vertex.add_data3(plane.project(b * y - a * num_segs));
    vertex.add_data3(plane.project(b * y + a * num_segs));
    lines->add_next_vertices(2);
    lines->close_primitive();
  }

  cdataw->_front_viz = new Geom(vdata->set_color(LColor(1.0f, 1.0f, 0.0f, 1.0f)));
  cdataw->_front_viz->add_primitive(lines);

  cdataw->_back_viz = new Geom(vdata->set_color(LColor(0.4, 0.4, 0.0f, 1.0f)));
  cdataw->_back_viz->add_primitive(lines);

  return front ? cdataw->_front_viz : cdataw->_back_viz;
}

/**
 * Tells the BamReader how to create objects of type PlaneNode.
 */
void PlaneNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PlaneNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
  dg.add_int32(_priority);
  dg.add_uint8(_clip_effect);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type PlaneNode is encountered in the Bam file.  It should create the
 * PlaneNode and extract its information from the file.
 */
TypedWritable *PlaneNode::
make_from_bam(const FactoryParams &params) {
  PlaneNode *node = new PlaneNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PlaneNode.
 */
void PlaneNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
  _priority = scan.get_int32();

  if (manager->get_file_minor_ver() < 9) {
    _clip_effect = ~0;
  } else {
    _clip_effect = scan.get_uint8();
  }
}
