// Filename: graphicsEngine.cxx
// Created by:  drose (24Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "graphicsEngine.h"
#include "config_display.h"
#include "pipeline.h"
#include "drawCullHandler.h"
#include "binCullHandler.h"
#include "cullResult.h"
#include "cullTraverser.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "pStatClient.h"

#ifndef CPPPARSER
PStatCollector GraphicsEngine::_cull_pcollector("Cull");
PStatCollector GraphicsEngine::_draw_pcollector("Draw");
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::Constructor
//       Access: Published
//  Description: Creates a new GraphicsEngine object.  The Pipeline is
//               normally left to default to NULL, which indicates the
//               global render pipeline, but it may be any Pipeline
//               you choose.
////////////////////////////////////////////////////////////////////
GraphicsEngine::
GraphicsEngine(Pipeline *pipeline) :
  _pipeline(pipeline)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::add_window
//       Access: Published
//  Description: Adds a new window to the set of windows that will be
//               processed when render_frame() is called.  This also
//               increments the reference count to the window.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
add_window(GraphicsWindow *window) {
  _windows.insert(window);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::remove_window
//       Access: Published
//  Description: Removes the indicated window from the set of windows
//               that will be processed when render_frame() is called.
//               This also decrements the reference count to the
//               window, allowing the window to be destructed if there
//               are no other references to it.
//
//               The return value is true if the window was removed,
//               false if it was not found.
////////////////////////////////////////////////////////////////////
bool GraphicsEngine::
remove_window(GraphicsWindow *window) {
  size_t count = _windows.erase(window);
  return (count != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::render_frame
//       Access: Published
//  Description: Renders the next frame in all the registered windows,
//               and flips all of the frame buffers.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
render_frame() {
  if (cull_sorting) {
    cull_bin_draw();
  } else {
    cull_and_draw_together();
  }

  // **** This doesn't belong here; it really belongs in the Pipeline,
  // but here it is for now.
  ClockObject::get_global_clock()->tick();
  PStatClient::main_tick();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::render_subframe
//       Access: Published
//  Description: Performs a complete cull and draw pass for one
//               particular display region.  This is normally useful
//               only for special effects, like shaders, that require
//               a complete offscreen render pass before they can
//               complete.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
render_subframe(GraphicsStateGuardian *gsg, DisplayRegion *dr) {
  if (cull_sorting) {
    cull_bin_draw(gsg, dr);
  } else {
    cull_and_draw_together(gsg, dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::cull_and_draw_together
//       Access: Private
//  Description: An implementation of render_frame() that renders the
//               frame with a DrawCullHandler, to cull and draw all
//               windows in the same pass.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
cull_and_draw_together() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    GraphicsWindow *win = (*wi);
    if (win->get_window_active()) {
      win->begin_frame();
      win->clear();
      
      int num_display_regions = win->get_num_display_regions();
      for (int i = 0; i < num_display_regions; i++) {
        DisplayRegion *dr = win->get_display_region(i);
        cull_and_draw_together(win->get_gsg(), dr);
      }
      win->end_frame();
    }
    win->process_events();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::cull_and_draw_together
//       Access: Private
//  Description: An implementation of render_frame() that renders the
//               frame with a DrawCullHandler, to cull and draw all
//               windows in the same pass.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
cull_and_draw_together(GraphicsStateGuardian *gsg, DisplayRegion *dr) {
  nassertv(gsg != (GraphicsStateGuardian *)NULL);

  PT(SceneSetup) scene_setup = setup_scene(dr->get_camera(), gsg);
  if (setup_gsg(gsg, scene_setup)) {
    DisplayRegionStack old_dr = gsg->push_display_region(dr);
    gsg->prepare_display_region();
    if (dr->is_any_clear_active()) {
      gsg->clear(dr);
    }
    
    DrawCullHandler cull_handler(gsg);
    do_cull(&cull_handler, scene_setup, gsg);
    
    gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::cull_bin_draw
//       Access: Private
//  Description: An implementation of render_frame() that renders the
//               frame with a BinCullHandler, to cull into bins and
//               then draw the bins.  This is the normal method.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
cull_bin_draw() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    GraphicsWindow *win = (*wi);
    if (win->get_window_active()) {
      win->begin_frame();
      win->clear();
      
      int num_display_regions = win->get_num_display_regions();
      for (int i = 0; i < num_display_regions; i++) {
        DisplayRegion *dr = win->get_display_region(i);
        cull_bin_draw(win->get_gsg(), dr);
      }
      win->end_frame();
    }
    win->process_events();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::cull_bin_draw
//       Access: Private
//  Description: An implementation of render_frame() that renders the
//               frame with a BinCullHandler, to cull into bins and
//               then draw the bins.  This is the normal method.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
cull_bin_draw(GraphicsStateGuardian *gsg, DisplayRegion *dr) {
  nassertv(gsg != (GraphicsStateGuardian *)NULL);

  PT(CullResult) cull_result = dr->_cull_result;
  if (cull_result == (CullResult *)NULL) {
    cull_result = new CullResult(gsg);
  }

  PT(SceneSetup) scene_setup = setup_scene(dr->get_camera(), gsg);
  if (scene_setup != (SceneSetup *)NULL) {
    BinCullHandler cull_handler(cull_result);
    do_cull(&cull_handler, scene_setup, gsg);
    
    cull_result->finish_cull();
    
    // Save the results for next frame.
    dr->_cull_result = cull_result->make_next();
    
    // Now draw.
    do_draw(cull_result, scene_setup, gsg, dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::setup_scene
//       Access: Private
//  Description: Returns a new SceneSetup object appropriate for
//               rendering the scene from the indicated camera, or
//               NULL if the scene should not be rendered for some
//               reason.
////////////////////////////////////////////////////////////////////
PT(SceneSetup) GraphicsEngine::
setup_scene(const NodePath &camera, GraphicsStateGuardian *gsg) {
  if (camera.is_empty()) {
    // No camera, no draw.
    return NULL;
  }

  Camera *camera_node;
  DCAST_INTO_R(camera_node, camera.node(), NULL);

  if (!camera_node->is_active()) {
    // Camera inactive, no draw.
    return NULL;
  }

  Lens *lens = camera_node->get_lens();
  if (lens == (Lens *)NULL) {
    // No lens, no draw.
    return NULL;
  }

  NodePath scene_root = camera_node->get_scene();
  if (scene_root.is_empty()) {
    // No scene, no draw.
    return NULL;
  }

  PT(SceneSetup) scene_setup = new SceneSetup;

  // We will need both the camera transform (the net transform to the
  // camera from the scene) and the world transform (the camera
  // transform inverse, or the net transform to the scene from the
  // camera).
  CPT(TransformState) camera_transform = camera.get_transform(scene_root);
  CPT(TransformState) world_transform = scene_root.get_transform(camera);

  // The render transform is the same as the world transform, except
  // it is converted into the GSG's internal coordinate system.  This
  // is the transform that the GSG will apply to all of its vertices.
  CPT(TransformState) render_transform = world_transform;

  CoordinateSystem external_cs = gsg->get_coordinate_system();
  CoordinateSystem internal_cs = gsg->get_internal_coordinate_system();
  if (internal_cs != CS_default && internal_cs != external_cs) {
    CPT(TransformState) cs_transform = 
      TransformState::make_mat(LMatrix4f::convert_mat(external_cs, internal_cs));
    render_transform = cs_transform->compose(render_transform);
  }

  scene_setup->set_scene_root(scene_root);
  scene_setup->set_camera_path(camera);
  scene_setup->set_camera_node(camera_node);
  scene_setup->set_lens(lens);
  scene_setup->set_camera_transform(camera_transform);
  scene_setup->set_world_transform(world_transform);
  scene_setup->set_render_transform(render_transform);

  return scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::do_cull
//       Access: Private
//  Description: Fires off a cull traversal using the indicated camera.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg) {
  // Statistics
  PStatTimer timer(_cull_pcollector);

  CullTraverser trav;
  trav.set_cull_handler(cull_handler);
  trav.set_depth_offset_decals(gsg->depth_offset_decals());
  trav.set_scene(scene_setup);
  trav.set_camera_mask(scene_setup->get_camera_node()->get_camera_mask());

  if (view_frustum_cull) {
    // If we're to be performing view-frustum culling, determine the
    // bounding volume associated with the current viewing frustum.

    // First, we have to get the current viewing frustum, which comes
    // from the lens.
    PT(BoundingVolume) bv = scene_setup->get_lens()->make_bounds();

    if (bv != (BoundingVolume *)NULL &&
        bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      // Transform it into the appropriate coordinate space.
      PT(GeometricBoundingVolume) local_frustum;
      local_frustum = DCAST(GeometricBoundingVolume, bv->make_copy());
      local_frustum->xform(scene_setup->get_camera_transform()->get_mat());

      trav.set_view_frustum(local_frustum);
    }
  }
  
  trav.traverse(scene_setup->get_scene_root());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::do_draw
//       Access: Private
//  Description: Draws the previously-culled scene.
////////////////////////////////////////////////////////////////////
void GraphicsEngine::
do_draw(CullResult *cull_result, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg, DisplayRegion *dr) {
  // Statistics
  PStatTimer timer(_draw_pcollector);

  if (setup_gsg(gsg, scene_setup)) {
    DisplayRegionStack old_dr = gsg->push_display_region(dr);
    gsg->prepare_display_region();
    if (dr->is_any_clear_active()) {
      gsg->clear(dr);
    }
    cull_result->draw();
    gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::setup_gsg
//       Access: Private
//  Description: Sets up the GSG to draw the indicated scene.  Returns
//               true if the scene (and its lens) is acceptable, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsEngine::
setup_gsg(GraphicsStateGuardian *gsg, SceneSetup *scene_setup) {
  if (scene_setup == (SceneSetup *)NULL) {
    // No scene, no draw.
    return false;
  }

  const Lens *lens = scene_setup->get_lens();
  if (lens == (const Lens *)NULL) {
    // No lens, no draw.
    return false;
  }

  if (!gsg->set_lens(lens)) {
    // The lens is inappropriate somehow.
    display_cat.error()
      << gsg->get_type() << " cannot render with " << lens->get_type()
      << "\n";
    return false;
  }

  gsg->set_scene(scene_setup);

  return true;
}
