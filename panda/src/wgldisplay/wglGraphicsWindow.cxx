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

typedef enum {Software, MCD, ICD} OGLDriverType;
static const char * const OGLDrvStrings[] = { "Software", "MCD", "ICD" };

TypeHandle wglGraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::
wglGraphicsWindow(GraphicsPipe *pipe) :
  WinGraphicsWindow(pipe) 
{
  _context = (HGLRC)0;
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
//     Function: wglGraphicsWindow::make_gsg
//       Access: Public, Virtual
//  Description: Creates a new GSG for the window and stores it in the
//               _gsg pointer.  This should only be called from within
//               the draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
make_gsg() {
  nassertv(_gsg == (GraphicsStateGuardian *)NULL);

  _context = wglCreateContext(_hdc);
  if (!_context) {
    wgldisplay_cat.error()
      << "Could not create GL context.\n";
    return;
  }

  // And make sure the new context is current.
  wglMakeCurrent(_hdc, _context);

  // Now we can make a GSG.
  _gsg = new GLGraphicsStateGuardian(this);
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  This should only
//               be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
release_gsg() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    wglMakeCurrent(_hdc, _context);
    GraphicsWindow::release_gsg();
    wglDeleteContext(_context);
    wglMakeCurrent(_hdc, NULL);
    _context = (HGLRC)0;
  }
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
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  wglMakeCurrent(_hdc, _context);
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
    wglMakeCurrent(_hdc, _context);
    glFinish();
    SwapBuffers(_hdc);
  }
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

  // Set up the pixel format of the window appropriately for GL.
  _hdc = GetDC(_mwindow);

  int pfnum = choose_pfnum();

  if (gl_force_pixfmt != 0) {
    if (wgldisplay_cat.is_debug())
      wgldisplay_cat.debug()
        << "overriding pixfmt choice algorithm (" << pfnum 
        << ") with gl-force-pixfmt(" << gl_force_pixfmt << ")\n";
    pfnum = gl_force_pixfmt;
  }

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "config() - picking pixfmt #" << pfnum <<endl;
  }
  
  DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), 
                      &_pixelformat);

#ifdef _DEBUG
  char msg[200];
  sprintf(msg, "Selected GL PixelFormat is #%d", pfnum);
  print_pfd(&_pixelformat, msg);
#endif

  if (!SetPixelFormat(_hdc, pfnum, &_pixelformat)) {
    wgldisplay_cat.error()
      << "SetPixelFormat(" << pfnum << ") failed after window create\n";
    close_window();
    return false;
  }

  // Initializes _colormap
  setup_colormap();

  /*
  wgldisplay_cat.error()
    << "Artificially failing window.\n";
  close_window();
  return false;
  */

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
close_window() {
  ReleaseDC(_mwindow, _hdc);
  _hdc = (HDC)0;
  WinGraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::choose_pfnum
//       Access: Private
//  Description: Selects a suitable pixel format number for the
//               window, based on the requested properties.
////////////////////////////////////////////////////////////////////
int wglGraphicsWindow::
choose_pfnum() const {
  int pfnum;
  bool bUsingSoftware = false;

  if (force_software_renderer) {
    pfnum = find_pixfmtnum(false);
    
    if (pfnum == 0) {
      wgldisplay_cat.error()
        << "Couldn't find compatible software-renderer OpenGL pixfmt, check your window properties!\n";
      return 0;
    }
    bUsingSoftware = true;

  } else {
    pfnum = find_pixfmtnum(true);
    if (pfnum == 0) {
      if (allow_software_renderer) {
        pfnum = find_pixfmtnum(false);
        if(pfnum == 0) {
          wgldisplay_cat.error()
            << "Couldn't find HW or Software OpenGL pixfmt appropriate for this desktop!!\n";
        }
      } else {
        wgldisplay_cat.error()
          << "Couldn't find HW-accelerated OpenGL pixfmt appropriate for this desktop!!\n";
      }
      
      if (pfnum == 0) {
        wgldisplay_cat.error()
          << "make sure OpenGL driver is installed, and try reducing the screen size, reducing the\n"
          << "desktop screen pixeldepth to 16bpp,and check your panda window properties\n";
        return 0;
      }
      bUsingSoftware=true;
    }
  }
  
  if (bUsingSoftware) {
    wgldisplay_cat.info()
      << "Couldn't find compatible OGL HW pixelformat, using software rendering.\n";
  }
  
  return pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::find_pixfmtnum
//       Access: Private
//  Description: This helper routine looks for either HW-only or
//               SW-only format, but not both.  Returns the
//               pixelformat number, or 0 if a suitable format could
//               not be found.
////////////////////////////////////////////////////////////////////
int wglGraphicsWindow::
find_pixfmtnum(bool bLookforHW) const {
  int framebuffer_mode = _properties.get_framebuffer_mode();
  bool want_depth_bits = _properties.has_depth_bits();
  bool want_color_bits = _properties.has_color_bits();
  OGLDriverType drvtype;

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;

  // just use the pixfmt of the current desktop

  int MaxPixFmtNum = 
    DescribePixelFormat(_hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  int cur_bpp = GetDeviceCaps(_hdc,BITSPIXEL);
  int pfnum;

  for(pfnum = 1; pfnum <= MaxPixFmtNum; pfnum++) {
    DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    // official, nvidia sanctioned way.
    if ((pfd.dwFlags & PFD_GENERIC_FORMAT) != 0) {
      drvtype = Software;
    } else if (pfd.dwFlags & PFD_GENERIC_ACCELERATED) {
      drvtype = MCD;
    } else {
      drvtype = ICD;
    }

    // skip driver types we are not looking for
    if (bLookforHW) {
      if (drvtype == Software) {
        continue;
      }
    } else {
      if (drvtype != Software) {
        continue;
      }
    }

    if ((pfd.iPixelType == PFD_TYPE_COLORINDEX) && 
        (framebuffer_mode & WindowProperties::FM_index) == 0) {
      continue;
    }

    DWORD dwReqFlags = (PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat->debug()
        << "----------------" << endl;

      if ((framebuffer_mode & WindowProperties::FM_alpha) != 0) {
        wgldisplay_cat->debug()
          << "want alpha, pfd says '"
          << (int)(pfd.cAlphaBits) << "'" << endl;
      }
      if ((framebuffer_mode & WindowProperties::FM_depth) != 0) {
        wgldisplay_cat->debug()
          << "want depth, pfd says '"
          << (int)(pfd.cDepthBits) << "'" << endl;
      }
      if ((framebuffer_mode & WindowProperties::FM_stencil) != 0) {
        wgldisplay_cat->debug()
          << "want stencil, pfd says '"
          << (int)(pfd.cStencilBits) << "'" << endl;
      }
      wgldisplay_cat->debug()
        << "final flag check " << (int)(pfd.dwFlags & dwReqFlags) << " =? "
        << (int)dwReqFlags << endl;
      wgldisplay_cat->debug() 
        << "pfd bits = " << (int)(pfd.cColorBits) << endl;
      wgldisplay_cat->debug() 
        << "cur_bpp = " << cur_bpp << endl;
    }

    if ((framebuffer_mode & WindowProperties::FM_double_buffer) != 0) {
      dwReqFlags|= PFD_DOUBLEBUFFER;
    }
    if ((framebuffer_mode & WindowProperties::FM_alpha) != 0 && 
        (pfd.cAlphaBits==0)) {
      continue;
    }
    if ((framebuffer_mode & WindowProperties::FM_depth) != 0 && 
        (pfd.cDepthBits==0)) {
      continue;
    }
    if ((framebuffer_mode & WindowProperties::FM_stencil) != 0 && 
        (pfd.cStencilBits==0)) {
      continue;
    }

    if ((pfd.dwFlags & dwReqFlags) != dwReqFlags) {
      continue;
    }

    // now we ignore the specified want_color_bits for windowed mode
    // instead we use the current screen depth

    if ((pfd.cColorBits!=cur_bpp) && 
        (!((cur_bpp==16) && (pfd.cColorBits==15))) && 
        (!((cur_bpp==32) && (pfd.cColorBits==24)))) {
      continue;
    }

    // We've passed all the tests, go ahead and pick this fmt.
    // Note: could go continue looping looking for more alpha bits or
    // more depth bits so this would pick 16bpp depth buffer, probably
    // not 24bpp
    break;
  }

  if (pfnum > MaxPixFmtNum) {
    pfnum = 0;
  }

  return pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::setup_colormap
//       Access: Private
//  Description: Sets up a colormap for the window matching the
//               selected pixel format.  This is necessary before
//               creating a GL context.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
setup_colormap() {
  PIXELFORMATDESCRIPTOR pfd;
  LOGPALETTE *logical;
  int n;

  /* grab the pixel format */
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  DescribePixelFormat(_hdc, GetPixelFormat(_hdc),
                      sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  if (!(pfd.dwFlags & PFD_NEED_PALETTE ||
      pfd.iPixelType == PFD_TYPE_COLORINDEX))
    return;

  n = 1 << pfd.cColorBits;

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

  if (pfd.iPixelType == PFD_TYPE_RGBA) {
    int redMask = (1 << pfd.cRedBits) - 1;
    int greenMask = (1 << pfd.cGreenBits) - 1;
    int blueMask = (1 << pfd.cBlueBits) - 1;
    int i;

    /* fill in an RGBA color palette */
    for (i = 0; i < n; ++i) {
      logical->palPalEntry[i].peRed =
        (((i >> pfd.cRedShift)   & redMask)   * 255) / redMask;
      logical->palPalEntry[i].peGreen =
        (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
        logical->palPalEntry[i].peBlue =
        (((i >> pfd.cBlueShift)  & blueMask)  * 255) / blueMask;
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
