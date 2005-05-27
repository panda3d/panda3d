// Filename: collisionVisualizer.cxx
// Created by:  drose (16Apr03)
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

#include "collisionVisualizer.h"
#include "collisionEntry.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "renderState.h"
#include "omniBoundingVolume.h"
#include "depthOffsetAttrib.h"
#include "colorScaleAttrib.h"
#include "transparencyAttrib.h"
#include "geomLine.h"
#include "geomSphere.h"


#ifdef DO_COLLISION_RECORDING

TypeHandle CollisionVisualizer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CollisionVisualizer::
CollisionVisualizer(const string &name) : PandaNode(name) {
  // We always want to render the CollisionVisualizer node itself
  // (even if it doesn't appear to have any geometry within it).
  set_bound(OmniBoundingVolume());
  _viz_scale = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionVisualizer::
~CollisionVisualizer() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::clear
//       Access: Published
//  Description: Removes all the visualization data from a previous
//               traversal and resets the visualizer to empty.
////////////////////////////////////////////////////////////////////
void CollisionVisualizer::
clear() {
  _data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *CollisionVisualizer::
make_copy() const {
  return new CollisionVisualizer(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool CollisionVisualizer::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool CollisionVisualizer::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Now we go through and actually draw our visualized collision solids.

  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    const TransformState *net_transform = (*di).first;
    const VizInfo &viz_info = (*di).second;

    CullTraverserData xform_data(data);
    
    // We don't want to inherit the transform from above!  We ignore
    // whatever transforms were above the CollisionVisualizer node; it
    // always renders its objects according to their appropriate net
    // transform.
    xform_data._modelview_transform = trav->get_world_transform();
    xform_data.apply_transform_and_state(trav, net_transform, 
                                         RenderState::make_empty(),
                                         RenderEffects::make_empty());

    // Draw all the collision solids.
    Solids::const_iterator si;
    for (si = viz_info._solids.begin(); si != viz_info._solids.end(); ++si) {
      // Note that we don't preserve the clip plane attribute from the
      // collision solid.  We always draw the whole polygon (or
      // whatever) in the CollisionVisualizer.  This is a deliberate
      // decision; clipping the polygons may obscure many collision
      // tests that are being made.
      const CollisionSolid *solid = (*si).first;
      const SolidInfo &solid_info = (*si).second;
      bool was_detected = (solid_info._detected_count > 0);
      PT(PandaNode) node = solid->get_viz(trav, xform_data, !was_detected);
      if (node != (PandaNode *)NULL) {
        CullTraverserData next_data(xform_data, node);
        
        // We don't want to inherit the render state from above for
        // these guys.
        next_data._state = get_viz_state();
        trav->traverse(next_data);
      }
    }

    // Now draw all of the detected points.
    if (!viz_info._points.empty()) {
      CPT(RenderState) empty_state = RenderState::make_empty();
        
      PTA_Colorf colors;
      colors.push_back(Colorf(1.0f, 0.0f, 0.0f, 1.0f));
      colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
        
      Points::const_iterator pi;
      for (pi = viz_info._points.begin(); pi != viz_info._points.end(); ++pi) {
        const CollisionPoint &point = (*pi);
          
        // Draw a small red sphere at the surface point, and a smaller
        // white sphere at the interior point.
        {
          PT(GeomSphere) sphere = new GeomSphere;
          PTA_Vertexf verts;
          verts.push_back(point._surface_point);
          verts.push_back(point._surface_point + 
                          LVector3f(0.1f * _viz_scale, 0.0f, 0.0f));
          sphere->set_coords(verts);
          sphere->set_colors(colors, G_PER_PRIM);
          sphere->set_num_prims(1);
            
          if (point._interior_point != point._surface_point) {
            verts.push_back(point._interior_point);
            verts.push_back(point._interior_point + 
                            LVector3f(0.05f * _viz_scale, 0.0f, 0.0f));
            sphere->set_num_prims(2);
          }
            
          CullableObject *object = 
            new CullableObject(sphere, empty_state, xform_data._modelview_transform);
          
          trav->get_cull_handler()->record_object(object, trav);
        }

        // Draw the normal vector at the surface point.
        if (!point._surface_normal.almost_equal(LVector3f::zero())) {
          PT(GeomLine) line = new GeomLine;
        
          PTA_Vertexf verts;
          verts.push_back(point._surface_point);
          verts.push_back(point._surface_point + 
                          point._surface_normal * _viz_scale);
          line->set_coords(verts);
          line->set_colors(colors, G_PER_VERTEX);
          line->set_num_prims(1);
          
          CullableObject *object = 
            new CullableObject(line, empty_state, xform_data._modelview_transform);
          
          trav->get_cull_handler()->record_object(object, trav);
        }
      }
    }
  }

  // Now carry on to render our child nodes.
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::output
//       Access: Public, Virtual
//  Description: Writes a brief description of the node to the
//               indicated output stream.  This is invoked by the <<
//               operator.  It may be overridden in derived classes to
//               include some information relevant to the class.
////////////////////////////////////////////////////////////////////
void CollisionVisualizer::
output(ostream &out) const {
  PandaNode::output(out);
  out << " ";
  CollisionRecorder::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::begin_traversal
//       Access: Public, Virtual
//  Description: This method is called at the beginning of a
//               CollisionTraverser::traverse() call.  It is provided
//               as a hook for the derived class to reset its state as
//               appropriate.
////////////////////////////////////////////////////////////////////
void CollisionVisualizer::
begin_traversal() {
  CollisionRecorder::begin_traversal();
  _data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::collision_tested
//       Access: Public, Virtual
//  Description: This method is called when a pair of collision solids
//               have passed all bounding-volume tests and have been
//               tested for a collision.  The detected value is set
//               true if a collision was detected, false otherwise.
////////////////////////////////////////////////////////////////////
void CollisionVisualizer::
collision_tested(const CollisionEntry &entry, bool detected) {
  CollisionRecorder::collision_tested(entry, detected);

  NodePath node_path = entry.get_into_node_path();
  CPT(TransformState) net_transform = node_path.get_net_transform();

  VizInfo &viz_info = _data[net_transform];
  if (detected) {
    viz_info._solids[entry.get_into()]._detected_count++;

    if (entry.has_surface_point()) {
      CollisionPoint p;
      entry.get_all(entry.get_into_node_path(),
                    p._surface_point, p._surface_normal, p._interior_point);
      viz_info._points.push_back(p);
    }

  } else {
    viz_info._solids[entry.get_into()]._missed_count++;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionVisualizer::get_viz_state
//       Access: Private
//  Description: Returns a RenderState suitable for rendering the
//               collision solids with which a collision was detected.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CollisionVisualizer::
get_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (DepthOffsetAttrib::make());
  }

  return state;
}

#endif  // DO_COLLISION_RECORDING
