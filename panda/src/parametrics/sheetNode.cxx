/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sheetNode.cxx
 * @author drose
 * @date 2003-10-11
 */

#include "sheetNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "pStatTimer.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "boundingSphere.h"
#include "colorAttrib.h"
#include "renderState.h"

TypeHandle SheetNode::_type_handle;

PStatCollector SheetNode::_sheet_node_pcollector("*:SheetNode");

/**
 *
 */
CycleData *SheetNode::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SheetNode::CData::
write_datagram(BamWriter *writer, Datagram &dg) const {
  // For now, we write a NULL pointer.  Eventually we will write out the
  // NurbsSurfaceEvaluator pointer.
  writer->write_pointer(dg, nullptr);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SheetNode.
 */
void SheetNode::CData::
fillin(DatagramIterator &scan, BamReader *reader) {
  // For now, we skip over the NULL pointer that we wrote out.
  reader->skip_pointer(scan);
  _surface.clear();
}

/**
 *
 */
SheetNode::
SheetNode(const std::string &name) :
  PandaNode(name)
{
  set_cull_callback();
}

/**
 *
 */
SheetNode::
SheetNode(const SheetNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *SheetNode::
make_copy() const {
  return new SheetNode(*this);
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * Node by calling the xform() method, false otherwise.  For instance, it's
 * usually a bad idea to attempt to xform a SheetNode.
 */
bool SheetNode::
safe_to_transform() const {
  return false;
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
bool SheetNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Statistics
  PStatTimer timer(_sheet_node_pcollector);

  // Create some geometry on-the-fly to render the sheet.
  if (get_num_u_subdiv() > 0 && get_num_v_subdiv() > 0) {
    NurbsSurfaceEvaluator *surface = get_surface();
    if (surface != nullptr) {
      PT(NurbsSurfaceResult) result = surface->evaluate(data.get_node_path());

      if (result->get_num_u_segments() > 0 && result->get_num_v_segments() > 0) {
        render_sheet(trav, data, result);
      }
    }
  }

  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool SheetNode::
is_renderable() const {
  return true;
}

/**
 *
 */
void SheetNode::
output(std::ostream &out) const {
  PandaNode::output(out);
  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != nullptr) {
    out << " " << *surface;
  } else {
    out << " (no surface)";
  }
}

/**
 *
 */
void SheetNode::
write(std::ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != nullptr) {
    indent(out, indent_level + 2) << *surface << "\n";
  } else {
    indent(out, indent_level + 2) << "(no surface)\n";
  }
}

/**
 * Recomputes the bounding volume.  This is normally called automatically, but
 * it must occasionally be called explicitly when the surface has changed
 * properties outside of this node's knowledge.
 */
void SheetNode::
reset_bound(const NodePath &rel_to) {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  do_recompute_bounds(rel_to, pipeline_stage, current_thread);
  mark_internal_bounds_stale(current_thread);
}

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void SheetNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  PT(BoundingVolume) bounds =
    do_recompute_bounds(NodePath((PandaNode *)this), pipeline_stage,
                        current_thread);

  internal_bounds = bounds;
  internal_vertices = 0;  // TODO--estimate this better.
}

/**
 * Does the actual internal recompute.
 */
PT(BoundingVolume) SheetNode::
do_recompute_bounds(const NodePath &rel_to, int pipeline_stage,
                    Thread *current_thread) const {
  // TODO: fix the bounds so that it properly reflects the indicated pipeline
  // stage.  At the moment, we cheat and get some of the properties from the
  // current pipeline stage, the lazy way.

  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;

  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != nullptr) {
    NurbsSurfaceEvaluator::Vert3Array verts;
    get_surface()->get_vertices(verts, rel_to);

    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->around(&verts[0], &verts[0] + verts.size());
  }
  return bound;
}

/**
 * Draws the sheet as a series of tristrips along its length.
 */
void SheetNode::
render_sheet(CullTraverser *trav, CullTraverserData &data,
             NurbsSurfaceResult *result) {
  bool use_vertex_color = get_use_vertex_color();

  int num_u_segments = result->get_num_u_segments();
  int num_v_segments = result->get_num_v_segments();
  int num_u_verts = get_num_u_subdiv() + 1;
  int num_v_verts = get_num_v_subdiv() + 1;

  CPT(GeomVertexFormat) format;
  if (use_vertex_color) {
    format = GeomVertexFormat::get_v3n3cpt2();
  } else {
    format = GeomVertexFormat::get_v3n3t2();
  }
  PT(GeomVertexData) vdata = new GeomVertexData
    ("sheet", format, Geom::UH_stream);
  int expected_num_vertices = num_u_segments * (num_u_verts + 1) * num_v_segments * num_v_verts;
  vdata->reserve_num_rows(expected_num_vertices);

  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  GeomVertexWriter color(vdata, InternalName::get_color());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  for (int ui = 0; ui < num_u_segments; ui++) {
    for (int uni = 0; uni <= num_u_verts; uni++) {
      PN_stdfloat u0 = (PN_stdfloat)uni / (PN_stdfloat)num_u_verts;
      PN_stdfloat u0_tc = result->get_segment_u(ui, u0);

      for (int vi = 0; vi < num_v_segments; vi++) {
        for (int vni = 0; vni < num_v_verts; vni++) {
          PN_stdfloat v = (PN_stdfloat)vni / (PN_stdfloat)(num_v_verts - 1);
          PN_stdfloat v_tc = result->get_segment_v(vi, v);

          LPoint3 point;
          LVector3 norm;
          result->eval_segment_point(ui, vi, u0, v, point);
          result->eval_segment_normal(ui, vi, u0, v, norm);
          vertex.add_data3(point);
          normal.add_data3(norm);
          texcoord.add_data2(u0_tc, v_tc);

          if (use_vertex_color) {
            LColor c0;
            result->eval_segment_extended_points(ui, vi, u0, v, 0, &c0[0], 4);
            color.add_data4(c0);
          }
        }
      }
    }
  }
  nassertv(vdata->get_num_rows() == expected_num_vertices);

  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_stream);

  int expected_num_tristrips = num_u_segments * num_u_verts * num_v_segments;
  int expected_verts_per_tristrip = num_v_verts * 2;

  int expected_prim_vertices = (expected_num_tristrips - 1) * (expected_verts_per_tristrip + strip->get_num_unused_vertices_per_primitive()) + expected_verts_per_tristrip;

  strip->reserve_num_vertices(expected_prim_vertices);

  int verts_per_row = num_v_segments * num_v_verts;

  for (int ui = 0; ui < num_u_segments; ui++) {
    for (int uni = 0; uni < num_u_verts; uni++) {
      int row_start_index = ((ui * (num_u_verts + 1)) + uni) * verts_per_row;

      for (int vi = 0; vi < num_v_segments; vi++) {
        for (int vni = 0; vni < num_v_verts; vni++) {
          int vert_index_0 = row_start_index + (vi * num_v_verts) + vni;
          int vert_index_1 = vert_index_0 + verts_per_row;
          strip->add_vertex(vert_index_0);
          strip->add_vertex(vert_index_1);
        }
        strip->close_primitive();
      }
    }
  }
  nassertv(strip->get_num_vertices() == expected_prim_vertices);

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  CPT(RenderState) state = data._state;
  if (use_vertex_color) {
    state = state->add_attrib(ColorAttrib::make_vertex());
  }

  CullableObject *object =
    new CullableObject(geom, state,
                       data.get_internal_transform(trav));
  trav->get_cull_handler()->record_object(object, trav);
}

/**
 * Tells the BamReader how to create objects of type SheetNode.
 */
void SheetNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SheetNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type SheetNode is encountered in the Bam file.  It should create the
 * SheetNode and extract its information from the file.
 */
TypedWritable *SheetNode::
make_from_bam(const FactoryParams &params) {
  SheetNode *node = new SheetNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SheetNode.
 */
void SheetNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}
