/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowProc.h
 * @author Bei Yang
 * @date 2010-03
 */

#ifndef GRAPHICSWINDOWPROC_H
#define GRAPHICSWINDOWPROC_H

#include "pandabase.h"

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

class GraphicsWindow;

/**
 * Defines an interface for storing platform-specific window processor
 * methods.
 */
class EXPCL_PANDA_DISPLAY GraphicsWindowProc {
public:
  GraphicsWindowProc();
  virtual ~GraphicsWindowProc() = default;
#if defined(__WIN32__) || defined(_WIN32)
  virtual LONG wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd,
                        UINT msg, WPARAM wparam, LPARAM lparam);
#endif
  // purely virtual class
};

#endif  // GRAPHICSWINDOWPROC_H
