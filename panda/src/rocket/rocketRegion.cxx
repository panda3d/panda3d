// Filename: rocketRegion.cxx
// Created by:  rdb (30Nov11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "rocketRegion.h"
#include "graphicsOutput.h"
#include "orthographicLens.h"
#include "pStatTimer.h"

TypeHandle RocketRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RocketRegion::Constructor
//       Access: Protected
//  Description: Make sure that context_name is unique.
////////////////////////////////////////////////////////////////////
RocketRegion::
RocketRegion(GraphicsOutput *window, const LVecBase4 &dr_dimensions,
             const string &context_name) :
  DisplayRegion(window, dr_dimensions) {

  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rocket::Core::Vector2i dimensions (pr - pl, pt - pb);

  rocket_cat.debug()
    << "Setting initial context dimensions to ("
    << dimensions.x << ", " << dimensions.y << ")\n";

  _context = Rocket::Core::CreateContext(context_name.c_str(),
                                         dimensions, &_interface);
  nassertv(_context != NULL);

  _lens = new OrthographicLens;
  _lens->set_film_size(dimensions.x, -dimensions.y);
  _lens->set_film_offset(dimensions.x * 0.5, dimensions.y * 0.5);
  _lens->set_near_far(-1, 1);
  set_camera(new Camera(context_name, _lens));
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRegion::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
RocketRegion::
~RocketRegion() {
  if (_context != NULL) {
    if (_context->GetReferenceCount() > 1) {
      _context->RemoveReference();
      return;
    }

    // We need to do this because libRocket may call into Python
    // code to throw destruction events.
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

////////////////////////////////////////////////////////////////////
//     Function: RocketRegion::do_cull
//       Access: Protected, Virtual
//  Description: Performs a cull traversal for this region.
////////////////////////////////////////////////////////////////////
void RocketRegion::
do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg, Thread *current_thread) {

  PStatTimer timer(get_cull_region_pcollector(), current_thread);

  // We (unfortunately) need to do this because libRocket
  // may call into Python code to throw events.
#ifdef HAVE_ROCKET_PYTHON
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  int pl, pr, pb, pt;
  get_pixels(pl, pr, pb, pt);
  Rocket::Core::Vector2i dimensions (pr - pl, pt - pb);

  if (_context->GetDimensions() != dimensions) {
    rocket_cat.debug() << "Setting context dimensions to ("
      << dimensions.x << ", " << dimensions.y << ")\n";

    _context->SetDimensions(dimensions);

    _lens->set_film_size(dimensions.x, -dimensions.y);
    _lens->set_film_offset(dimensions.x * 0.5, dimensions.y * 0.5);
  }

  if (_input_handler != NULL) {
    _input_handler->update_context(_context, pl, pb);
  } else {
    _context->Update();
  }

  CullTraverser *trav = get_cull_traverser();
  trav->set_cull_handler(cull_handler);
  trav->set_scene(scene_setup, gsg, get_incomplete_render());
  trav->set_view_frustum(NULL);

  _interface.render(_context, trav);

#ifdef HAVE_ROCKET_PYTHON
  PyGILState_Release(gstate);
#endif

  trav->end_traverse();
}
