// Filename: wglGraphicsBuffer.cxx
// Created by:  drose (08Feb04)
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

#include "wglGraphicsBuffer.h"
#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#include "glgsg.h"
#include "frameBufferProperties.h"

#include <wingdi.h>

TypeHandle wglGraphicsBuffer::_type_handle;
const char * const wglGraphicsBuffer::_window_class_name = "wglGraphicsBuffer";
bool wglGraphicsBuffer::_window_class_registered = false;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
wglGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  int x_size, int y_size, bool want_texture) :
  GraphicsBuffer(pipe, gsg, x_size, y_size, want_texture) 
{
  _window = (HWND)0;
  _window_dc = (HDC)0;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
~wglGraphicsBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
make_current() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);
  wglMakeCurrent(_window_dc, wglgsg->get_context(_window_dc));

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  wglgsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
release_gsg() {
  GraphicsBuffer::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
begin_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    make_current();
    glFinish();

    if (has_texture()) {
      // Use glCopyTexImage2D to copy the framebuffer to the texture.
      // This appears to be the only way to "render to a texture" in
      // OpenGL; there's no interface to make the offscreen buffer
      // itself be a texture.
      PT(DisplayRegion) dr = make_scratch_display_region(_x_size, _y_size);
      get_texture()->copy(_gsg, dr, _gsg->get_render_buffer(RenderBuffer::T_back));
    }

    SwapBuffers(_window_dc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_events() {
  GraphicsBuffer::process_events();

  MSG msg;
    
  // Handle all the messages on the queue in a row.  Some of these
  // might be for another window, but they will get dispatched
  // appropriately.
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
close_buffer() {
  if (_window_dc) {
    ReleaseDC(_window, _window_dc);
    _window_dc = 0;
  }
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
open_buffer() {
  // I made a good solid effort to use the wglPbuffer extension.  Not
  // only are wgl extensions incredibly convoluted to get to, but the
  // pbuffer extension turned out not be supported on the Intel card I
  // tried it on, and crashed the driver for the nVidia card I tried
  // it on.  And it's not even supported on the Microsoft software
  // reference implementation.  Not a good record.

  // In lieu of the pbuffer extension, it appears that rendering to a
  // window that is hidden works fine on some drivers (although it
  // doesn't work on other drivers).

  DWORD window_style = WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;

  RECT win_rect;
  SetRect(&win_rect, 0, 0, _x_size, _y_size);
  
  // compute window size based on desired client area size
  if (!AdjustWindowRect(&win_rect, window_style, FALSE)) {
    wgldisplay_cat.error()
      << "AdjustWindowRect failed!" << endl;
    return false;
  }

  register_window_class();
  HINSTANCE hinstance = GetModuleHandle(NULL);
  _window = CreateWindow(_window_class_name, "buffer", window_style, 
                         win_rect.left, win_rect.top,
                         win_rect.right - win_rect.left,
                         win_rect.bottom - win_rect.top,
                         NULL, NULL, hinstance, 0);
  
  if (!_window) {
    wgldisplay_cat.error()
      << "CreateWindow() failed!" << endl;
    return false;
  }

  ShowWindow(_window, SW_SHOWNORMAL);
  if (!show_pbuffers) {
    ShowWindow(_window, SW_HIDE);
  }

  _window_dc = GetDC(_window);

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);
  int pfnum = wglgsg->get_pfnum();
  PIXELFORMATDESCRIPTOR pixelformat;
  if (!SetPixelFormat(_window_dc, pfnum, &pixelformat)) {
    wgldisplay_cat.error()
      << "SetPixelFormat(" << pfnum << ") failed after window create\n";
    return false;
  }

  _is_valid = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    //    DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}
  
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::register_window_class
//       Access: Private, Static
//  Description: Registers a Window class for all wglGraphicsBuffers.
//               This only needs to be done once per session.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
register_window_class() {
  if (_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  HINSTANCE instance = GetModuleHandle(NULL);

  // Clear before filling in window structure!
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = static_window_proc;
  wc.hInstance = instance;
  wc.lpszClassName = _window_class_name;
  
  if (!RegisterClass(&wc)) {
    wgldisplay_cat.error()
      << "could not register window class!" << endl;
    return;
  }
  _window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::static_window_proc
//       Access: Private, Static
//  Description: This is attached to the window class for all
//               wglGraphicsBuffer windows; it is called to handle
//               window events.
////////////////////////////////////////////////////////////////////
LONG WINAPI wglGraphicsBuffer::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return DefWindowProc(hwnd, msg, wparam, lparam);
}





/*
    if (wglgsg->_supports_pixel_format) {
      // Get ready to query for a suitable pixel format that meets our
      // minimum requirements.
      static const int MAX_ATTRIBS = 128;
      static const int MAX_PFORMATS = 128;
      int iattributes[2*MAX_ATTRIBS];
      float fattributes[2*MAX_ATTRIBS];
      int nfattribs = 0;
      int niattribs = 0;
      // Attribute arrays must be "0" terminated--for simplicity, first
    // just zero-out the array then fill from left to right.
      for ( int a = 0; a < 2*MAX_ATTRIBS; a++ )
        {
          iattributes[a] = 0;
          fattributes[a] = 0;
        }
      // Since we are trying to create a pbuffer, the pixel format we
      // request (and subsequently use) must be "pbuffer capable".
      iattributes[2*niattribs ] = WGL_DRAW_TO_PBUFFER_ARB;
      iattributes[2*niattribs+1] = true;
      niattribs++;
      // We require a minimum of 24-bit depth.
      iattributes[2*niattribs ] = WGL_DEPTH_BITS_ARB;
      iattributes[2*niattribs+1] = 24;
      niattribs++;
      // We require a minimum of 8-bits for each R, G, B, and A.
      iattributes[2*niattribs ] = WGL_RED_BITS_ARB;
      iattributes[2*niattribs+1] = 8;
      niattribs++;
      iattributes[2*niattribs ] = WGL_GREEN_BITS_ARB;
      iattributes[2*niattribs+1] = 8;
      niattribs++;
      iattributes[2*niattribs ] = WGL_BLUE_BITS_ARB;
      iattributes[2*niattribs+1] = 8;
      niattribs++;
      iattributes[2*niattribs ] = WGL_ALPHA_BITS_ARB;
      iattributes[2*niattribs+1] = 8;
      niattribs++;
      // Now obtain a list of pixel formats that meet these minimum
      // requirements.
      int pformat[MAX_PFORMATS];
      memset(pformat, 0, sizeof(pformat));
      unsigned int nformats = 0;
      if (!wglgsg->_wglChoosePixelFormatARB(_window_dc, iattributes, fattributes,
                                            MAX_PFORMATS, pformat, &nformats)) {
        cerr << "pbuffer creation error: Couldn't find a suitable pixel format.\n";
      } else {
        cerr << "Found " << nformats << " formats, selecting " << pformat[0] << "\n";
        pbformat = pformat[0];
      }
    }

    HPBUFFERARB _pbuffer;
    wglMakeCurrent(_window_dc, wglgsg->get_context(_window_dc));
    wglgsg->reset_if_new();

    cerr << "Creating pbuffer(" << _window_dc << ", " << wglgsg->get_pfnum()
         << ", " << _x_size << ", " << _y_size << ", ...)\n";
    int attrib_list[] = {
      0,
    };

    HDC hdc = wglGetCurrentDC();
    cerr << "current dc = " << hdc << " window dc = " << _window_dc << "\n";
    _pbuffer = wglgsg->_wglCreatePbufferARB(hdc, pbformat, 
                                            _x_size, _y_size, attrib_list);
    cerr << "pbuffer = " << (void *)_pbuffer << "\n";

    if (_pbuffer == NULL) {
      wgldisplay_cat.error()
        << "Attempt to create pbuffer failed.\n";
    }
*/
