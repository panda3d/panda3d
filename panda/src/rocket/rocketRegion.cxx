/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRegion.cxx
 * @author rdb
 * @date 2011-11-30
 */

#include "rocketRegion.h"
#include "graphicsOutput.h"
#include "orthographicLens.h"
#include "pStatTimer.h"

#if defined(HAVE_ROCKET_DEBUGGER) && !defined(CPPPARSER)
#include <Rocket/Debugger.h>
#endif

#ifdef HAVE_PYTHON
#include "py_panda.h"
#endif

TypeHandle RocketRegion::_type_handle;

/**
 * Make sure that context_name is unique.
 */
RocketRegion::
RocketRegion(GraphicsOutput *window, const LVecBase4 &dr_dimensions,
             const std::string &context_name) :
  DisplayRegion(window, dr_dimensions) {

  // A hack I don't like.  libRocket's decorator system has a bug somewhere,
  // and this seems to be a workaround.
  if (Rocket::Core::GetRenderInterface() == nullptr) {
    Rocket::Core::SetRenderInterface(&_interface);
  }

  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rocket::Core::Vector2i dimensions (pr - pl, pt - pb);

  if (rocket_cat.is_debug()) {
    rocket_cat.debug()
      << "Setting initial context dimensions to ("
      << dimensions.x << ", " << dimensions.y << ")\n";
  }

  _context = Rocket::Core::CreateContext(context_name.c_str(),
                                         dimensions, &_interface);
  nassertv(_context != nullptr);

  _lens = new OrthographicLens;
  _lens->set_film_size(dimensions.x, -dimensions.y);
  _lens->set_film_offset(dimensions.x * 0.5, dimensions.y * 0.5);
  _lens->set_near_far(-1, 1);

  PT(Camera) cam = new Camera(context_name, _lens);
  set_camera(NodePath(cam));
}

/**
 *
 */
RocketRegion::
~RocketRegion() {
  if (Rocket::Core::GetRenderInterface() == &_interface) {
    Rocket::Core::SetRenderInterface(nullptr);
  }

  if (_context != nullptr) {
    if (_context->GetReferenceCount() > 1) {
      _context->RemoveReference();
      return;
    }

    // We need to do this because libRocket may call into Python code to throw
    // destruction events.
#ifdef HAVE_ROCKET_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    _context->RemoveReference();

#ifdef HAVE_ROCKET_PYTHON
    PyGILState_Release(gstate);
#endif
  }
}

/**
 * Performs a cull traversal for this region.
 */
void RocketRegion::
do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg, Thread *current_thread) {

  PStatTimer timer(get_cull_region_pcollector(), current_thread);

  // We (unfortunately) need to do this because libRocket may call into Python
  // code to throw events.
#ifdef HAVE_ROCKET_PYTHON
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rocket::Core::Vector2i dimensions (pr - pl, pt - pb);

  if (_context->GetDimensions() != dimensions) {
    if (rocket_cat.is_debug()) {
      rocket_cat.debug() << "Setting context dimensions to ("
        << dimensions.x << ", " << dimensions.y << ")\n";
    }

    _context->SetDimensions(dimensions);

    _lens->set_film_size(dimensions.x, -dimensions.y);
    _lens->set_film_offset(dimensions.x * 0.5, dimensions.y * 0.5);
  }

  if (_input_handler != nullptr) {
    _input_handler->update_context(_context, pl, pb);
  } else {
    _context->Update();
  }

  CullTraverser *trav = get_cull_traverser();
  trav->set_cull_handler(cull_handler);
  trav->set_scene(scene_setup, gsg, get_incomplete_render());
  trav->set_view_frustum(nullptr);

  _interface.render(_context, trav);

#ifdef HAVE_ROCKET_PYTHON
  PyGILState_Release(gstate);
#endif

  trav->end_traverse();
}

/**
 * Initializes the libRocket debugger.  This will return false if the debugger
 * failed to initialize, or if support for the debugger has not been built in
 * (for example in an optimize=4 build).
 */
bool RocketRegion::
init_debugger() {
#ifdef HAVE_ROCKET_DEBUGGER
  return Rocket::Debugger::Initialise(_context);
#else
  return false;
#endif
}

/**
 * Sets whether the debugger should be visible.
 */
void RocketRegion::
set_debugger_visible(bool visible) {
#ifdef HAVE_ROCKET_DEBUGGER
  Rocket::Debugger::SetVisible(visible);
#endif
}

/**
 * Returns true if the debugger is visible.
 */
bool RocketRegion::
is_debugger_visible() const {
#ifdef HAVE_ROCKET_DEBUGGER
  return Rocket::Debugger::IsVisible();
#else
  return false;
#endif
}
