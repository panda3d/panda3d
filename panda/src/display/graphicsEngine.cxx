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
#include "pipeline.h"
#include "drawCullHandler.h"
#include "qpcullTraverser.h"
#include "clockObject.h"

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
  cull_and_draw_together();

  // **** This doesn't belong here; it really belongs in the Pipeline,
  // but here it is for now.
  ClockObject::get_global_clock()->tick();
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
    win->clear();

    int num_display_regions = win->get_num_display_regions();
    for (int i = 0; i < num_display_regions; i++) {
      DisplayRegion *dr = win->get_display_region(i);
      cull_and_draw_together(win, dr);
    }
    win->flip();
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
cull_and_draw_together(GraphicsWindow *win, DisplayRegion *dr) {
  const NodeChain &camera = dr->get_qpcamera();
  if (camera.is_empty()) {
    // No camera, no draw.
    return;
  }

  qpCamera *camera_node;
  DCAST_INTO_V(camera_node, camera.node());

  if (!camera_node->is_active()) {
    // Camera inactive, no draw.
    return;
  }

  Lens *lens = camera_node->get_lens();
  if (lens == (Lens *)NULL) {
    // No lens, no draw.
    return;
  }

  PandaNode *scene = camera_node->get_scene();
  if (scene == (PandaNode *)NULL) {
    // No scene, no draw.
    return;
  }

  GraphicsStateGuardian *gsg = win->get_gsg();
  nassertv(gsg != (GraphicsStateGuardian *)NULL);

  if (!gsg->set_lens(lens)) {
    // The lens is inappropriate somehow.
    display_cat.error()
      << gsg->get_type() << " cannot render with " << lens->get_type()
      << "\n";
    return;
  }

  DrawCullHandler cull_handler(gsg);
  qpCullTraverser trav;
  trav.set_cull_handler(&cull_handler);

  // The world transform is computed from the camera's position; we
  // then might need to adjust it into the GSG's internal coordinate
  // system.
  CPT(TransformState) world_transform = camera.get_rel_transform(NodeChain());
  CoordinateSystem external_cs = gsg->get_coordinate_system();
  CoordinateSystem internal_cs = gsg->get_internal_coordinate_system();
  if (internal_cs != CS_default && internal_cs != external_cs) {
    CPT(TransformState) cs_transform = 
      TransformState::make_mat(LMatrix4f::convert_mat(external_cs, internal_cs));
    world_transform = cs_transform->compose(world_transform);
  }
  trav.set_world_transform(world_transform);
  
  DisplayRegionStack old_dr = gsg->push_display_region(dr);
  gsg->prepare_display_region();
  
  trav.traverse(scene);
  
  gsg->pop_display_region(old_dr);
}
