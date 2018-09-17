/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRegion.I
 * @author rdb
 * @date 2011-12-01
 */

/**
 * Creates a libRocket context with the given name (must be unique) for the
 * given window.  Returns a new DisplayRegion where the libRocket context will
 * render to.  This variant of make() creates a region that fills the entire
 * window.
 */
INLINE RocketRegion *RocketRegion::
make(const std::string &context_name, GraphicsOutput *window) {
  return make(context_name, window, LVecBase4(0.0f, 1.0f, 0.0f, 1.0f));
}

/**
 * Creates a libRocket context with the given name (must be unique) for the
 * given window.  Returns a new DisplayRegion where the libRocket context will
 * render to.
 */
INLINE RocketRegion *RocketRegion::
make(const std::string &context_name, GraphicsOutput *window,
     const LVecBase4 &dimensions) {

  return new RocketRegion(window, dimensions, context_name);
}

#ifndef CPPPARSER
/**
 * Returns a pointer to the Rocket context associated with this region.  Does
 * not increase the reference count, so if you want to preserve it for longer
 * than this region exists, be sure to call AddReference() and
 * RemoveReference() yourself.
 */
Rocket::Core::Context* RocketRegion::
get_context() const {
  return _context;
}
#endif

/**
 * Sets the RocketInputHandler object from which to gather the keyboard and
 * mouse inputs to pass to the libRocket context before rendering.  You may
 * use one input handler to provide inputs to any number of RocketRegion
 * objects.
 */
INLINE void RocketRegion::
set_input_handler(RocketInputHandler *handler) {
  _input_handler = handler;
}

/**
 * Returns the RocketInputHandler object previously passed to
 * set_input_handler.
 */
INLINE RocketInputHandler *RocketRegion::
get_input_handler() const {
  return _input_handler;
}
