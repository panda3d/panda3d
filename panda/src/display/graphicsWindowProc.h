// Filename: graphicswindowProc.h
// Created by:  Bei Yang (Mar 2010)
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


#ifndef GRAPHICSWINDOWPROC_H
#define GRAPHICSWINDOWPROC_H

#include "pandabase.h"
#include "typedReferenceCount.h"

#if defined(__WIN32__) || defined(_WIN32)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN 1
	#endif
	#include <windows.h>
#endif

/*
Defines a little interface for storing a platform specific window
processor methods.  Since this is a purely virtual class, it never
really gets instaniated so this even though this is type reference
counted, it doesn't need to registered.
*/
class EXPCL_PANDA_DISPLAY GraphicsWindowProc: public TypedReferenceCount{
public:
	GraphicsWindowProc();
#if defined(__WIN32__) || defined(_WIN32)
	virtual LONG wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif
	//purely virtual class

	// In theory, this stuff below never gets used since it's purely virtual
	// class that can't be instaniated anyways
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsWindowProc",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif //GRAPHICSWINDOWPROC_H