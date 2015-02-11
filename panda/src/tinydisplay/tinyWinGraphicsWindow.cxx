// Filename: tinyWinGraphicsWindow.cxx
// Created by:  drose (06May08)
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

#include "pandabase.h"

#ifdef WIN32

#include "tinyWinGraphicsWindow.h"
#include "config_tinydisplay.h"
#include "config_windisplay.h"
#include "tinyWinGraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"
#include "pStatTimer.h"

#include <wingdi.h>

TypeHandle TinyWinGraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyWinGraphicsWindow::
TinyWinGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host) :
  WinGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _frame_buffer = NULL;
  _hdc = (HDC)0;
  update_pixel_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TinyWinGraphicsWindow::
~TinyWinGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool TinyWinGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  if (!get_unexposed_draw() && !_got_expose_event) {
    if (tinydisplay_cat.is_spam()) {
      tinydisplay_cat.spam()
        << "Not drawing " << this << ": unexposed.\n";
    }
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _frame_buffer;
  tinygsg->reset_if_new();

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
end_flip() {
  if (!_flip_ready) {
    GraphicsWindow::end_flip();
    return;
  }

  HBITMAP bm = CreateCompatibleBitmap(_hdc, _frame_buffer->xsize, _frame_buffer->ysize);
  HDC bmdc = CreateCompatibleDC(_hdc);
  SelectObject(bmdc, bm);

  int fb_xsize = get_fb_x_size();
  int fb_ysize = get_fb_y_size();
  int fb_ytop = _frame_buffer->ysize - fb_ysize;

  SetDIBits(_hdc, bm, fb_ytop, fb_ysize, _frame_buffer->pbuf,
            &_bitmap_info, DIB_RGB_COLORS);

  if (fb_xsize == _frame_buffer->xsize) {
    BitBlt(_hdc, 0, 0, fb_xsize, fb_ysize,
           bmdc, 0, 0, SRCCOPY);
  } else {
    //    SetStretchBltMode(_hdc, HALFTONE);
    StretchBlt(_hdc, 0, 0, _frame_buffer->xsize, _frame_buffer->ysize,
               bmdc, 0, 0,fb_xsize, fb_ysize,
               SRCCOPY);
  }

  DeleteDC(bmdc);
  DeleteObject(bm);
  GdiFlush();
  GraphicsWindow::end_flip();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::supports_pixel_zoom
//       Access: Published, Virtual
//  Description: Returns true if a call to set_pixel_zoom() will be
//               respected, false if it will be ignored.  If this
//               returns false, then get_pixel_factor() will always
//               return 1.0, regardless of what value you specify for
//               set_pixel_zoom().
//
//               This may return false if the underlying renderer
//               doesn't support pixel zooming, or if you have called
//               this on a DisplayRegion that doesn't have both
//               set_clear_color() and set_clear_depth() enabled.
////////////////////////////////////////////////////////////////////
bool TinyWinGraphicsWindow::
supports_pixel_zoom() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
close_window() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    TinyGraphicsStateGuardian *tinygsg;
    DCAST_INTO_V(tinygsg, _gsg);
    tinygsg->_current_frame_buffer = NULL;
    _gsg.clear();
  }

  ReleaseDC(_hWnd, _hdc);
  _hdc = (HDC)0;
  WinGraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool TinyWinGraphicsWindow::
open_window() {
  if (!WinGraphicsWindow::open_window()) {
    return false;
  }

  // GSG Creation/Initialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, NULL);
    _gsg = tinygsg;
  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  _hdc = GetDC(_hWnd);

  create_frame_buffer();
  if (_frame_buffer == NULL) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _frame_buffer;

  tinygsg->reset_if_new();
  if (!tinygsg->is_valid()) {
    close_window();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_reshape
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
handle_reshape() {
  WinGraphicsWindow::handle_reshape();
  if (_frame_buffer != NULL) {
    ZB_resize(_frame_buffer, NULL, _properties.get_x_size(), _properties.get_y_size());
    setup_bitmap_info();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::do_fullscreen_resize
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
bool TinyWinGraphicsWindow::
do_fullscreen_resize(int x_size, int y_size) {
  bool result = WinGraphicsWindow::do_fullscreen_resize(x_size, y_size);
  ZB_resize(_frame_buffer, NULL, _properties.get_x_size(), _properties.get_y_size());
  setup_bitmap_info();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::create_frame_buffer
//       Access: Private
//  Description: Creates a suitable frame buffer for the current
//               window size.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
create_frame_buffer() {
  if (_frame_buffer != NULL) {
    ZB_close(_frame_buffer);
    _frame_buffer = NULL;
  }

  _frame_buffer = ZB_open(_properties.get_x_size(), _properties.get_y_size(), ZB_MODE_RGBA, 0, 0, 0, 0);
  setup_bitmap_info();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsWindow::setup_bitmap_info
//       Access: Private
//  Description: Determines the BITMAPINFO stuff for blitting the
//               frame buffer to the window.
////////////////////////////////////////////////////////////////////
void TinyWinGraphicsWindow::
setup_bitmap_info() {
  _bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  _bitmap_info.bmiHeader.biWidth = _frame_buffer->xsize;
  _bitmap_info.bmiHeader.biHeight = -_frame_buffer->ysize;
  _bitmap_info.bmiHeader.biPlanes = 1;
  _bitmap_info.bmiHeader.biBitCount = 32;
  _bitmap_info.bmiHeader.biCompression = BI_RGB;
  _bitmap_info.bmiHeader.biSizeImage = _frame_buffer->linesize * _frame_buffer->ysize;
  _bitmap_info.bmiHeader.biXPelsPerMeter = 0;
  _bitmap_info.bmiHeader.biYPelsPerMeter = 0;
  _bitmap_info.bmiHeader.biClrUsed = 0;
  _bitmap_info.bmiHeader.biClrImportant = 0;
}

#endif  // WIN32
