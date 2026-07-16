/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlRegion.cxx
 * @author rdb
 * @date 2011-11-30
 */

#include "rmlRegion.h"
#include "rmlContext.h"
#include "graphicsOutput.h"
#include "graphicsStateGuardian.h"
#include "graphicsWindow.h"
#include "orthographicLens.h"
#include "pStatTimer.h"
#include "drawCullHandler.h"
#include "displayRegionDrawCallbackData.h"
#include "sceneSetup.h"

#ifndef CPPPARSER
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#ifdef HAVE_RMLUI_DEBUGGER
#include <RmlUi/Debugger.h>
#endif
#endif

TypeHandle RmlRegion::_type_handle;

/**
 * Returns the cached Python-accessible wrapper around the underlying
 * Rml::Context.  The same object is returned on every call.
 */
PT(RmlContext) RmlRegion::
get_context() const {
  return _rml_context;
}

/**
 * In RmlUi v6 each context has its own render interface, passed at creation
 * time.  The interface lifetime must exceed the context's.
 */
RmlRegion::
RmlRegion(GraphicsOutput *window, const LVecBase4 &dr_dimensions,
          const std::string &context_name)
  : DisplayRegion(window, dr_dimensions)
{
  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rml::Vector2i dimensions(pr - pl, pt - pb);

  if (rmlui_cat.is_debug()) {
    rmlui_cat.debug()
      << "Creating RmlUi context '" << context_name
      << "' at (" << dimensions.x << ", " << dimensions.y << ")\n";
  }

  _interface.init(window);
  _context = Rml::CreateContext(context_name, dimensions, &_interface);
  if (_context == nullptr) {
    rmlui_cat.error()
      << "Failed to create RmlUi context '" << context_name
      << "' — a context with that name may already exist.\n";
    return;
  }
  _rml_context = new RmlContext(_context);

  // On a high-DPI ("Retina") display with dpi-aware enabled, the window's
  // framebuffer has more physical pixels than the logical size that was
  // requested.  Tell RmlUi about that ratio so that dp-based sizes (fonts,
  // layout) stay the intended physical size and text is rendered at native
  // resolution rather than upscaled.  The ratio is the backing scale factor:
  // actual framebuffer pixels / requested logical pixels.
  if (window->is_of_type(GraphicsWindow::get_class_type())) {
    GraphicsWindow *gwin = DCAST(GraphicsWindow, window);
    // get_requested_properties() may not carry a size (e.g. once the window is
    // fully open the original request has been consumed); guard with has_size()
    // so get_x_size() does not assert.
    const WindowProperties &req = gwin->get_requested_properties();
    int req_x = req.has_size() ? req.get_x_size() : 0;
    int fb_x = window->get_x_size();
    if (req_x > 0 && fb_x > 0) {
      float ratio = (float)fb_x / (float)req_x;
      if (ratio > 1.01f) {
        _context->SetDensityIndependentPixelRatio(ratio);
        if (rmlui_cat.is_debug()) {
          rmlui_cat.debug()
            << "RmlUi density-independent pixel ratio set to " << ratio << "\n";
        }
      }
    }
  }

  _lens = new OrthographicLens;
  _lens->set_film_size(dimensions.x, -dimensions.y);
  _lens->set_film_offset(dimensions.x * 0.5f, dimensions.y * 0.5f);
  _lens->set_near_far(-1, 1);

  PT(Camera) cam = new Camera(context_name, _lens);
  set_camera(NodePath(cam));

  // Install a draw-phase callback.  Under a sorted (cull/draw) threading model,
  // do_cull runs in the cull phase with no open GSG frame; RmlUi's layer
  // compositing (render-target switches that need an open command buffer) can
  // only run with the window frame open, which happens in the draw phase.  The
  // draw callback runs the RmlUi render there with a DrawCullHandler so geometry
  // and compositing both execute immediately.  (Under the "-" cull-and-draw-
  // together model do_cull already has an open frame and renders directly; the
  // draw callback is never invoked in that model.)
  set_draw_callback(new DrawCallback(this));
}

/**
 *
 */
RmlRegion::
~RmlRegion() {
  if (_context != nullptr) {
    // RemoveContext destroys the context and all its documents/elements, which
    // may call back into _interface (ReleaseGeometry/ReleaseTexture); _interface
    // is still alive here (it is a by-value member destroyed after this body).
    Rml::RemoveContext(_context->GetName());
    _context = nullptr;
    if (_rml_context) {
      // RemoveContext destroyed the RmlUi data models; invalidate any retained
      // RmlDataModel wrappers so dirty_variable()/dirty_all() on a stale handle
      // become safe no-ops instead of dereferencing the freed model.
      _rml_context->_invalidate_data_models();
      _rml_context->_ctx = nullptr;
      _rml_context = nullptr;
    }
  }

  // Release the render interface's offscreen layer-buffer pool now, while the
  // GraphicsEngine is still valid.  (~RmlRenderInterface would also do this,
  // but doing it explicitly keeps teardown off the late static-destruction path.)
  _interface.shutdown();
}

/**
 * Cull callback: resize the context if the window dimensions changed and pump
 * input.  The actual RmlUi render is performed by render_now().
 *
 * Under a sorted (cull/draw) threading model — the default — do_cull runs in
 * the cull phase with NO open GSG frame, and `cull_handler` is a BinCullHandler
 * that merely records geometry into bins for the later draw phase.  RmlUi's
 * layer compositing needs to switch render targets mid-frame, which requires an
 * open command buffer; that exists only during the draw phase.  So in that
 * model we DEFER: render_now() is invoked later from the draw callback.
 *
 * Under the "-" cull-and-draw-together model do_cull already runs with an open
 * frame and an immediate-drawing DrawCullHandler, so we render right here.
 */
void RmlRegion::
do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg, Thread *current_thread) {

  if (_context == nullptr) return;

  PStatTimer timer(get_cull_region_pcollector(), current_thread);

  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rml::Vector2i dimensions(pr - pl, pt - pb);

  if (_context->GetDimensions() != dimensions) {
    if (rmlui_cat.is_debug()) {
      rmlui_cat.debug()
        << "Resizing context to (" << dimensions.x << ", " << dimensions.y << ")\n";
    }
    _context->SetDimensions(dimensions);
    _lens->set_film_size(dimensions.x, -dimensions.y);
    _lens->set_film_offset(dimensions.x * 0.5f, dimensions.y * 0.5f);
  }

  if (_input_handler != nullptr) {
    // Panda's pixel_xy mouse coordinate is y-down with the origin at the top
    // of the framebuffer, but get_pixels() returns pb measured y-up from the
    // framebuffer bottom.  RmlUi expects window coords relative to the region's
    // top-left (also y-down), so the correct vertical offset is the region's
    // y-down top edge.  get_region_pixels_i() returns exactly that (yo) and is
    // correct regardless of whether the window is inverted.
    int xo, yo, w, h;
    get_region_pixels_i(xo, yo, w, h);
    _input_handler->update_context(_context, xo, yo);
  } else {
    _context->Update();
  }

  // In a sorted threading model the frame is not open here; defer the render to
  // the draw callback (DrawCallback::do_callback -> render_now).
  if (gsg->get_threading_model().get_cull_sorting()) {
    return;
  }

  // Cull-and-draw-together: the frame is open and cull_handler draws
  // immediately, so render right now.
  render_now(cull_handler, scene_setup, gsg, current_thread);
}

/**
 * Performs the RmlUi render.  Must be called with the GSG frame open and an
 * immediate-drawing cull handler, so that geometry draws as it is submitted and
 * RmlRenderInterface's layer compositing can switch render targets within the
 * open frame.
 */
void RmlRegion::
render_now(CullHandler *cull_handler, SceneSetup *scene_setup,
           GraphicsStateGuardian *gsg, Thread *current_thread) {
  if (_context == nullptr) return;

  CullTraverser *trav = get_cull_traverser();
  trav->set_cull_handler(cull_handler);
  trav->set_scene(scene_setup, gsg, get_incomplete_render());
  trav->set_view_frustum(nullptr);

  _interface.render(_context, trav, gsg, current_thread);

  trav->end_traverse();
}

/**
 * Draw-phase callback (sorted threading models).  Invoked from the draw thread
 * by GraphicsEngine::do_draw with the window frame open, so the RmlUi render
 * and its layer compositing can run against an open command buffer.  Uses a
 * DrawCullHandler so geometry is drawn immediately as RmlUi submits it.
 */
void RmlRegion::DrawCallback::
do_callback(CallbackData *cbdata) {
  DisplayRegionDrawCallbackData *draw_data =
    DCAST(DisplayRegionDrawCallbackData, cbdata);
  SceneSetup *scene_setup = draw_data->get_scene_setup();
  if (scene_setup == nullptr) return;

  RmlRegion *region = _region;
  GraphicsStateGuardian *gsg = region->get_window()->get_gsg();
  Thread *current_thread = Thread::get_current_thread();

  if (!gsg->set_scene(scene_setup)) {
    return;
  }
  gsg->clear_state_and_transform();

  if (gsg->begin_scene()) {
    DrawCullHandler cull_handler(gsg);
    region->render_now(&cull_handler, scene_setup, gsg, current_thread);
    gsg->end_scene();
  }

  gsg->clear_before_callback();
}

/**
 * Initialises the RmlUi visual debugger for this context.  Returns true on
 * success.  Has no effect and returns false if the library was not built with
 * HAVE_RMLUI_DEBUGGER.
 */
bool RmlRegion::
init_debugger() {
#ifdef HAVE_RMLUI_DEBUGGER
  if (_context == nullptr) {
    return false;
  }
  return Rml::Debugger::Initialise(_context);
#else
  return false;
#endif
}

/**
 * Shows or hides the RmlUi visual debugger overlay.
 */
void RmlRegion::
set_debugger_visible(bool visible) {
#ifdef HAVE_RMLUI_DEBUGGER
  Rml::Debugger::SetVisible(visible);
#endif
}

/**
 * Returns true if the RmlUi visual debugger is currently visible.
 */
bool RmlRegion::
is_debugger_visible() const {
#ifdef HAVE_RMLUI_DEBUGGER
  return Rml::Debugger::IsVisible();
#else
  return false;
#endif
}
