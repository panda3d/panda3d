/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowProc.cxx
 * @author Bei
 * @date 2010-03
 */

#include "graphicsWindowProc.h"

/**
 * Does nothing.
 */
GraphicsWindowProc::
GraphicsWindowProc(){
}

#if defined(__WIN32__) || defined(_WIN32)

/**
 * A WIN32-specific method that is called when a Window proc event occurrs.
 * Should be overridden by a derived class.
 */
LONG GraphicsWindowProc::
wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
  return 0;
}

#endif
//most an empty file.
