/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portalNode.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "portalNode.h"

#include "geomNode.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "renderState.h"
#include "portalClipper.h"
#include "transformState.h"
#include "clipPlaneAttrib.h"
#include "colorScaleAttrib.h"
#include "transparencyAttrib.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "boundingSphere.h"

#include "plane.h"

using std::endl;

TypeHandle PortalNode::_type_handle;


/**
 * Default constructor, just an empty node, no geo This is used to read portal
 * from model.  You can also use this from python to create an empty portal.
 * Then you can set the vertices yourself, with addVertex.
 */
PortalNode::
PortalNode(const std::string &name) :
  PandaNode(name),
  _from_portal_mask(PortalMask::all_on()),
  _into_portal_mask(PortalMask::all_on()),
  _flags(0)
{
  set_cull_callback();

  _visible = false;
  _open = true;
  _clip_plane = false;
  _max_depth = 10;
}

/**
 * Create a default rectangle as portal.  Use this to create an arbitrary
 * portal and setup from Python
 */
PortalNode::
PortalNode(const std::string &name, LPoint3 pos, PN_stdfloat scale) :
  PandaNode(name),
  _from_portal_mask(PortalMask::all_on()),
  _into_portal_mask(PortalMask::all_on()),
  _flags(0)
{
  set_cull_callback();

  add_vertex(LPoint3(pos[0]-1.0*scale, pos[1], pos[2]-1.0*scale));
  add_vertex(LPoint3(pos[0]+1.0*scale, pos[1], pos[2]-1.0*scale));
  add_vertex(LPoint3(pos[0]+1.0*scale, pos[1], pos[2]+1.0*scale));
  add_vertex(LPoint3(pos[0]-1.0*scale, pos[1], pos[2]+1.0*scale));

  _visible = false;
  _open = true;
  _clip_plane = false;
  _max_depth = 10;
}

/**
 *
 */
PortalNode::
PortalNode(const PortalNode &copy) :
  PandaNode(copy),
  _from_portal_mask(copy._from_portal_mask),
  _into_portal_mask(copy._into_portal_mask),
  _flags(copy._flags),
  _vertices(copy._vertices),
  _cell_in(copy._cell_in),
  _cell_out(copy._cell_out),
  _clip_plane(copy._clip_plane),
  _visible(copy._visible),
  _open(copy._open),
  _max_depth(copy._max_depth)
{
}

/**
 *
 */
PortalNode::
~PortalNode() {
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PortalNode::
make_copy() const {
  return new PortalNode(*this);
}

/**
 * Returns true if the node's name has extrinsic meaning and must be preserved
 * across a flatten operation, false otherwise.
 */
bool PortalNode::
preserve_name() const {
  return true;
}

/**
 * initialize the clipping planes and renderstate
 */
void PortalNode::
enable_clipping_planes() {
  _top_plane_node = new PlaneNode("top");
  NodePath top_plane_np = NodePath(this).attach_new_node(_top_plane_node);

  _bottom_plane_node = new PlaneNode("bottom");
  NodePath bottom_plane_np = NodePath(this).attach_new_node(_bottom_plane_node);

  _left_plane_node = new PlaneNode("left");
  NodePath left_plane_np = NodePath(this).attach_new_node(_left_plane_node);

  _right_plane_node = new PlaneNode("right");
  NodePath right_plane_np = NodePath(this).attach_new_node(_right_plane_node);

  CPT(RenderAttrib) plane_attrib = ClipPlaneAttrib::make();
  plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib)->add_on_plane(NodePath(top_plane_np));
  plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib)->add_on_plane(NodePath(bottom_plane_np));
  plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib)->add_on_plane(NodePath(left_plane_np));
  plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib)->add_on_plane(NodePath(right_plane_np));

  _clip_state = RenderState::make(plane_attrib);
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void PortalNode::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

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
PandaNode *PortalNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two PortalNodes can combine, but only if they have the same name,
    // because the name is often meaningful.
    PortalNode *cother = DCAST(PortalNode, other);
    if (get_name() == cother->get_name()) {
      return this;
    }

    // Two PortalNodes with different names can't combine.
    return nullptr;
  }

  return PandaNode::combine_with(other);
}

/**
 * This function will be called during the cull traversal to perform reduced
 * frustum culling.  Basically, once the scenegraph comes across a portal
 * node, it calculates a CulltraverserData with which cell, this portal leads
 * out to and the new frustum.  Then it traverses that child
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool PortalNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  Thread *current_thread = trav->get_current_thread();

  PortalClipper *portal_viewer = trav->get_portal_clipper();
  set_visible(false);
  if (is_open() && !_cell_out.is_empty() && portal_viewer && data._portal_depth <= _max_depth) {
    portal_cat.debug() << "checking portal node  " << *this << endl;
    portal_cat.debug() << "portal_depth is " << data._portal_depth << endl;
    PT(GeometricBoundingVolume) vf = trav->get_view_frustum();
    PT(BoundingVolume) reduced_frustum;

    // remember old viewport and frustum, so we can restore them for the
    // siblings.  (it gets changed by the prepare_portal call)
    LPoint2 old_reduced_viewport_min, old_reduced_viewport_max;
    portal_viewer->get_reduced_viewport(old_reduced_viewport_min, old_reduced_viewport_max);
    PT(BoundingHexahedron) old_bh = portal_viewer->get_reduced_frustum();

    if (portal_viewer->prepare_portal(data.get_node_path())) {
      if ((reduced_frustum = portal_viewer->get_reduced_frustum())) {
        // remember current clip state, we might change it
        CPT(RenderState) old_clip_state = portal_viewer->get_clip_state();

        set_visible(true);
        // The frustum is in camera space
        vf = DCAST(GeometricBoundingVolume, reduced_frustum);

        // create a copy of this reduced frustum, we'll transform it from
        // camera space to the cell_out space
        PT(BoundingHexahedron) new_bh = DCAST(BoundingHexahedron, vf->make_copy());

        // Get the net trasform of the _cell_out as seen from the camera.
        CPT(TransformState) cell_transform = _cell_out.get_net_transform();
        CPT(TransformState) frustum_transform = cell_transform ->invert_compose(portal_viewer->_scene_setup->get_cull_center().get_net_transform());

        // transform to _cell_out space
        new_bh->xform(frustum_transform->get_mat());

        CPT(RenderState) next_state = data._state;

        // set clipping planes, if desired..
        if (_clip_plane) {
          // create a copy of this reduced frustum, we'll transform it from
          // camera space to this portal node's space (because the clip planes
          // are attached to this node)
          PT(BoundingHexahedron) temp_bh = DCAST(BoundingHexahedron, vf->make_copy());
          CPT(TransformState) temp_frustum_transform = data.get_node_path().get_net_transform()->invert_compose(portal_viewer->_scene_setup->get_cull_center().get_net_transform());

          portal_cat.spam() << "clipping plane frustum transform " << *temp_frustum_transform << endl;
          portal_cat.spam() << "frustum before transform " << *temp_bh << endl;
          // transform to portalNode space
          temp_bh->xform(temp_frustum_transform->get_mat());

          portal_cat.spam() << "frustum after transform " << *temp_bh << endl;

          _left_plane_node->set_plane(-temp_bh->get_plane(4)); // left plane of bh
          _right_plane_node->set_plane(-temp_bh->get_plane(2));// right plane of bh
          _top_plane_node->set_plane(-temp_bh->get_plane(3)); // top plane of bh
          _bottom_plane_node->set_plane(-temp_bh->get_plane(1));// bottom plane of bh

          portal_cat.spam() << "left plane " << *_left_plane_node << endl;
          portal_cat.spam() << "right plane " << *_right_plane_node << endl;
          portal_cat.spam() << "top plane " << *_top_plane_node << endl;
          portal_cat.spam() << "bottom plane " << *_bottom_plane_node << endl;

          // remember the clip state we just generated
          portal_viewer->set_clip_state(_clip_state);

          if (old_clip_state) {
            portal_cat.spam() << "parent clip state " << *old_clip_state << endl;
          } else {
            portal_cat.spam() << "parent clip state None" << endl;
          }
          portal_cat.spam() << "own clip state " << *_clip_state << endl;
          portal_cat.spam() << "next state " << *next_state << endl;

          // undo parent clip state and compose our new clip state ito the new
          // state
          if (old_clip_state != nullptr) {
              next_state = old_clip_state->invert_compose(next_state);
              portal_cat.spam() << "next state after removing parent state " << *next_state << endl;
          }
          next_state = next_state->compose(_clip_state);
          portal_cat.spam() << "next state after composition " << *next_state << endl;
        }

        CullTraverserData next_data(_cell_out,
                                    cell_transform,
                                    next_state, new_bh,
                                    current_thread);
        next_data._portal_depth = data._portal_depth + 1;

        portal_viewer->set_reduced_frustum(new_bh);
        portal_cat.spam() << "cull_callback: before traversing " << _cell_out.get_name() << endl;
        trav->traverse_below(next_data);
        portal_cat.spam() << "cull_callback: after traversing " << _cell_out.get_name() << endl;

        // restore clip state
        portal_viewer->set_clip_state(old_clip_state);
      }
    }
    // reset portal viewer frustum for the siblings;
    portal_viewer->set_reduced_frustum(old_bh);
    // reset portal viewer viewport for the siblings;
    portal_viewer->set_reduced_viewport(old_reduced_viewport_min, old_reduced_viewport_max);
  }
  // Now carry on to render our child nodes.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool PortalNode::
is_renderable() const {
  return true;
}


/**
 * Writes a brief description of the node to the indicated output stream.
 * This is invoked by the << operator.  It may be overridden in derived
 * classes to include some information relevant to the class.
 */
void PortalNode::
output(std::ostream &out) const {
  PandaNode::output(out);
}

/**
 * Draws the vertices of this portal rectangle to the screen with a line
 */
/*
void PortalNode::
draw() const {
  move_to(get_vertex(0));
  draw_to(get_vertex(1));
  draw_to(get_vertex(2));
  draw_to(get_vertex(3));
}
*/

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void PortalNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume by putting it around all of our
  // vertices.

  const LPoint3 *vertices_begin = &_vertices[0];
  const LPoint3 *vertices_end = vertices_begin + _vertices.size();

  // Now actually compute the bounding volume by putting it around all
  gbv->around(vertices_begin, vertices_end);

  internal_bounds = bound;
  internal_vertices = 0;
}

/**
 * Returns a RenderState for rendering the ghosted portal rectangle that
 * represents the previous frame's position, for those collision nodes that
 * indicate a velocity.
 */
CPT(RenderState) PortalNode::
get_last_pos_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make
      (ColorScaleAttrib::make(LVecBase4(1.0f, 1.0f, 1.0f, 0.5f)),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  return state;
}


/**
 * Tells the BamReader how to create objects of type PortalNode.
 */
void PortalNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PortalNode::
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
int PortalNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type PortalNode is encountered in the Bam file.  It should create the
 * PortalNode and extract its information from the file.
 */
TypedWritable *PortalNode::
make_from_bam(const FactoryParams &params) {
  PortalNode *node = new PortalNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PortalNode.
 */
void PortalNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  int num_vertices = scan.get_uint16();
  _vertices.reserve(num_vertices);
  for (int i = 0; i < num_vertices; i++) {
    LPoint3 vertex;
    vertex.read_datagram(scan);
    _vertices.push_back(vertex);
  }

  /*
  _from_portal_mask.set_word(scan.get_uint32());
  _into_portal_mask.set_word(scan.get_uint32());
  _flags = scan.get_uint8();
  */
}
