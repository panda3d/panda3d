// Filename: cullBinOcclusionTest.cxx
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

#include "cullBinOcclusionTest.h"
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

#include <algorithm>

PStatCollector CullBinOcclusionTest::_wait_occlusion_pcollector("Draw:Wait occlusion");
PStatCollector CullBinOcclusionTest::_occlusion_previous_pcollector("Occlusion test:Previously visible");
PStatCollector CullBinOcclusionTest::_occlusion_passed_pcollector("Occlusion test:Visible");
PStatCollector CullBinOcclusionTest::_occlusion_failed_pcollector("Occlusion test:Occluded");

const LPoint3f CullBinOcclusionTest::_corner_points[8] = {
  LPoint3f(-1.0f, -1.0f, -1.0f),    // 0
  LPoint3f(1.0f, -1.0f, -1.0f),     // OC_x
  LPoint3f(-1.0f, 1.0f, -1.0f),     // OC_y
  LPoint3f(1.0f, 1.0f, -1.0f),      // OC_x | OC_y
  LPoint3f(-1.0f, -1.0f, 1.0f),     // OC_z
  LPoint3f(1.0f, -1.0f, 1.0f),      // OC_x | OC_z
  LPoint3f(-1.0f, 1.0f, 1.0f),      // OC_y | OC_z
  LPoint3f(1.0f, 1.0f, 1.0f),       // OC_x | OC_y | OC_z
};

PT(Geom) CullBinOcclusionTest::_octree_solid_test;
PT(Geom) CullBinOcclusionTest::_octree_wireframe_viz;
CPT(RenderState) CullBinOcclusionTest::_octree_solid_test_state;
TypeHandle CullBinOcclusionTest::_type_handle;

// This class is used to sort the corner index numbers into order from
// closest to the camera to furthest from the camera.
class SortCornersFrontToBack {
public:
  inline bool operator () (int a, int b) {
    return _distances[a] < _distances[b];
  }
  float _distances[8];
};


////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinOcclusionTest::
~CullBinOcclusionTest() {
  ObjectPointers::iterator pi;
  for (pi = _object_pointers.begin(); pi != _object_pointers.end(); ++pi) {
    delete (*pi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::make_bin
//       Access: Public, Static
//  Description: Factory constructor for passing to the CullBinManager.
////////////////////////////////////////////////////////////////////
CullBin *CullBinOcclusionTest::
make_bin(const string &name, GraphicsStateGuardianBase *gsg) {
  return new CullBinOcclusionTest(name, gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::make_next
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
PT(CullBin) CullBinOcclusionTest::
make_next() const {
  // We use the copy constructor, which creates an empty CullBin
  // object, but also copies the _prev_draw pointer into it, so that
  // there will be inter-frame continuity.
  return new CullBinOcclusionTest(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::
add_object(CullableObject *object, Thread *current_thread) {
  // Determine the world-space bounding sphere for the object.
  CPT(BoundingVolume) volume = object->_geom->get_bounds();
  if (volume->is_empty()) {
    return;
  }

  ++_num_objects;

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
//     Function: CullBinOcclusionTest::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::
finish_cull(SceneSetup *scene_setup, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);

  // Now we have a loose list of objects that are to be rendered.
  // We'd rather have them in an octree, which has much better
  // grouping properties for the purpose of this algorithm.

  // For now, we'll just build an octree here at runtime, a new one
  // fresh for each frame.  Maybe it won't be *too* bad.  But later,
  // we can optimize this to take advantage of temporal coherence by
  // starting from the previous frame's octree.
  _root.make_initial_bounds();
  _root.group_objects();

  // Figure out the best front-to-back order of the corners of each
  // octree node, based on the current viewing orientation.
  CPT(TransformState) world_transform = scene_setup->get_world_transform();
  const LMatrix4f &world_mat = world_transform->get_mat();

  // A temporary object to record distances, and manage the sorting.
  SortCornersFrontToBack sorter;

  for (int i = 0; i < 8; ++i) {
    _corners_front_to_back[i] = i;

    LPoint3f p = _corner_points[i] * world_mat;
    sorter._distances[i] = _gsg->compute_distance_to(p);
  }

  // Now sort, using the STL sort function.
  ::sort(&_corners_front_to_back[0], &_corners_front_to_back[8], sorter);

  // Finally, use that information to compute the distance of each
  // octree node fom the camera plane.
  _root.compute_distance(world_mat, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::draw
//       Access: Public, Virtual
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::
draw(Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);

  // We'll want to know the near plane distance.
  _near_distance = _gsg->get_scene()->get_lens()->get_near();

  // First, draw any objects that were visible last frame.
  int num_drawn_previous;
  {
    MutexHolder holder(_prev_draw->_visible_lock);
    num_drawn_previous = _root.draw_previous(*this, current_thread);
  }

  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Drew " << num_drawn_previous << " objects.\n";
  }

  // Now draw the objects that may or may not remain.
  int num_drawn;
  num_drawn = _root.draw(*this, current_thread);
  if (show_octree) {
    _root.draw_wireframe(*this, current_thread);
  }

  while (!_pending_nodes.empty()) {
    PendingNode &pending = _pending_nodes.front();
    int num_fragments;
    if (!pending._query->is_answer_ready()) {
      // The answer isn't ready yet.  We have to wait.
      PStatTimer timer(_wait_occlusion_pcollector);
      num_fragments = pending._query->get_num_fragments();
    } else {
      // The answer is ready right now.  There will be no waiting.
      num_fragments = pending._query->get_num_fragments();
    }
    if (cull_cat.is_spam()) {
      cull_cat.spam()
        << "OctreeNode " << *pending._octree_node
        << " shows " << num_fragments << " fragments\n";
    }
    if (num_fragments != 0) {
      // The octree cell is at least partially visible.  Draw it, and
      // continue recursion.
      num_drawn += pending._octree_node->draw(*this, current_thread);
      if (show_octree) {
        pending._octree_node->draw_wireframe(*this, current_thread);
      }
    }
    _pending_nodes.pop_front();
  }

  _occlusion_previous_pcollector.add_level_now(num_drawn_previous);
  _occlusion_passed_pcollector.add_level_now(num_drawn);
  _occlusion_failed_pcollector.add_level_now(_num_objects - (num_drawn_previous + num_drawn));

  // Now, store a list of the objects within OctreeNodes that passed
  // the occlusion test this frame, so we can ensure that they are
  // drawn first next frame.
  VisibleGeoms visible_geoms;
  _root.record_visible_geoms(visible_geoms);
  visible_geoms.sort();

  {
    MutexHolder holder(_prev_draw->_visible_lock);
    _prev_draw->_visible_geoms.swap(visible_geoms);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinOcclusionTest::OctreeNode::
OctreeNode() {
  for (int i = 0; i < 8; ++i) {
    _corners[i] = (OctreeNode *)NULL;
  }

  _is_visible = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinOcclusionTest::OctreeNode::
OctreeNode(float mid_x, float mid_y, float mid_z, float half_side) :
  _mid(mid_x, mid_y, mid_z),
  _half_side(half_side)
{
  for (int i = 0; i < 8; ++i) {
    _corners[i] = (OctreeNode *)NULL;
  }

  // OctreeNodes are considered occluded until they pass the occlusion
  // test.
  _is_visible = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinOcclusionTest::OctreeNode::
~OctreeNode() {
  for (int i = 0; i < 8; ++i) {
    if (_corners[i] != (OctreeNode *)NULL) {
      delete _corners[i];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::make_initial_bounds
//       Access: Public
//  Description: Determines the minmax bounding volume of the root
//               OctreeNode, based on the objects it contains.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
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
//     Function: CullBinOcclusionTest::OctreeNode::group_objects
//       Access: Public
//  Description: Recursively groups the objects assigned to this node
//               into smaller octree nodes, as appropriate.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
group_objects() {
  if ((int)_objects.size() <= max_objects_per_octree_node) {
    // No need to do any more subdividing.
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
//     Function: CullBinOcclusionTest::OctreeNode::compute_distance
//       Access: Public
//  Description: Recursively computes the _distance member of each
//               octree node, as the linear distance from the camera
//               plane to the nearest corner of the octree node.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
compute_distance(const LMatrix4f &world_mat,
                 CullBinOcclusionTest &bin) {
  int index = bin._corners_front_to_back[0];
  LPoint3f p = get_corner_point(index) * world_mat;
  _distance = bin._gsg->compute_distance_to(p);
  
  for (int i = 0; i < 8; ++i) {
    if (_corners[i] != (OctreeNode *)NULL) {
      _corners[i]->compute_distance(world_mat, bin);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::occlusion_test
//       Access: Public
//  Description: Tests the octree node for visibility by rendering the
//               octree cube, invisibly, with a query.  Returns the
//               occlusion query object representing this test.
////////////////////////////////////////////////////////////////////
PT(OcclusionQueryContext) CullBinOcclusionTest::OctreeNode::
occlusion_test(CullBinOcclusionTest &bin, Thread *current_thread) {
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
  CPT(TransformState) internal_transform = bin._gsg->get_cs_transform()->compose(modelview_transform);
  
  CPT(RenderState) state = get_octree_solid_test_state();
  PT(GeomMunger) munger = bin._gsg->get_geom_munger(state, current_thread);
  
  CPT(Geom) viz = get_octree_solid_test();
  CPT(GeomVertexData) munged_data = viz->get_vertex_data();
  munger->munge_geom(viz, munged_data, current_thread);
  
  bin._gsg->set_state_and_transform(state, internal_transform);

  PStatTimer timer(bin._draw_occlusion_pcollector);
  bin._gsg->begin_occlusion_query();
  viz->draw(bin._gsg, munger, munged_data, current_thread);
  return bin._gsg->end_occlusion_query();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::draw_previous
//       Access: Public
//  Description: Recursively draws only those objects which are known
//               to have been drawn last frame.  Returns the number of
//               objects drawn.
////////////////////////////////////////////////////////////////////
int CullBinOcclusionTest::OctreeNode::
draw_previous(CullBinOcclusionTest &bin, Thread *current_thread) {
  int num_drawn = 0;

  if (!_objects.empty()) {
    Objects::const_iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
      CullableObject *object = (*oi)._object;
      if (!object->_already_drawn) {
        VisibleGeom vg(object->_geom, object->_net_transform);
        if (bin._prev_draw->_visible_geoms.find(vg) != bin._prev_draw->_visible_geoms.end()) {
          // This object is visible.
          CullHandler::draw(object, bin._gsg, current_thread);
          object->_already_drawn = true;
          ++num_drawn;
        }
      }
    }
  }

  for (int i = 0; i < 8; ++i) {
    // Render the child octree nodes in order from nearest to
    // farthest.
    int index = bin._corners_front_to_back[i];
    if (_corners[index] != (OctreeNode *)NULL) {
      num_drawn += _corners[index]->draw_previous(bin, current_thread);
    }
  }

  return num_drawn;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::draw
//       Access: Public
//  Description: Draws all of the objects in this node, and
//               recursively performs occlusion tests on all of the
//               nested nodes.  Returns the number of objects drawn.
////////////////////////////////////////////////////////////////////
int CullBinOcclusionTest::OctreeNode::
draw(CullBinOcclusionTest &bin, Thread *current_thread) {
  // If the node is being drawn, it must have passed the occlusion
  // test.  Flag it as such.
  _is_visible = true;
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Drawing OctreeNode " << this << "\n";
  }

  int num_drawn = 0;

  if (!_objects.empty()) {
    // Now draw the objects within the octree node.
    Objects::const_iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
      CullableObject *object = (*oi)._object;
      if (!object->_already_drawn) {
        CullHandler::draw(object, bin._gsg, current_thread);
        object->_already_drawn = true;
        ++num_drawn;
      }
    }
  }

  // Now recurse on each child node.
  for (int i = 0; i < 8; ++i) {
    // Make sure we render the child octree nodes in order from
    // nearest to farthest.
    int index = bin._corners_front_to_back[i];
    if (_corners[index] != (OctreeNode *)NULL) {
      if (_corners[index]->_distance < bin._near_distance) {
        // If a corner of the cube pokes through the near plane, go
        // ahead and render the whole cube without performing an
        // occlusion test.  The occlusion test would be invalid (since
        // some or all of the cube would be clipped), but it's not
        // likely that anything will be occluding something so close
        // to the camera anyway.
        _corners[index]->draw(bin, current_thread);

      } else {
        // Otherwise, if the entire cube is in front of the near
        // plane, perform an occlusion test by rendering out the
        // (invisible) octree cube and then see if any pixels make it
        // through the depth test.
        PendingNode pending;
        pending._octree_node = _corners[index];
        pending._query = _corners[index]->occlusion_test(bin, current_thread);

        // We push it onto the list of nodes that are awaiting
        // feedback from the graphics pipe.  This way we can go work
        // on another octree node while we're waiting for this one.
        bin._pending_nodes.push_back(pending);
      }
    }
  }

  return num_drawn;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::draw_wireframe
//       Access: Public
//  Description: Draws a wireframe representation of the octree cube,
//               for debugging and visualization purposes.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
draw_wireframe(CullBinOcclusionTest &bin, Thread *current_thread) {
  // As above, this is complicated because we're doing this at such a
  // low level.
  CPT(TransformState) net_transform = TransformState::make_pos_hpr_scale
    (_mid, LVecBase3f(0.0f, 0.0f, 0.0f), 
     LVecBase3f(_half_side, _half_side, _half_side));
  CPT(TransformState) world_transform = bin._gsg->get_scene()->get_world_transform();
  CPT(TransformState) modelview_transform = world_transform->compose(net_transform);
  CPT(TransformState) internal_transform = bin._gsg->get_cs_transform()->compose(modelview_transform);
  
  CPT(RenderState) state = RenderState::make_empty();
  PT(GeomMunger) munger = bin._gsg->get_geom_munger(state, current_thread);
  
  CPT(Geom) viz = get_octree_wireframe_viz();
  CPT(GeomVertexData) munged_data = viz->get_vertex_data();
  munger->munge_geom(viz, munged_data, current_thread);
  
  bin._gsg->set_state_and_transform(state, internal_transform);
  viz->draw(bin._gsg, munger, munged_data, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::record_visible_geoms
//       Access: Public
//  Description: Records any Geoms associated with OctreeNodes that
//               passed the occlusion test for next frame, to improve
//               temporal coherence.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
record_visible_geoms(CullBinOcclusionTest::VisibleGeoms &visible_geoms) {
  if (_is_visible) {
    Objects::const_iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
      CullableObject *object = (*oi)._object;
      nassertv(object->_already_drawn);
      VisibleGeom vg(object->_geom, object->_net_transform);
      visible_geoms.push_back(vg);
    }

    for (int i = 0; i < 8; ++i) {
      if (_corners[i] != (OctreeNode *)NULL) {
        _corners[i]->record_visible_geoms(visible_geoms);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
output(ostream &out) const {
  out << "OctreeNode " << _mid << ", " << _half_side << ", dist "
      << _distance;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::get_octree_solid_test
//       Access: Private, Static
//  Description: Returns a Geom that may be used to render the solid
//               faces of octree cube, presumably invisibly.  This
//               returns a cube over the range (-1, -1, -1) - (1, 1,
//               1).
////////////////////////////////////////////////////////////////////
CPT(Geom) CullBinOcclusionTest::
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
//     Function: CullBinOcclusionTest::get_octree_wireframe_viz
//       Access: Private, Static
//  Description: Returns a Geom that may be used to render an
//               OctreeNode in wireframe.  This actually draws a
//               wireframe cube in the range (-1, -1, -1) - (1, 1, 1).
////////////////////////////////////////////////////////////////////
CPT(Geom) CullBinOcclusionTest::
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
//     Function: CullBinOcclusionTest::get_octree_solid_test_state
//       Access: Private, Static
//  Description: Returns the RenderState appropriate to rendering the
//               octree test invisibly.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullBinOcclusionTest::
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
//     Function: CullBinOcclusionTest::OctreeNode::multi_assign
//       Access: Public
//  Description: The object intersects a center plane, but is too
//               small to justify keeping within this big node.
//               Duplicate it into the sub-nodes.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
multi_assign(const CullBinOcclusionTest::ObjectData &object_data) {
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
//     Function: CullBinOcclusionTest::OctreeNode::make_corner
//       Access: Private
//  Description: Makes a new octree node for the indicated corner.
////////////////////////////////////////////////////////////////////
void CullBinOcclusionTest::OctreeNode::
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

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::get_corner_point
//       Access: Private
//  Description: Returns the 3-d point, in world space, of the
//               indicated corner.
////////////////////////////////////////////////////////////////////
LPoint3f CullBinOcclusionTest::OctreeNode::
get_corner_point(int index) {
  switch (index) {
  case 0:
    // -X, -Y, -Z
    return LPoint3f(_mid[0] - _half_side, _mid[1] - _half_side, _mid[2] - _half_side);

  case OC_x:
    // +X, -Y, -Z
    return LPoint3f(_mid[0] + _half_side, _mid[1] - _half_side, _mid[2] - _half_side);

  case OC_y:
    // -X, +Y, -Z
    return LPoint3f(_mid[0] - _half_side, _mid[1] + _half_side, _mid[2] - _half_side);

  case OC_x | OC_y:
    // +X, +Y, -Z
    return LPoint3f(_mid[0] + _half_side, _mid[1] + _half_side, _mid[2] - _half_side);

  case OC_z:
    // -X, -Y, +Z
    return LPoint3f(_mid[0] - _half_side, _mid[1] - _half_side, _mid[2] + _half_side);

  case OC_x | OC_z:
    // +X, -Y, +Z
    return LPoint3f(_mid[0] + _half_side, _mid[1] - _half_side, _mid[2] + _half_side);

  case OC_y | OC_z:
    // -X, +Y, +Z
    return LPoint3f(_mid[0] - _half_side, _mid[1] + _half_side, _mid[2] + _half_side);

  case OC_x | OC_y | OC_z:
    // +X, +Y, +Z
    return LPoint3f(_mid[0] + _half_side, _mid[1] + _half_side, _mid[2] + _half_side);
  }

  nassertr(false, LPoint3f::zero());
  return LPoint3f::zero();
}
