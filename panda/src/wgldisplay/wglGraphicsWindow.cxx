// Filename: wglGraphicsWindow.cxx
// Created by:  drose (20Dec02)
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

#include "wglGraphicsWindow.h"
#include "config_wgldisplay.h"
#include "config_windisplay.h"
#include "wglGraphicsPipe.h"
#include "glGraphicsStateGuardian.h"

#include <wingdi.h>
#include <ddraw.h>

TypeHandle wglGraphicsWindow::_type_handle;

static char *ConvDDErrorToString(const HRESULT &error);

////////////////////////////////////////////////////////////////////
//  Function: GetAvailVidMem
//  Description: Uses DDraw to get available video memory
////////////////////////////////////////////////////////////////////
static DWORD
GetAvailVidMem() {
  LPDIRECTDRAW2 pDD2;
  LPDIRECTDRAW pDD;
  HRESULT hr;

  typedef HRESULT (WINAPI *DIRECTDRAWCREATEPROC)(GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter);
  DIRECTDRAWCREATEPROC pfnDDCreate=NULL;
  
  HINSTANCE DDHinst = LoadLibrary("ddraw.dll");
  if (DDHinst == 0) {
    wgldisplay_cat.fatal() << "LoadLibrary() can't find DDRAW.DLL!" << endl;
    return 0x7FFFFFFF;
  }
  
  pfnDDCreate = (DIRECTDRAWCREATEPROC) GetProcAddress(DDHinst, "DirectDrawCreate");
  
  // just use DX5 DD interface, since that's the minimum ver we need
  if (NULL == pfnDDCreate) {
    wgldisplay_cat.fatal() << "GetProcAddress failed on DirectDrawCreate\n";
    FreeLibrary(DDHinst);
    return 0x7FFFFFFF;
  }
  
  // Create the Direct Draw Object
  hr = (*pfnDDCreate)((GUID *)DDCREATE_HARDWAREONLY, &pDD, NULL);
  if (hr != DD_OK) {
    wgldisplay_cat.fatal()
      << "DirectDrawCreate failed : result = " << ConvDDErrorToString(hr)
      << endl;
    FreeLibrary(DDHinst);
    return 0x7FFFFFFF;
  }
  
  FreeLibrary(DDHinst);    //undo LoadLib above, decrement ddrawl.dll refcnt (after DDrawCreate, since dont want to unload/reload)
  
  // need DDraw2 interface for GetAvailVidMem
  hr = pDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&pDD2);
  pDD->Release();

  if (hr != DD_OK) {
    wgldisplay_cat.fatal()
      << "DDraw QueryInterface failed : result = " << ConvDDErrorToString(hr)
      << endl;
    return 0x7FFFFFFF;
  }
  
  // Now we try to figure out if we can use requested screen
  // resolution and best rendertarget bpp and still have at least 2
  // meg of texture vidmem
  
  // Get Current VidMem avail.  Note this is only an estimate, when we
  // switch to fullscreen mode from desktop, more vidmem will be
  // available (typically 1.2 meg).  I dont want to switch to
  // fullscreen more than once due to the annoying monitor flicker, so
  // try to figure out optimal mode using this estimate
  DDSCAPS ddsCaps;
  DWORD dwTotal,dwFree;
  ZeroMemory(&ddsCaps,sizeof(DDSCAPS));
  ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;  // dont count AGP mem!
  if (FAILED(hr = pDD2->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
    if (hr==DDERR_NODIRECTDRAWHW) {
      wgldisplay_cat.info() << "GetAvailableVidMem returns no-DDraw HW, assuming we have plenty of vidmem\n";
      dwTotal=dwFree=0x7FFFFFFF;
    } else {
      wgldisplay_cat.fatal() << "GetAvailableVidMem failed : result = " << ConvDDErrorToString(hr) << endl;
      dwTotal=dwFree=0x7FFFFFFF;
    }
  } else {
    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat.info() << "before FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
    }

    if (dwTotal==0) {
      wgldisplay_cat.info() << "GetAvailVidMem returns bogus total of 0, assuming we have plenty of vidmem\n";
      dwTotal=dwFree=0x7FFFFFFF;
    }
  }
  
  pDD2->Release();  // bye-bye ddraw
  return dwFree;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::
wglGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) :
  WinGraphicsWindow(pipe, gsg) 
{
  _hdc = (HDC)0;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::
~wglGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
make_current() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);
  wglMakeCurrent(_hdc, wglgsg->get_context(_hdc));

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  wglgsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
release_gsg() {
  wglMakeCurrent(_hdc, NULL);
  GraphicsWindow::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::begin_flip
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
void wglGraphicsWindow::
begin_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    make_current();
    glFinish();
    SwapBuffers(_hdc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
close_window() {
  ReleaseDC(_hWnd, _hdc);
  _hdc = (HDC)0;
  WinGraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wglGraphicsWindow::
open_window() {
  if (!WinGraphicsWindow::open_window()) {
    return false;
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  _hdc = GetDC(_hWnd);

  // Set up the pixel format of the window appropriately for GL.
  int pfnum = wglgsg->get_pfnum();
  PIXELFORMATDESCRIPTOR pixelformat;
  DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), 
                      &pixelformat);

#ifdef _DEBUG
  char msg[200];
  sprintf(msg, "Selected GL PixelFormat is #%d", pfnum);
  print_pfd(&pixelformat, msg);
#endif

  if (!SetPixelFormat(_hdc, pfnum, &pixelformat)) {
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

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::reconsider_fullscreen_size
//       Access: Protected, Virtual
//  Description: Called before creating a fullscreen window to give
//               the driver a chance to adjust the particular
//               resolution request, if necessary.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size, DWORD &bitdepth) {
  // Here we arbitrarily insist that low-memory cards can only open
  // 640x480 fullscreen windows.  It would probably be better to make
  // this decision at a higher level, or at least allow the app to
  // decide whether it wants to yield this decision to us, but this
  // will do for now.

  // Actually, this test doesn't even work on the Riva128 (the driver
  // reports a bogus value of 0 bytes available memory), and the
  // Riva128 works fine at 800x600 mode anyway.  It's not clear
  // whether any cards we are actively supported are helped by this or
  // not.

#define LOWVIDMEMTHRESHOLD 3500000
  if (gl_do_vidmemsize_check && (GetAvailVidMem() < LOWVIDMEMTHRESHOLD)) {
    //_bIsLowVidMemCard = true;
    wgldisplay_cat.info()
      << "low video memory card detected, insisting on 640x480x16.\n";
    // we're going to need 640x480 at 16 bit to save enough tex vidmem
    x_size = 640;
    y_size = 480;
    bitdepth = 16;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::setup_colormap
//       Access: Private
//  Description: Sets up a colormap for the window matching the
//               selected pixel format.  This is necessary before
//               creating a GL context.
////////////////////////////////////////////////////////////////////
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

#ifdef _DEBUG
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::print_pfd
//       Access: Private, Static
//  Description: Reports information about the selected pixel format
//               descriptor, along with the indicated message.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
print_pfd(PIXELFORMATDESCRIPTOR *pfd, char *msg) {
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
  wgldisplay_cat.spam()
    << "================================\n";

  wgldisplay_cat.spam()
    << msg << ", " << OGLDrvStrings[drvtype] << " driver\n"
    << "PFD flags: 0x" << (void*)pfd->dwFlags << " (" 
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
    << endl
    << "PFD cColorBits: " << (DWORD)pfd->cColorBits
    << "  R: " << (DWORD)pfd->cRedBits
    <<" G: " << (DWORD)pfd->cGreenBits
    <<" B: " << (DWORD)pfd->cBlueBits << endl
    << "PFD cAlphaBits: " << (DWORD)pfd->cAlphaBits
    << "  DepthBits: " << (DWORD)pfd->cDepthBits
    <<" StencilBits: " << (DWORD)pfd->cStencilBits
    <<" AccumBits: " << (DWORD)pfd->cAccumBits
    << endl;
}
#endif

