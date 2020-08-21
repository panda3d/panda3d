/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsWindow.cxx
 * @author drose
 * @date 2002-12-20
 */

#include "wglGraphicsWindow.h"
#include "config_wgldisplay.h"
#include "config_windisplay.h"
#include "wglGraphicsPipe.h"
#include "pStatTimer.h"
#include "glgsg.h"
#include "wglGraphicsStateGuardian.h"

#include <wingdi.h>

TypeHandle wglGraphicsWindow::_type_handle;

/**
 *
 */
wglGraphicsWindow::
wglGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const std::string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  WinGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _hdc = (HDC)0;
}

/**
 *
 */
wglGraphicsWindow::
~wglGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool wglGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  if (!get_unexposed_draw() && !_got_expose_event) {
    if (wgldisplay_cat.is_spam()) {
      wgldisplay_cat.spam()
        << "Not drawing " << this << ": unexposed.\n";
    }
    return false;
  }

  if (wgldisplay_cat.is_spam()) {
    wgldisplay_cat.spam()
      << "Drawing " << this << ": exposed.\n";
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  HGLRC context = wglgsg->get_context(_hdc);
  nassertr(context, false);

  if (!wglGraphicsPipe::wgl_make_current(_hdc, context, &_make_current_pcollector)) {
    wgldisplay_cat.error()
      << "Failed to make WGL context current.\n";
    return false;
  }
  wglgsg->reset_if_new();

  if (mode == FM_render) {
    wglgsg->push_group_marker(std::string("wglGraphicsWindow ") + get_name());
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void wglGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();

    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);
    wglgsg->pop_group_marker();
  }
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void wglGraphicsWindow::
begin_flip() {
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip when command, but
 * will not actually flip
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void wglGraphicsWindow::
ready_flip() {
  if (_hdc) {
    // The documentation on SwapBuffers() is not at all clear on whether the
    // GL context needs to be current before it can be called.  Empirically,
    // it appears that it is not necessary in many cases, but it definitely is
    // necessary at least in the case of Mesa on Windows.
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);
    HGLRC context = wglgsg->get_context(_hdc);
    nassertv(context);
    wglGraphicsPipe::wgl_make_current(_hdc, context, &_make_current_pcollector);
    wglgsg->finish();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void wglGraphicsWindow::
end_flip() {
  if (_hdc != nullptr && _flip_ready) {
    // The documentation on SwapBuffers() is not at all clear on whether the
    // GL context needs to be current before it can be called.  Empirically,
    // it appears that it is not necessary in many cases, but it definitely is
    // necessary at least in the case of Mesa on Windows.
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);
    HGLRC context = wglgsg->get_context(_hdc);
    nassertv(context);
    wglGraphicsPipe::wgl_make_current(_hdc, context, &_make_current_pcollector);
    SwapBuffers(_hdc);
  }
  WinGraphicsWindow::end_flip();
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void wglGraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    wglGraphicsPipe::wgl_make_current(_hdc, nullptr, &_make_current_pcollector);
    _gsg.clear();
  }
  ReleaseDC(_hWnd, _hdc);
  _hdc = (HDC)0;
  WinGraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool wglGraphicsWindow::
open_window() {
  if (!WinGraphicsWindow::open_window()) {
    return false;
  }

  // GSG creationinitialization.

  wglGraphicsStateGuardian *wglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    wglgsg = new wglGraphicsStateGuardian(_engine, _pipe, nullptr);
    wglgsg->choose_pixel_format(_fb_properties, false);
    _gsg = wglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(wglgsg, _gsg, false);
    if (!wglgsg->get_fb_properties().subsumes(_fb_properties)) {
      wglgsg = new wglGraphicsStateGuardian(_engine, _pipe, wglgsg);
      wglgsg->choose_pixel_format(_fb_properties, false);
      _gsg = wglgsg;
    }
  }

  // Set up the pixel format of the window appropriately for GL.

  _hdc = GetDC(_hWnd);
  int pfnum = wglgsg->get_pfnum();
  PIXELFORMATDESCRIPTOR pixelformat;
  DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR),
                      &pixelformat);

#ifdef NOTIFY_DEBUG
  char msg[200];
  sprintf(msg, "Selected GL PixelFormat is #%d", pfnum);
  print_pfd(&pixelformat, msg);
#endif

  BOOL set_pfnum = SetPixelFormat(_hdc, pfnum, &pixelformat);

  if (!set_pfnum) {
    if (wglgsg->fail_pfnum()) {
      wgldisplay_cat.error()
        << "SetPixelFormat(" << pfnum << ") failed; trying "
        << wglgsg->get_pfnum() << " instead\n";

      pfnum = wglgsg->get_pfnum();
      DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR),
                          &pixelformat);

#ifdef NOTIFY_DEBUG
      sprintf(msg, "Selected GL PixelFormat is #%d", pfnum);
      print_pfd(&pixelformat, msg);
#endif

      DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR),
                          &pixelformat);
      set_pfnum = SetPixelFormat(_hdc, pfnum, &pixelformat);
    }
  }

  if (!set_pfnum) {
    wgldisplay_cat.error()
      << "SetPixelFormat(" << pfnum << ") failed after window create\n";
    close_window();
    return false;
  }

#ifndef NDEBUG
  if (gl_force_invalid) {
    wgldisplay_cat.error()
      << "Artificially failing window.\n";
    close_window();
    return false;
  }
#endif  // NDEBUG

  // Initializes _colormap
  setup_colormap(pixelformat);

  // Make sure we have a context created.
  HGLRC context = wglgsg->get_context(_hdc);
  if (!context) {
    // The context failed to create for some reason.
    wgldisplay_cat.error()
      << "Closing window because no valid context is available.\n";
    close_window();
    return false;
  }

  // Initialize the gsg.
  wglGraphicsPipe::wgl_make_current(_hdc, context, &_make_current_pcollector);
  wglgsg->reset_if_new();
  wglgsg->report_my_gl_errors();
  if (!wglgsg->get_fb_properties().verify_hardware_software
      (_fb_properties,wglgsg->get_gl_renderer())) {
    close_window();
    return false;
  }
  _fb_properties = wglgsg->get_fb_properties();

  return true;
}

/**
 * Sets up a colormap for the window matching the selected pixel format.  This
 * is necessary before creating a GL context.
 */
void wglGraphicsWindow::
setup_colormap(const PIXELFORMATDESCRIPTOR &pixelformat) {
  LOGPALETTE *logical;
  int n;

  if (!(pixelformat.dwFlags & PFD_NEED_PALETTE ||
      pixelformat.iPixelType == PFD_TYPE_COLORINDEX))
    return;

  n = 1 << pixelformat.cColorBits;

  /* allocate a bunch of memory for the logical palette (assume 256
     colors in a Win32 palette */
  logical = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +
                                sizeof(PALETTEENTRY) * n);
  memset(logical, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);

  /* set the entries in the logical palette */
  logical->palVersion = 0x300;
  logical->palNumEntries = n;

  /* start with a copy of the current system palette */
  GetSystemPaletteEntries(_hdc, 0, 256, &logical->palPalEntry[0]);

  if (pixelformat.iPixelType == PFD_TYPE_RGBA) {
    int redMask = (1 << pixelformat.cRedBits) - 1;
    int greenMask = (1 << pixelformat.cGreenBits) - 1;
    int blueMask = (1 << pixelformat.cBlueBits) - 1;
    int i;

    /* fill in an RGBA color palette */
    for (i = 0; i < n; ++i) {
      logical->palPalEntry[i].peRed =
        (((i >> pixelformat.cRedShift)   & redMask)   * 255) / redMask;
      logical->palPalEntry[i].peGreen =
        (((i >> pixelformat.cGreenShift) & greenMask) * 255) / greenMask;
        logical->palPalEntry[i].peBlue =
        (((i >> pixelformat.cBlueShift)  & blueMask)  * 255) / blueMask;
      logical->palPalEntry[i].peFlags = 0;
    }
  }

  _colormap = CreatePalette(logical);
  free(logical);

  SelectPalette(_hdc, _colormap, FALSE);
  RealizePalette(_hdc);
}

#ifdef NOTIFY_DEBUG

typedef enum {Software, MCD, ICD} OGLDriverType;
static const char *OGLDrvStrings[3] = {"Software","MCD","ICD"};

/**
 * Reports information about the selected pixel format descriptor, along with
 * the indicated message.
 */
void wglGraphicsWindow::
print_pfd(PIXELFORMATDESCRIPTOR *pfd, char *msg) {
  if (!wgldisplay_cat.is_debug()) {
    return;
  }

  OGLDriverType drvtype;
  if ((pfd->dwFlags & PFD_GENERIC_ACCELERATED) &&
      (pfd->dwFlags & PFD_GENERIC_FORMAT)) {
    drvtype=MCD;
  } else if (!(pfd->dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd->dwFlags & PFD_GENERIC_FORMAT)) {
    drvtype=ICD;
  } else {
    drvtype=Software;
  }

#define PRINT_FLAG(FLG) ((pfd->dwFlags &  PFD_##FLG) ? (" PFD_" #FLG "|") : "")
  wgldisplay_cat.debug()
    << "================================\n";

  wgldisplay_cat.debug()
    << msg << ", " << OGLDrvStrings[drvtype] << " driver\n"
    << "PFD flags: 0x" << std::hex << pfd->dwFlags << std::dec << " ("
    << PRINT_FLAG(GENERIC_ACCELERATED)
    << PRINT_FLAG(GENERIC_FORMAT)
    << PRINT_FLAG(DOUBLEBUFFER)
    << PRINT_FLAG(SUPPORT_OPENGL)
    << PRINT_FLAG(SUPPORT_GDI)
    << PRINT_FLAG(STEREO)
    << PRINT_FLAG(DRAW_TO_WINDOW)
    << PRINT_FLAG(DRAW_TO_BITMAP)
    << PRINT_FLAG(SWAP_EXCHANGE)
    << PRINT_FLAG(SWAP_COPY)
    << PRINT_FLAG(SWAP_LAYER_BUFFERS)
    << PRINT_FLAG(NEED_PALETTE)
    << PRINT_FLAG(NEED_SYSTEM_PALETTE)
    << PRINT_FLAG(SUPPORT_DIRECTDRAW) << ")\n"
    << "PFD iPixelType: "
    << ((pfd->iPixelType==PFD_TYPE_RGBA) ? "PFD_TYPE_RGBA":"PFD_TYPE_COLORINDEX")
    << std::endl
    << "PFD cColorBits: " << (DWORD)pfd->cColorBits
    << "  R: " << (DWORD)pfd->cRedBits
    <<" G: " << (DWORD)pfd->cGreenBits
    <<" B: " << (DWORD)pfd->cBlueBits << std::endl
    << "PFD cAlphaBits: " << (DWORD)pfd->cAlphaBits
    << "  DepthBits: " << (DWORD)pfd->cDepthBits
    <<" StencilBits: " << (DWORD)pfd->cStencilBits
    <<" AccumBits: " << (DWORD)pfd->cAccumBits
    << std::endl;
}
#endif
