// Filename: winGraphicsWindowProc.cxx
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

TypeHandle GraphicsWindowProc::_type_handle;

GraphicsWindowProc::GraphicsWindowProc(){
}
#if defined(__WIN32__) || defined(_WIN32)
LONG GraphicsWindowProc::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
	return 0;
}
#endif
//most an empty file.