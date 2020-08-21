/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderNode.cxx
 * @author jenes
 * @date 2011-03-11
 */

#include "occluderNode.h"

#include "geomNode.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "renderState.h"
#include "plane.h"
#include "pnmImage.h"
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "depthOffsetAttrib.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "transformState.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "boundingSphere.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"
#include "geom.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include "plane.h"

TypeHandle OccluderNode::_type_handle;
PT(Texture) OccluderNode::_viz_tex;


/**
 * The default constructor creates a default occlusion polygon in the XZ plane
 * (or XY plane in a y-up coordinate system).  Use the normal Panda set_pos(),
 * set_hpr(), set_scale() to position it appropriately, or replace the
 * vertices with set_vertices().
 */
OccluderNode::
OccluderNode(const std::string &name) :
  PandaNode(name)
{
  set_cull_callback();
  // OccluderNodes are hidden by default.
  set_overall_hidden(true);
  set_double_sided(false);
  set_min_coverage(0.0);
  set_vertices(LPoint3::rfu(-1.0, 0.0, -1.0),
               LPoint3::rfu(1.0, 0.0, -1.0),
               LPoint3::rfu(1.0, 0.0, 1.0),
               LPoint3::rfu(-1.0, 0.0, 1.0));
}

/**
 *
 */
OccluderNode::
OccluderNode(const OccluderNode &copy) :
  PandaNode(copy),
  _double_sided(copy._double_sided),
  _min_coverage(copy._min_coverage),
  _vertices(copy._vertices)
{
}

/**
 *
 */
OccluderNode::
~OccluderNode() {
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *OccluderNode::
make_copy() const {
  return new OccluderNode(*this);
}

/**
 * Returns true if the node's name has extrinsic meaning and must be preserved
 * across a flatten operation, false otherwise.
 */
bool OccluderNode::
preserve_name() const {
  return true;
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void OccluderNode::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  for (Vertices::iterator vi = _vertices.begin();
       vi != _vertices.end();
       ++vi) {
    (*vi) = (*vi) * mat;
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
bool OccluderNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Normally, an OccluderNode is invisible.  But if someone shows it, we will
  // draw a visualization, a checkerboard-textured polygon.
  CullableObject *occluder_viz =
    new CullableObject(get_occluder_viz(trav, data), get_occluder_viz_state(trav, data),
                       data.get_internal_transform(trav));
  trav->get_cull_handler()->record_object(occluder_viz, trav);

  // Also get the frame.
  nassertr(_frame_viz != nullptr, false);
  CullableObject *frame_viz =
    new CullableObject(_frame_viz, get_frame_viz_state(trav, data),
                       data.get_internal_transform(trav));
  trav->get_cull_handler()->record_object(frame_viz, trav);

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool OccluderNode::
is_renderable() const {
  return true;
}


/**
 * Writes a brief description of the node to the indicated output stream.
 * This is invoked by the << operator.  It may be overridden in derived
 * classes to include some information relevant to the class.
 */
void OccluderNode::
output(std::ostream &out) const {
  PandaNode::output(out);
}

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void OccluderNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume by putting it around all of our
  // vertices.
  if (!_vertices.empty()) {
    const LPoint3 *vertices_begin = &_vertices[0];
    const LPoint3 *vertices_end = vertices_begin + _vertices.size();
    gbv->around(vertices_begin, vertices_end);
  }

  internal_bounds = bound;
  internal_vertices = 0;
}

/**
 * Returns a Geom that represents the visualization of the OccluderNode, as
 * seen from the front.
 */
PT(Geom) OccluderNode::
get_occluder_viz(CullTraverser *trav, CullTraverserData &data) {
  if (_occluder_viz == nullptr) {
    nassertr(_vertices.size() == 4, nullptr);

    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Recomputing viz for " << *this << "\n";
    }

    PT(GeomVertexData) vdata = new GeomVertexData
      (get_name(), GeomVertexFormat::get_v3n3t2(), Geom::UH_static);

    // Compute the polygon normal from the first three vertices.
    LPlane plane(_vertices[0], _vertices[1], _vertices[2]);
    LVector3 poly_normal = plane.get_normal();

    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    GeomVertexWriter normal(vdata, InternalName::get_normal());
    GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
    vertex.add_data3(_vertices[0]);
    normal.add_data3(poly_normal);
    texcoord.add_data2(0.0, 0.0);

    vertex.add_data3(_vertices[1]);
    normal.add_data3(poly_normal);
    texcoord.add_data2(8.0, 0.0);

    vertex.add_data3(_vertices[2]);
    normal.add_data3(poly_normal);
    texcoord.add_data2(8.0, 8.0);

    vertex.add_data3(_vertices[3]);
    normal.add_data3(poly_normal);
    texcoord.add_data2(0.0, 8.0);

    PT(GeomTriangles) triangles = new GeomTriangles(Geom::UH_static);
    triangles->add_vertices(0, 1, 2);
    triangles->close_primitive();
    triangles->add_vertices(0, 2, 3);
    triangles->close_primitive();

    _occluder_viz = new Geom(vdata);
    _occluder_viz->add_primitive(triangles);

    PT(GeomLinestrips) lines = new GeomLinestrips(Geom::UH_static);
    lines->add_vertices(0, 1, 2, 3);
    lines->add_vertex(0);
    lines->close_primitive();

    _frame_viz = new Geom(vdata);
    _frame_viz->add_primitive(lines);
  }

  return _occluder_viz;
}

/**
 * Returns the RenderState to apply to the visualization.
 */
CPT(RenderState) OccluderNode::
get_occluder_viz_state(CullTraverser *trav, CullTraverserData &data) {
  if (_viz_tex == nullptr) {
    // Create a default texture.  We set it up as a 2x2 graytone checkerboard,
    // since that's real easy, and it doesn't look like a CollisionPolygon.
    _viz_tex = new Texture("occluder_viz");
    _viz_tex->setup_2d_texture(2, 2, Texture::T_unsigned_byte, Texture::F_luminance);
    PTA_uchar image;
    image.set_data("\x20\x80\x80\x20");
    _viz_tex->set_ram_image(image);
    _viz_tex->set_minfilter(SamplerState::FT_nearest);
    _viz_tex->set_magfilter(SamplerState::FT_nearest);
  }

  static CPT(RenderState) viz_state;
  if (viz_state == nullptr) {
    viz_state = RenderState::make
      (ColorAttrib::make_flat(LVecBase4(1.0f, 1.0f, 1.0f, 0.5f)),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha),
       DepthOffsetAttrib::make(),
       TextureAttrib::make(_viz_tex));
    viz_state = viz_state->adjust_all_priorities(1);
  }

  CPT(RenderState) state = viz_state;
  if (is_double_sided()) {
    state = viz_state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none), 1);
  } else {
    state = viz_state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise), 1);
  }

  return state->compose(viz_state);
}

/**
 * Returns the RenderState to apply to the frame.
 */
CPT(RenderState) OccluderNode::
get_frame_viz_state(CullTraverser *trav, CullTraverserData &data) {
  static CPT(RenderState) viz_state;
  if (viz_state == nullptr) {
    viz_state = RenderState::make
      (ColorAttrib::make_flat(LVecBase4(0.0f, 0.0f, 0.0f, 1.0f)),
       TextureAttrib::make_off());
    viz_state = viz_state->adjust_all_priorities(1);
  }

  return data._state->compose(viz_state);
}

/**
 * Tells the BamReader how to create objects of type OccluderNode.
 */
void OccluderNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void OccluderNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  dg.add_uint16(_vertices.size());
  for (Vertices::const_iterator vi = _vertices.begin();
       vi != _vertices.end();
       ++vi) {
    (*vi).write_datagram(dg);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int OccluderNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type OccluderNode is encountered in the Bam file.  It should create the
 * OccluderNode and extract its information from the file.
 */
TypedWritable *OccluderNode::
make_from_bam(const FactoryParams &params) {
  OccluderNode *node = new OccluderNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new OccluderNode.
 */
void OccluderNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  int num_vertices = scan.get_uint16();
  _vertices.clear();
  _vertices.reserve(num_vertices);
  for (int i = 0; i < num_vertices; i++) {
    LPoint3 vertex;
    vertex.read_datagram(scan);
    _vertices.push_back(vertex);
  }
}
