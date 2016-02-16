/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsWindow.I
 * @author rdb
 * @date 2012-05-14
 */

/**
 * Returns a pointer to the underlying NSWindow.
 */
INLINE NSWindow *CocoaGraphicsWindow::
get_nswindow() const {
  return _window;
}

/**
 * Returns a pointer to the underlying NSView.
 */
INLINE NSView *CocoaGraphicsWindow::
get_nsview() const {
  return _view;
}
