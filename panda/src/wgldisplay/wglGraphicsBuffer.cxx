// Filename: wglGraphicsBuffer.cxx
// Created by:  drose (08Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  _pbuffer = (HPBUFFERARB)0;
  _pbuffer_dc = (HDC)0;
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

  // Use the pbuffer if we got it, otherwise fall back to the window.
  if (_pbuffer_dc) {
    wglMakeCurrent(_pbuffer_dc, wglgsg->get_context(_pbuffer_dc));

  } else {
    wglMakeCurrent(_window_dc, wglgsg->get_context(_window_dc));
  }
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

    if (_pbuffer_dc) {
      SwapBuffers(_pbuffer_dc);
    } else {
      SwapBuffers(_window_dc);
    }
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

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);

    if (_pbuffer_dc) {
      wglgsg->_wglReleasePbufferDCARB(_pbuffer, _pbuffer_dc);
    }
    if (_pbuffer) {
      wglgsg->_wglDestroyPbufferARB(_pbuffer);
    }
  }
  _pbuffer_dc = 0;
  _pbuffer = 0;

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
  if (!make_window()) {
    // If we couldn't make a window, we can't get a GL context.
    return false;
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  wglMakeCurrent(_window_dc, wglgsg->get_context(_window_dc));
  wglgsg->reset_if_new();

  // Now that we have fully made a window and used that window to
  // create a rendering context, we can attempt to create a pbuffer.
  // This might fail if the pbuffer extensions are not supported; in
  // that case, we'll just keep the window and hope it works even if
  // it is not shown.

  if (make_pbuffer()) {
    _pbuffer_dc = wglgsg->_wglGetPbufferDCARB(_pbuffer);
    wgldisplay_cat.info()
      << "Created PBuffer " << _pbuffer << ", DC " << _pbuffer_dc << "\n";

    wglMakeCurrent(_pbuffer_dc, wglgsg->get_context(_pbuffer_dc));
    wglgsg->report_gl_errors();

    // Now that the pbuffer is created, we don't need the window any
    // more.
    if (_window_dc) {
      ReleaseDC(_window, _window_dc);
      _window_dc = 0;
    }
    if (_window) {
      DestroyWindow(_window);
      _window = 0;
    }
  }

  _is_valid = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::make_window
//       Access: Private
//  Description: Creates an invisible window to associate with the GL
//               context, even if we are not going to use it.  This is
//               necessary because in the Windows OpenGL API, we have
//               to create window before we can create a GL
//               context--even before we can ask about what GL
//               extensions are available!
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
make_window() {
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

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::make_pbuffer
//       Access: Private
//  Description: Once the GL context has been fully realized, attempts
//               to create an offscreen pbuffer if the graphics API
//               supports it.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
make_pbuffer() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  if (!wglgsg->_supports_pbuffer) {
    return false;
  }

  int pbformat = wglgsg->get_pfnum();

  if (wglgsg->_supports_pixel_format) {
    // Select a suitable pixel format that matches the GSG's existing
    // format, and also is appropriate for a pixel buffer.
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;

    DescribePixelFormat(_window_dc, wglgsg->get_pfnum(), 
                        sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    static const int max_attrib_list = 32;
    int iattrib_list[max_attrib_list];
    float fattrib_list[max_attrib_list];
    int ni = 0;
    int nf = 0;

    // Since we are trying to create a pbuffer, the pixel format we
    // request (and subsequently use) must be "pbuffer capable".
    iattrib_list[ni++] = WGL_DRAW_TO_PBUFFER_ARB;
    iattrib_list[ni++] = true;

    // Match up the framebuffer bits.
    iattrib_list[ni++] = WGL_RED_BITS_ARB;
    iattrib_list[ni++] = pfd.cRedBits;
    iattrib_list[ni++] = WGL_GREEN_BITS_ARB;
    iattrib_list[ni++] = pfd.cGreenBits;
    iattrib_list[ni++] = WGL_BLUE_BITS_ARB;
    iattrib_list[ni++] = pfd.cBlueBits;
    iattrib_list[ni++] = WGL_ALPHA_BITS_ARB;
    iattrib_list[ni++] = pfd.cAlphaBits;

    iattrib_list[ni++] = WGL_DEPTH_BITS_ARB;
    iattrib_list[ni++] = pfd.cDepthBits;

    iattrib_list[ni++] = WGL_STENCIL_BITS_ARB;
    iattrib_list[ni++] = pfd.cStencilBits;

    // Terminate the lists.
    nassertr(ni < max_attrib_list && nf < max_attrib_list, NULL);
    iattrib_list[ni] = 0;
    fattrib_list[nf] = 0;

    // Now obtain a list of pixel formats that meet these minimum
    // requirements.
    static const int max_pformats = 32;
    int pformat[max_pformats];
    memset(pformat, 0, sizeof(pformat));
    unsigned int nformats = 0;
    if (!wglgsg->_wglChoosePixelFormatARB(_window_dc, iattrib_list, fattrib_list,
                                          max_pformats, pformat, &nformats)) {
      wgldisplay_cat.info()
        << "Couldn't find a suitable pixel format for creating a pbuffer.\n";
      return false;
    }

    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat.debug()
        << "Found " << nformats << " pbuffer formats: [";
      for (unsigned int i = 0; i < nformats; i++) {
        wgldisplay_cat.debug(false)
          << " " << pformat[i];
      }
      wgldisplay_cat.debug(false)
        << " ]\n";
    }

    pbformat = pformat[0];
  }
  
  int attrib_list[] = {
    0,
  };
  
  _pbuffer = wglgsg->_wglCreatePbufferARB(_window_dc, pbformat, 
                                          _x_size, _y_size, attrib_list);
  
  if (_pbuffer == 0) {
    wgldisplay_cat.info()
      << "Attempt to create pbuffer failed.\n";
    return false;
  }

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


