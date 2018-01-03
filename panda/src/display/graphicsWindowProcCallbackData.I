/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowProcCallbackData.I
 * @author Walt Destler
 * @date 2010-06
 */

/**
 *
 */
INLINE GraphicsWindowProcCallbackData::
GraphicsWindowProcCallbackData(GraphicsWindow* graphicsWindow){
  _graphicsWindow = graphicsWindow;
}

/**
 * Returns the GraphicsWindow that generated this event.
 */
INLINE GraphicsWindow* GraphicsWindowProcCallbackData::
get_graphics_window() const {
  return _graphicsWindow;
}

#ifdef WIN32

/**
 * Returns the Windows proc hwnd parameter.
 */
INLINE uintptr_t GraphicsWindowProcCallbackData::
get_hwnd() const {
  return _hwnd;
}

/**
 * Returns the Windows proc msg parameter.
 */
INLINE int GraphicsWindowProcCallbackData::
get_msg() const {
  return _msg;
}

/**
 * Returns the Windows proc wparam parameter.
 */
INLINE int GraphicsWindowProcCallbackData::
get_wparam() const {
  return _wparam;
}

/**
 * Returns the Windows proc lparam parameter.
 */
INLINE int GraphicsWindowProcCallbackData::
get_lparam() const {
  return _lparam;
}

/**
 * Sets the Windows proc hwnd parameter.
 */
INLINE void GraphicsWindowProcCallbackData::
set_hwnd(uintptr_t hwnd) {
  _hwnd = hwnd;
}

/**
 * Sets the Windows proc msg parameter.
 */
INLINE void GraphicsWindowProcCallbackData::
set_msg(int msg) {
  _msg = msg;
}

/**
 * Sets the Windows proc wparam parameter.
 */
INLINE void GraphicsWindowProcCallbackData::
set_wparam(int wparam) {
  _wparam = wparam;
}

/**
 * Sets the Windows proc lparam parameter.
 */
INLINE void GraphicsWindowProcCallbackData::
set_lparam(int lparam) {
  _lparam = lparam;
}

#endif
