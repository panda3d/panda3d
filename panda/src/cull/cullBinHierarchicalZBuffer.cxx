// Filename: cullBinHierarchicalZBuffer.cxx
// Created by:  drose (24Mar06)
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

#include "cullBinHierarchicalZBuffer.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "geomLines.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "colorWriteAttrib.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"
#include "config_cull.h"
#include "thread.h"

PStatCollector CullBinHierarchicalZBuffer::_wait_occlusion_pcollector("Wait:Occlusion test");
PStatCollector CullBinHierarchicalZBuffer::_geoms_occluded_pcollector("Geoms:Occluded");

PT(Geom) CullBinHierarchicalZBuffer::_octree_solid_test;
PT(Geom) CullBinHierarchicalZBuffer::_octree_wireframe_viz;
CPT(RenderState) CullBinHierarchicalZBuffer::_octree_solid_test_state;
TypeHandle CullBinHierarchicalZBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinHierarchicalZBuffer::
~CullBinHierarchicalZBuffer() {
  ObjectPointers::iterator pi;
  for (pi = _object_pointers.begin(); pi != _object_pointers.end(); ++pi) {
    delete (*pi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::make_bin
//       Access: Public, Static
//  Description: Factory constructor for passing to the CullBinManager.
////////////////////////////////////////////////////////////////////
CullBin *CullBinHierarchicalZBuffer::
make_bin(const string &name, GraphicsStateGuardianBase *gsg) {
  return new CullBinHierarchicalZBuffer(name, gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::make_next
//       Access: Public, Virtual
//  Description: Returns a newly-allocated CullBin object that
//               contains a copy of just the subset of the data from
//               this CullBin object that is worth keeping around
//               for next frame.
//
//               If a particular CullBin object has no data worth
//               preserving till next frame, it is acceptable to
//               return NULL (which is the default behavior of this
//               method).
////////////////////////////////////////////////////////////////////
PT(CullBin) CullBinHierarchicalZBuffer::
make_next() const {
  return (CullBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::
add_object(CullableObject *object) {
  // Determine the world-space bounding sphere for the object.
  CPT(BoundingVolume) volume = object->_geom->get_bounds();
  if (volume->is_empty()) {
    return;
  }

  PT(BoundingSphere) sphere;

  if (volume->is_exact_type(BoundingSphere::get_class_type())) {
    sphere = DCAST(BoundingSphere, volume->make_copy());
  } else {
    const GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, volume);
    PT(BoundingSphere) sphere = new BoundingSphere;
    sphere->around(&gbv, &gbv + 1);
  }

  object->_already_drawn = false;
  sphere->xform(object->_net_transform->get_mat());
  _root.initial_assign(ObjectData(object, sphere));
  _object_pointers.push_back(object);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::
finish_cull() {
  PStatTimer timer(_cull_this_pcollector);

  // Now we have a loose list of objects that are to be rendered.
  // We'd rather have them in an octree, which has much better
  // grouping properties for the purpose of this algorithm.

  // For now, we'll just build an octree here at runtime, a new one
  // fresh for each frame.  Maybe it won't be *too* bad.  But later,
  // we can optimize this to take advantage of temporal coherence by
  // starting from the previous frame's octree.
  _root.make_initial_bounds();
  _root.group_objects();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::draw
//       Access: Public
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::
draw() {
  PStatTimer timer(_draw_this_pcollector);
  _root.draw(*this);

  while (!_pending_nodes.empty()) {
    PendingNode &pending = _pending_nodes.front();
    bool is_occluded;
    if (!pending._query->is_answer_ready()) {
      // The answer isn't ready yet.  We have to wait.
      PStatTimer timer(_wait_occlusion_pcollector);
      is_occluded = (pending._query->get_num_fragments() == 0);
    } else {
      // The answer is ready right now.  There will be no waiting.
      is_occluded = (pending._query->get_num_fragments() == 0);
    }
    if (!is_occluded) {
      // The octree cell is at least partially visible.  Draw it, and
      // continue recursion.
      pending._octree_node->draw(*this);
      if (show_octree) {
        pending._octree_node->draw_wireframe(*this);
      }
    } else {
      // The octree cell is completely occluded.  Don't draw any of
      // it, and don't recurse into it.
      _geoms_occluded_pcollector.add_level(pending._octree_node->get_total_num_objects());
    }
    _pending_nodes.pop_front();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinHierarchicalZBuffer::OctreeNode::
OctreeNode() {
  _total_num_objects = 0;
  for (int i = 0; i < 8; ++i) {
    _corners[i] = (OctreeNode *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinHierarchicalZBuffer::OctreeNode::
OctreeNode(float mid_x, float mid_y, float mid_z, float half_side) :
  _mid(mid_x, mid_y, mid_z),
  _half_side(half_side)
{
  _total_num_objects = 0;
  for (int i = 0; i < 8; ++i) {
    _corners[i] = (OctreeNode *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinHierarchicalZBuffer::OctreeNode::
~OctreeNode() {
  for (int i = 0; i < 8; ++i) {
    if (_corners[i] != (OctreeNode *)NULL) {
      delete _corners[i];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::make_initial_bounds
//       Access: Public
//  Description: Determines the minmax bounding volume of the root
//               OctreeNode, based on the objects it contains.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::OctreeNode::
make_initial_bounds() {
  if (_objects.empty()) {
    return;
  }

  LPoint3f scene_min = _objects[0]._bounds->get_center();
  LPoint3f scene_max = _objects[0]._bounds->get_center();

  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    LPoint3f object_min = (*oi)._bounds->get_min();
    LPoint3f object_max = (*oi)._bounds->get_max();
    scene_min[0] = min(scene_min[0], object_min[0]);
    scene_min[1] = min(scene_min[1], object_min[1]);
    scene_min[2] = min(scene_min[2], object_min[2]);
    scene_max[0] = max(scene_max[0], object_max[0]);
    scene_max[1] = max(scene_max[1], object_max[1]);
    scene_max[2] = max(scene_max[2], object_max[2]);
  }

  float side = max(max(scene_max[0] - scene_min[0],
                       scene_max[1] - scene_min[1]),
                   scene_max[2] - scene_min[2]);

  _mid.set((scene_min[0] + scene_max[0]) * 0.5f,
           (scene_min[1] + scene_max[1]) * 0.5f,
           (scene_min[2] + scene_max[2]) * 0.5f);
  _half_side = side * 0.5f;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::group_objects
//       Access: Public
//  Description: Recursively groups the objects assigned to this node
//               into smaller octree nodes, as appropriate.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::OctreeNode::
group_objects() {
  if ((int)_objects.size() <= max_objects_per_octree_node) {
    // No need to do any more subdividing.
    _total_num_objects += (int)_objects.size();
    return;
  }

  // Assign all objects to one or more corners.
  
  Objects old_objects;
  old_objects.swap(_objects);

  Objects::const_iterator oi;
  for (oi = old_objects.begin(); oi != old_objects.end(); ++oi) {
    const ObjectData &object_data = (*oi);
    const LPoint3f &c = object_data._bounds->get_center();
    float r = object_data._bounds->get_radius();
  
    if (c[0] + r <= _mid[0]) {
      // -X
      if (c[1] + r <= _mid[1]) {
        // -X, -Y
        if (c[2] + r <= _mid[2]) {
          // -X, -Y, -Z
          assign_to_corner(0, object_data);
        } else if (c[2] - r >= _mid[2]) {
          // -X, -Y, +Z
          assign_to_corner(OC_z, object_data);
        } else {
          // -X, -Y, 0
          reassign(object_data);
        }
      } else if (c[1] - r >= _mid[1]) {
        // -X, +Y
        if (c[2] + r <= _mid[2]) {
          // -X, +Y, -Z
          assign_to_corner(OC_y, object_data);
        } else if (c[2] - r >= _mid[2]) {
          // -X, +Y, +Z
          assign_to_corner(OC_y | OC_z, object_data);
        } else {
          // -X, +Y, 0
          reassign(object_data);
        }
      } else {
        // -X, 0
        reassign(object_data);
      }
    } else if (c[0] - r >= _mid[0]) {
      // +X
      if (c[1] + r <= _mid[1]) {
        // +X, -Y
        if (c[2] + r <= _mid[2]) {
          // +X, -Y, -Z
          assign_to_corner(OC_x, object_data);
        } else if (c[2] - r >= _mid[2]) {
          // +X, -Y, +Z
          assign_to_corner(OC_x | OC_z, object_data);
        } else {
          // +X, -Y, 0
          reassign(object_data);
        }
      } else if (c[1] - r >= _mid[1]) {
        // +X, +Y
        if (c[2] + r <= _mid[2]) {
          // +X, +Y, -Z
          assign_to_corner(OC_x | OC_y, object_data);
        } else if (c[2] - r >= _mid[2]) {
          // +X, +Y, +Z
          assign_to_corner(OC_x | OC_y | OC_z, object_data);
        } else {
          // +X, +Y, 0
          reassign(object_data);
        }
      } else {
        // +X, 0
        reassign(object_data);
      }
    } else {
      // 0
      reassign(object_data);
    }
  } 

  for (int i = 0; i < 8; ++i) {
    if (_corners[i] != (OctreeNode *)NULL) {
      _corners[i]->group_objects();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::occlusion_test
//       Access: Public
//  Description: Tests the octree node for visibility by rendering the
//               octree cube, invisibly, with a query.  Returns the
//               occlusion query object representing this test.
////////////////////////////////////////////////////////////////////
PT(OcclusionQueryContext) CullBinHierarchicalZBuffer::OctreeNode::
occlusion_test(CullBinHierarchicalZBuffer &bin) {
  // Draw the bounding volume for visualization.  This is
  // complicated because we're doing this at such a low level, here
  // in the middle of the draw task--we've already completed the
  // cull traversal, so we can't just create a CullableObject or do
  // anything else that requires a pointer to a CullTraverser.
  // Instead, we have to do the relevant code by hand.
  CPT(TransformState) net_transform = TransformState::make_pos_hpr_scale
    (_mid, LVecBase3f(0.0f, 0.0f, 0.0f), 
     LVecBase3f(_half_side, _half_side, _half_side));
  CPT(TransformState) world_transform = bin._gsg->get_scene()->get_world_transform();
  CPT(TransformState) modelview_transform = world_transform->compose(net_transform);
  
  CPT(RenderState) state = get_octree_solid_test_state();
  PT(GeomMunger) munger = bin._gsg->get_geom_munger(state);
  
  CPT(Geom) viz = get_octree_solid_test();
  CPT(GeomVertexData) munged_data = viz->get_vertex_data();
  munger->munge_geom(viz, munged_data);
  
  bin._gsg->set_state_and_transform(state, modelview_transform);

  PStatTimer timer(bin._draw_occlusion_pcollector);
  bin._gsg->begin_occlusion_query();
  viz->draw(bin._gsg, munger, munged_data);
  return bin._gsg->end_occlusion_query();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::draw
//       Access: Public
//  Description: Draws all of the objects in this node, and
//               recursively performs occlusion tests on all of the
//               nested nodes.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::OctreeNode::
draw(CullBinHierarchicalZBuffer &bin) {
  if (!_objects.empty()) {
    // Now draw the objects within the octree node.
    Objects::const_iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
      CullableObject *object = (*oi)._object;
      if (!object->_already_drawn) {
        CullHandler::draw(object, bin._gsg);
        object->_already_drawn = true;
      }
    }
  }

  for (int i = 0; i < 8; ++i) {
    if (_corners[i] != (OctreeNode *)NULL) {
      PendingNode pending;
      pending._octree_node = _corners[i];
      pending._query = _corners[i]->occlusion_test(bin);
      bin._pending_nodes.push_back(pending);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::draw_wireframe
//       Access: Public
//  Description: Draws a wireframe representation of the octree cube,
//               for debugging and visualization purposes.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::OctreeNode::
draw_wireframe(CullBinHierarchicalZBuffer &bin) {
  // As above, this is complicated because we're doing this at such a
  // low level.
  CPT(TransformState) net_transform = TransformState::make_pos_hpr_scale
    (_mid, LVecBase3f(0.0f, 0.0f, 0.0f), 
     LVecBase3f(_half_side, _half_side, _half_side));
  CPT(TransformState) world_transform = bin._gsg->get_scene()->get_world_transform();
  CPT(TransformState) modelview_transform = world_transform->compose(net_transform);
  
  CPT(RenderState) state = RenderState::make_empty();
  PT(GeomMunger) munger = bin._gsg->get_geom_munger(state);
  
  CPT(Geom) viz = get_octree_wireframe_viz();
  CPT(GeomVertexData) munged_data = viz->get_vertex_data();
  munger->munge_geom(viz, munged_data);
  
  bin._gsg->set_state_and_transform(state, modelview_transform);
  viz->draw(bin._gsg, munger, munged_data);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::get_octree_solid_test
//       Access: Private, Static
//  Description: Returns a Geom that may be used to render the solid
//               faces of octree cube, presumably invisibly.  This
//               returns a cube over the range (-1, -1, -1) - (1, 1,
//               1).
////////////////////////////////////////////////////////////////////
CPT(Geom) CullBinHierarchicalZBuffer::
get_octree_solid_test() {
  if (_octree_solid_test == (Geom *)NULL) {
    CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3();
    PT(GeomVertexData) vdata = 
      new GeomVertexData("octree", format, Geom::UH_static);
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    vertex.add_data3f(-1.0f, -1.0f, -1.0f);
    vertex.add_data3f(1.0f, -1.0f, -1.0f);
    vertex.add_data3f(-1.0f, -1.0f, 1.0f);
    vertex.add_data3f(1.0f, -1.0f, 1.0f);
    vertex.add_data3f(-1.0f, 1.0f, -1.0f);
    vertex.add_data3f(1.0f, 1.0f, -1.0f);
    vertex.add_data3f(-1.0f, 1.0f, 1.0f);
    vertex.add_data3f(1.0f, 1.0f, 1.0f);
    PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
    tris->add_vertices(2, 0, 3); tris->close_primitive();
    tris->add_vertices(0, 1, 3); tris->close_primitive();
    tris->add_vertices(3, 1, 7); tris->close_primitive();
    tris->add_vertices(1, 5, 7); tris->close_primitive();
    tris->add_vertices(7, 5, 6); tris->close_primitive();
    tris->add_vertices(5, 4, 6); tris->close_primitive();
    tris->add_vertices(6, 4, 2); tris->close_primitive();
    tris->add_vertices(4, 0, 2); tris->close_primitive();
    tris->add_vertices(6, 2, 7); tris->close_primitive();
    tris->add_vertices(2, 3, 7); tris->close_primitive();
    tris->add_vertices(0, 4, 1); tris->close_primitive();
    tris->add_vertices(4, 5, 1); tris->close_primitive();

    _octree_solid_test = new Geom(vdata);
    _octree_solid_test->add_primitive(tris);
  }

  return _octree_solid_test;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::get_octree_wireframe_viz
//       Access: Private, Static
//  Description: Returns a Geom that may be used to render an
//               OctreeNode in wireframe.  This actually draws a
//               wireframe cube in the range (-1, -1, -1) - (1, 1, 1).
////////////////////////////////////////////////////////////////////
CPT(Geom) CullBinHierarchicalZBuffer::
get_octree_wireframe_viz() {
  if (_octree_wireframe_viz == (Geom *)NULL) {
    CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
    PT(GeomVertexData) vdata = 
      new GeomVertexData("octree", format, Geom::UH_static);
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    vertex.add_data3f(-1.0f, -1.0f, -1.0f);
    vertex.add_data3f(1.0f, -1.0f, -1.0f);
    vertex.add_data3f(-1.0f, -1.0f, 1.0f);
    vertex.add_data3f(1.0f, -1.0f, 1.0f);
    vertex.add_data3f(-1.0f, 1.0f, -1.0f);
    vertex.add_data3f(1.0f, 1.0f, -1.0f);
    vertex.add_data3f(-1.0f, 1.0f, 1.0f);
    vertex.add_data3f(1.0f, 1.0f, 1.0f);
    CPT(GeomVertexData) cvdata = vdata->set_color(Colorf(1.0f, 0.5f, 0.0f, 1.0f));
    PT(GeomLines) lines = new GeomLines(Geom::UH_static);
    lines->add_vertices(0, 1); lines->close_primitive();
    lines->add_vertices(1, 3); lines->close_primitive();
    lines->add_vertices(3, 2); lines->close_primitive();
    lines->add_vertices(2, 0); lines->close_primitive();
    lines->add_vertices(0, 4); lines->close_primitive();
    lines->add_vertices(4, 6); lines->close_primitive();
    lines->add_vertices(6, 7); lines->close_primitive();
    lines->add_vertices(7, 5); lines->close_primitive();
    lines->add_vertices(5, 4); lines->close_primitive();
    lines->add_vertices(1, 5); lines->close_primitive();
    lines->add_vertices(3, 7); lines->close_primitive();
    lines->add_vertices(2, 6); lines->close_primitive();

    _octree_wireframe_viz = new Geom(cvdata);
    _octree_wireframe_viz->add_primitive(lines);
  }

  return _octree_wireframe_viz;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::get_octree_solid_test_state
//       Access: Private, Static
//  Description: Returns the RenderState appropriate to rendering the
//               octree test invisibly.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullBinHierarchicalZBuffer::
get_octree_solid_test_state() {
  if (_octree_solid_test_state == (RenderState *)NULL) {
    _octree_solid_test_state = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       DepthTestAttrib::make(DepthTestAttrib::M_less),
       ColorWriteAttrib::make(ColorWriteAttrib::C_off));
  }

  return _octree_solid_test_state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::multi_assign
//       Access: Public
//  Description: The object intersects a center plane, but is too
//               small to justify keeping within this big node.
//               Duplicate it into the sub-nodes.
////////////////////////////////////////////////////////////////////
void CullBinHierarchicalZBuffer::OctreeNode::
multi_assign(const CullBinHierarchicalZBuffer::ObjectData &object_data) {
  const LPoint3f &c = object_data._bounds->get_center();
  float r = object_data._bounds->get_radius();

  if (c[0] + r <= _mid[0]) {
    // -X
    if (c[1] + r <= _mid[1]) {
      // -X, -Y
      if (c[2] + r <= _mid[2]) {
        // -X, -Y, -Z
        nassertv(false);
      } else if (c[2] - r >= _mid[2]) {
        // -X, -Y, +Z
        nassertv(false);
      } else {
        // -X, -Y, 0
        assign_to_corner(0, object_data);
        assign_to_corner(OC_z, object_data);
      }
    } else if (c[1] - r >= _mid[1]) {
      // -X, +Y
      if (c[2] + r <= _mid[2]) {
        // -X, +Y, -Z
        nassertv(false);
      } else if (c[2] - r >= _mid[2]) {
        // -X, +Y, +Z
        nassertv(false);
      } else {
        // -X, +Y, 0
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
      }
    } else {
      // -X, 0
      if (c[2] + r <= _mid[2]) {
        // -X, 0, -Z
        assign_to_corner(0, object_data);
        assign_to_corner(OC_y, object_data);
      } else if (c[2] - r >= _mid[2]) {
        // -X, 0, +Z
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
      } else {
        // -X, 0, 0
        assign_to_corner(0, object_data);
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
      }
    }
  } else if (c[0] - r >= _mid[0]) {
    // +X
    if (c[1] + r <= _mid[1]) {
      // +X, -Y
      if (c[2] + r <= _mid[2]) {
        // +X, -Y, -Z
        nassertv(false);
      } else if (c[2] - r >= _mid[2]) {
        // +X, -Y, +Z
        nassertv(false);
      } else {
        // +X, -Y, 0
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
      }
    } else if (c[1] - r >= _mid[1]) {
      // +X, +Y
      if (c[2] + r <= _mid[2]) {
        // +X, +Y, -Z
        nassertv(false);
      } else if (c[2] - r >= _mid[2]) {
        // +X, +Y, +Z
        nassertv(false);
      } else {
        // +X, +Y, 0
        assign_to_corner(OC_x | OC_y, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      }
    } else {
      // +X, 0
      if (c[2] + r <= _mid[2]) {
        // +X, 0, -Z
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
      } else if (c[2] - r >= _mid[2]) {
        // +X, 0, +Z
        assign_to_corner(OC_x | OC_z, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      } else {
        // +X, 0, 0
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      }
    }
  } else {
    // 0
    if (c[1] + r <= _mid[1]) {
      // 0, -Y
      if (c[2] + r <= _mid[2]) {
        // 0, -Y, -Z
        assign_to_corner(0, object_data);
        assign_to_corner(OC_x, object_data);
      } else if (c[2] - r >= _mid[2]) {
        // 0, -Y, +Z
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
      } else {
        // 0, -Y, 0
        assign_to_corner(0, object_data);
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
      }
    } else if (c[1] - r >= _mid[1]) {
      // 0, +Y
      if (c[2] + r <= _mid[2]) {
        // 0, +Y, -Z
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
      } else if (c[2] - r >= _mid[2]) {
        // 0, +Y, +Z
        assign_to_corner(OC_y | OC_z, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      } else {
        // 0, +Y, 0
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      }
    } else {
      // 0, 0
      if (c[2] + r <= _mid[2]) {
        // 0, 0, -Z
        assign_to_corner(0, object_data);
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
      } else if (c[2] - r >= _mid[2]) {
        // 0, 0, +Z
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      } else {
        // 0, 0, 0
        assign_to_corner(0, object_data);
        assign_to_corner(OC_z, object_data);
        assign_to_corner(OC_y, object_data);
        assign_to_corner(OC_y | OC_z, object_data);
        assign_to_corner(OC_x, object_data);
        assign_to_corner(OC_x | OC_z, object_data);
        assign_to_corner(OC_x | OC_y, object_data);
        assign_to_corner(OC_x | OC_y | OC_z, object_data);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::make_corner
//       Access: Private
//  Description: Makes a new octree node for the indicated corner.
////////////////////////////////////////////////////////////////////
INLINE void CullBinHierarchicalZBuffer::OctreeNode::
make_corner(int index) {
  nassertv(_corners[index] == NULL);

  OctreeNode *node = NULL;

  double q = _half_side * 0.5f;

  switch (index) {
  case 0:
    // -X, -Y, -Z
    node = new OctreeNode(_mid[0] - q, _mid[1] - q, _mid[2] - q, q);
    break;

  case OC_x:
    // +X, -Y, -Z
    node = new OctreeNode(_mid[0] + q, _mid[1] - q, _mid[2] - q, q);
    break;

  case OC_y:
    // -X, +Y, -Z
    node = new OctreeNode(_mid[0] - q, _mid[1] + q, _mid[2] - q, q);
    break;

  case OC_x | OC_y:
    // +X, +Y, -Z
    node = new OctreeNode(_mid[0] + q, _mid[1] + q, _mid[2] - q, q);
    break;

  case OC_z:
    // -X, -Y, +Z
    node = new OctreeNode(_mid[0] - q, _mid[1] - q, _mid[2] + q, q);
    break;

  case OC_x | OC_z:
    // +X, -Y, +Z
    node = new OctreeNode(_mid[0] + q, _mid[1] - q, _mid[2] + q, q);
    break;

  case OC_y | OC_z:
    // -X, +Y, +Z
    node = new OctreeNode(_mid[0] - q, _mid[1] + q, _mid[2] + q, q);
    break;

  case OC_x | OC_y | OC_z:
    // +X, +Y, +Z
    node = new OctreeNode(_mid[0] + q, _mid[1] + q, _mid[2] + q, q);
    break;
  }
  nassertv(node != (OctreeNode *)NULL);
  _corners[index] = node;
}
