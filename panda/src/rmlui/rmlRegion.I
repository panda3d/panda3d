/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlRegion.I
 * @author rdb
 * @date 2011-11-30
 */

INLINE RmlRegion *RmlRegion::
make(const std::string &context_name, GraphicsOutput *window) {
  return make(context_name, window, LVecBase4(0.0f, 1.0f, 0.0f, 1.0f));
}

INLINE RmlRegion *RmlRegion::
make(const std::string &context_name, GraphicsOutput *window,
     const LVecBase4 &dimensions) {
  return new RmlRegion(window, dimensions, context_name);
}

INLINE void RmlRegion::
set_input_handler(RmlInputHandler *handler) {
  _input_handler = handler;
}

INLINE RmlInputHandler *RmlRegion::
get_input_handler() const {
  return _input_handler;
}
