// Filename: wdxGraphicsPipe.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "wdxGraphicsPipe8.h"
#include "config_wdxdisplay8.h"
#include <mouseButton.h>
#include <keyboardButton.h>
#include <dxerr8.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsPipe::_type_handle;

//wdxGraphicsPipe *global_pipe;

wdxGraphicsPipe::wdxGraphicsPipe(const PipeSpecifier& spec)
: InteractiveGraphicsPipe(spec) {
//    _width = GetSystemMetrics(SM_CXSCREEN);
//    _height = GetSystemMetrics(SM_CYSCREEN);
    _shift = false;
//  global_pipe = this;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsPipe::
get_window_type() const {
    return wdxGraphicsWindow::get_class_type();
}

GraphicsPipe *wdxGraphicsPipe::
make_wdxGraphicsPipe(const FactoryParams &params) {
    GraphicsPipe::PipeSpec *pipe_param;
    if(!get_param_into(pipe_param, params)) {
        return new wdxGraphicsPipe(PipeSpecifier());
    } else {
        return new wdxGraphicsPipe(pipe_param->get_specifier());
    }
}

TypeHandle wdxGraphicsPipe::get_class_type(void) {
    return _type_handle;
}

const char *pipe_type_name="wdxGraphicsPipe";

void wdxGraphicsPipe::init_type(void) {
    InteractiveGraphicsPipe::init_type();
    register_type(_type_handle, pipe_type_name,
                  InteractiveGraphicsPipe::get_class_type());
}

TypeHandle wdxGraphicsPipe::get_type(void) const {
    return get_class_type();
}

wdxGraphicsPipe::wdxGraphicsPipe(void) {
    wdxdisplay_cat.error()
    << pipe_type_name <<"s should not be created with the default constructor" << endl;
}

wdxGraphicsPipe::wdxGraphicsPipe(const wdxGraphicsPipe&) {
    wdxdisplay_cat.error()
    << pipe_type_name << "s should not be copied" << endl;
}

wdxGraphicsPipe& wdxGraphicsPipe::operator=(const wdxGraphicsPipe&) {
    wdxdisplay_cat.error()
    << pipe_type_name << "s should not be assigned" << endl;
    return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: find_window
//       Access:
//  Description: Find the window that has the xwindow "win" in the
//       window list for the pipe (if it exists)
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow *wdxGraphicsPipe::
find_window(HWND win) {
    int num_windows = get_num_windows();
    for(int w = 0; w < num_windows; w++) {
        wdxGraphicsWindow *window = DCAST(wdxGraphicsWindow, get_window(w));
        if((window->_dxgsg!=NULL) && (window->_dxgsg->scrn.hWnd == win))
            return window;
    }
    return NULL;
}


