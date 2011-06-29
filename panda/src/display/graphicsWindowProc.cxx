// Filename: graphicsWindowProc.cxx
// Created by:  Bei (Mar2010)
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

#include "graphicsWindowProc.h"

////////////////////////////////////////////////////////////////////
//     Function: GraphicWindowProc::Constructor
//       Access: Public
//  Description: Does nothing.
////////////////////////////////////////////////////////////////////
GraphicsWindowProc::
GraphicsWindowProc(){
}

#if defined(__WIN32__) || defined(_WIN32)

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProc::wnd_proc
//       Access: Public, Virtual
//  Description: A WIN32-specific method that is called when a Window
//               proc event occurrs. Should be overridden by a derived
//               class.
////////////////////////////////////////////////////////////////////
LONG GraphicsWindowProc::
wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
  return 0;
}

#endif
//most an empty file.
