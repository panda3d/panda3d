// Filename: wglGraphicsPipe.cxx
// Created by:  drose (20Dec02)
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

#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include "config_windisplay.h"
#include "wglGraphicsWindow.h"
#include "wglGraphicsBuffer.h"

typedef enum {Software, MCD, ICD} OGLDriverType;
static const char * const OGLDrvStrings[] = { "Software", "MCD", "ICD" };

TypeHandle wglGraphicsPipe::_type_handle;
  

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
wglGraphicsPipe::
wglGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
wglGraphicsPipe::
~wglGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string wglGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               wglGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) wglGraphicsPipe::
pipe_constructor() {
  return new wglGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::make_gsg
//       Access: Protected, Virtual
//  Description: Creates a new GSG to use the pipe (but no windows
//               have been created yet for the GSG).  This method will
//               be called in the draw thread for the GSG.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) wglGraphicsPipe::
make_gsg(const FrameBufferProperties &properties, 
         GraphicsStateGuardian *share_with) {
  if (!_is_valid) {
    return NULL;
  }

  wglGraphicsStateGuardian *share_gsg = NULL;

  if (share_with != (GraphicsStateGuardian *)NULL) {
    if (!share_with->is_exact_type(wglGraphicsStateGuardian::get_class_type())) {
      wgldisplay_cat.error()
        << "Cannot share context between wglGraphicsStateGuardian and "
        << share_with->get_type() << "\n";
      return NULL;
    }

    DCAST_INTO_R(share_gsg, share_with, NULL);
  }

  // We need a DC to examine the available pixel formats.  We'll use
  // the screen DC.
  HDC hdc = GetDC(NULL);
  int pfnum = choose_pfnum(properties, hdc);

  if (gl_force_pixfmt != 0) {
    wgldisplay_cat.info()
      << "overriding pixfmt choice algorithm (" << pfnum 
      << ") with gl-force-pixfmt(" << gl_force_pixfmt << ")\n";
    pfnum = gl_force_pixfmt;
  }

  FrameBufferProperties new_properties;
  get_properties(new_properties, hdc, pfnum);
  ReleaseDC(NULL, hdc);

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "Picking pixfmt #" << pfnum << " = " 
      << new_properties << "\n";
  }

  PT(wglGraphicsStateGuardian) gsg = 
    new wglGraphicsStateGuardian(new_properties, share_gsg, pfnum);

  // Ideally, we should be able to detect whether the share_gsg will
  // be successful, and return NULL if it won't work.  But we can't do
  // that yet, because we can't try to actually share the gsg until we
  // create the context, for which we need to have a window.  That
  // means the app will just have to trust it will work; if it doesn't
  // work, too bad.

  return gsg.p();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::make_window
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) wglGraphicsPipe::
make_window(GraphicsStateGuardian *gsg, const string &name) {
  return new wglGraphicsWindow(this, gsg, name);
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::make_buffer
//       Access: Protected, Virtual
//  Description: Creates a new offscreen buffer on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsBuffer) wglGraphicsPipe::
make_buffer(GraphicsStateGuardian *gsg, const string &name,
            int x_size, int y_size, bool want_texture) {
  return new wglGraphicsBuffer(this, gsg, name, x_size, y_size, want_texture);
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::choose_pfnum
//       Access: Private, Static
//  Description: Selects a suitable pixel format number for the given
//               frame buffer properties.  Returns the selected number
//               if successful, or 0 otherwise.
//
//               If successful, this may modify properties to reflect
//               the actual visual chosen.
////////////////////////////////////////////////////////////////////
int wglGraphicsPipe::
choose_pfnum(const FrameBufferProperties &properties, HDC hdc) {
  int pfnum;

  int mode = properties.get_frame_buffer_mode();
  bool hardware = ((mode & FrameBufferProperties::FM_hardware) != 0);
  bool software = ((mode & FrameBufferProperties::FM_software) != 0);

  // If the user specified neither hardware nor software frame buffer,
  // he gets either one.
  if (!hardware && !software) {
    hardware = true;
    software = true;
  }

  if (hardware) {
    pfnum = find_pixfmtnum(properties, hdc, true);
    if (pfnum == 0) {
      if (software) {
        pfnum = find_pixfmtnum(properties, hdc, false);
        if (pfnum == 0) {
          wgldisplay_cat.error()
            << "Couldn't find HW or Software OpenGL pixfmt appropriate for this desktop!!\n";
        } else {
          wgldisplay_cat.info()
            << "Couldn't find compatible OGL HW pixelformat, using software rendering.\n";
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
    }

  } else {
    pfnum = find_pixfmtnum(properties, hdc, false);
    
    if (pfnum == 0) {
      wgldisplay_cat.error()
        << "Couldn't find compatible software-renderer OpenGL pixfmt, check your window properties!\n";
      return 0;
    }
  }
  
  return pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::find_pixfmtnum
//       Access: Private, Static
//  Description: This helper routine looks for either HW-only or
//               SW-only format, but not both.  Returns the
//               pixelformat number, or 0 if a suitable format could
//               not be found.
////////////////////////////////////////////////////////////////////
int wglGraphicsPipe::
find_pixfmtnum(const FrameBufferProperties &properties, HDC hdc,
               bool bLookforHW) {
  int frame_buffer_mode = properties.get_frame_buffer_mode();
  int depth_bits = properties.get_depth_bits();
  int color_bits = properties.get_color_bits();
  int alpha_bits = properties.get_alpha_bits();
  int stencil_bits = properties.get_stencil_bits();
  OGLDriverType drvtype;

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;

  // We have to call DescribePixelFormat() once just to get the
  // highest pfnum available.  Then we can iterate through all of the
  // pfnums.
  int max_pfnum = DescribePixelFormat(hdc, 1, 0, NULL);
  int cur_bpp = GetDeviceCaps(hdc, BITSPIXEL);
  int pfnum = 0;

  int found_pfnum = 0;
  int found_colorbits = 0;
  int found_depthbits = 0;

  for (pfnum = 1; pfnum <= max_pfnum; pfnum++) {
    DescribePixelFormat(hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

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
        (frame_buffer_mode & FrameBufferProperties::FM_index) == 0) {
      continue;
    }

    DWORD want_flags = (PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);
    DWORD dont_want_flags = 0;

    switch (frame_buffer_mode & FrameBufferProperties::FM_buffer) {
    case FrameBufferProperties::FM_single_buffer:
      dont_want_flags |= PFD_DOUBLEBUFFER;
      break;
      
    case FrameBufferProperties::FM_double_buffer:
    case FrameBufferProperties::FM_triple_buffer:
      want_flags |= PFD_DOUBLEBUFFER;
      break;
    }

    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat.debug()
        << "---------------- pfnum " << pfnum << "\n";

      wgldisplay_cat.debug() 
        << "color = " << (int)(pfd.cColorBits)
        << " = R" << (int)(pfd.cRedBits) 
        << " G" << (int)(pfd.cGreenBits)
        << " B" << (int)(pfd.cBlueBits)
        << " A" << (int)(pfd.cAlphaBits) << "\n";

      if ((frame_buffer_mode & FrameBufferProperties::FM_alpha) != 0) {
        wgldisplay_cat.debug()
          << "alpha = " << (int)(pfd.cAlphaBits) << "\n";
      }
      if ((frame_buffer_mode & FrameBufferProperties::FM_depth) != 0) {
        wgldisplay_cat.debug()
          << "depth = " << (int)(pfd.cDepthBits) << "\n";
      }
      if ((frame_buffer_mode & FrameBufferProperties::FM_stencil) != 0) {
        wgldisplay_cat.debug()
          << "stencil = " << (int)(pfd.cStencilBits) << "\n";
      }
      wgldisplay_cat.debug()
        << "flags = " << format_pfd_flags(pfd.dwFlags) << " (missing "
        << format_pfd_flags((~pfd.dwFlags) & want_flags) << ", extra "
        << format_pfd_flags(pfd.dwFlags & dont_want_flags) << ")\n";
    }

    if ((frame_buffer_mode & FrameBufferProperties::FM_alpha) != 0 && 
        (pfd.cAlphaBits < alpha_bits)) {
      wgldisplay_cat.debug() 
        << "  rejecting.\n";
      continue;
    }
    if ((frame_buffer_mode & FrameBufferProperties::FM_depth) != 0 && 
        (pfd.cDepthBits < depth_bits)) {
      wgldisplay_cat.debug() 
        << "  rejecting.\n";
      continue;
    }
    if ((frame_buffer_mode & FrameBufferProperties::FM_stencil) != 0 && 
        (pfd.cStencilBits < stencil_bits)) {
      wgldisplay_cat.debug() 
        << "  rejecting.\n";
      continue;
    }

    if ((pfd.dwFlags & want_flags) != want_flags ||
        (pfd.dwFlags & dont_want_flags) != 0) {
      wgldisplay_cat.debug() 
        << "  rejecting.\n";
      continue;
    }

    if (pfd.cColorBits < color_bits) {
      wgldisplay_cat.debug() 
        << "  rejecting.\n";
      continue;
    }

    // We've passed all the tests; this is an acceptable format.  Do
    // we prefer this one over the previously-found acceptable
    // formats?
    bool preferred = false;

    if (found_pfnum == 0) {
      // If this is the first acceptable format we've found, of course
      // we prefer it.
      preferred = true;

    } else if ((frame_buffer_mode & FrameBufferProperties::FM_depth) != 0
               && pfd.cDepthBits > found_depthbits) {
      // We like having lots of depth bits, to a point.
      if (pfd.cColorBits < found_colorbits && found_depthbits >= 16) {
        // We don't like sacrificing color bits if we have at least 16
        // bits of Z.
      } else {
        preferred = true;
      }

    } else if (pfd.cColorBits > found_colorbits) {
      // We also like having lots of color bits.
      preferred = true;
    }

    if (preferred) {
      wgldisplay_cat.debug() 
        << "  format is acceptable, and preferred.\n";
      found_pfnum = pfnum;
      found_colorbits = pfd.cColorBits;
      found_depthbits = pfd.cDepthBits;
    } else {
      wgldisplay_cat.debug() 
        << "  format is acceptable, but not preferred.\n";
    }
  }

  return found_pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::get_properties
//       Access: Private, Static
//  Description: Gets the FrameBufferProperties to match the
//               indicated pixel format descriptor.
////////////////////////////////////////////////////////////////////
void wglGraphicsPipe::
get_properties(FrameBufferProperties &properties, HDC hdc,
               int pfnum) {
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;

  DescribePixelFormat(hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  properties.clear();

  int mode = 0;
  if (pfd.dwFlags & PFD_DOUBLEBUFFER) {
    mode |= FrameBufferProperties::FM_double_buffer;
  }
  if (pfd.dwFlags & PFD_STEREO) {
    mode |= FrameBufferProperties::FM_stereo;
  }
  if (pfd.dwFlags & PFD_GENERIC_FORMAT) {
    mode |= FrameBufferProperties::FM_software;
  } else {
    mode |= FrameBufferProperties::FM_hardware;
  }

  if (pfd.cColorBits != 0) {
    mode |= FrameBufferProperties::FM_rgb;
    properties.set_color_bits(pfd.cColorBits);
  }
  if (pfd.cAlphaBits != 0) {
    mode |= FrameBufferProperties::FM_alpha;
    properties.set_alpha_bits(pfd.cAlphaBits);
  }
  if (pfd.cDepthBits != 0) {
    mode |= FrameBufferProperties::FM_depth;
    properties.set_depth_bits(pfd.cDepthBits);
  }
  if (pfd.cStencilBits != 0) {
    mode |= FrameBufferProperties::FM_stencil;
    properties.set_stencil_bits(pfd.cStencilBits);
  }

  properties.set_frame_buffer_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::format_pfd_flags
//       Access: Private, Static
//  Description: Returns pfd_flags formatted as a string in a
//               user-friendly way.
////////////////////////////////////////////////////////////////////
string wglGraphicsPipe::
format_pfd_flags(DWORD pfd_flags) {
  struct FlagDef {
    DWORD flag;
    const char *name;
  };
  static FlagDef flag_def[] = {
    { PFD_DRAW_TO_WINDOW, "PFD_DRAW_TO_WINDOW" },
    { PFD_DRAW_TO_BITMAP, "PFD_DRAW_TO_BITMAP" },
    { PFD_SUPPORT_GDI, "PFD_SUPPORT_GDI" },
    { PFD_SUPPORT_OPENGL, "PFD_SUPPORT_OPENGL" },
    { PFD_GENERIC_ACCELERATED, "PFD_GENERIC_ACCELERATED" },
    { PFD_GENERIC_FORMAT, "PFD_GENERIC_FORMAT" },
    { PFD_NEED_PALETTE, "PFD_NEED_PALETTE" },
    { PFD_NEED_SYSTEM_PALETTE, "PFD_NEED_SYSTEM_PALETTE" },
    { PFD_DOUBLEBUFFER, "PFD_DOUBLEBUFFER" },
    { PFD_STEREO, "PFD_STEREO" },
    { PFD_SWAP_LAYER_BUFFERS, "PFD_SWAP_LAYER_BUFFERS" },
    { PFD_SWAP_COPY, "PFD_SWAP_COPY" },
    { PFD_SWAP_EXCHANGE, "PFD_SWAP_EXCHANGE" },
  };
  static const int num_flag_defs = sizeof(flag_def) / sizeof(FlagDef);

  ostringstream out;

  const char *sep = "";
  bool got_any = false;
  for (int i = 0; i < num_flag_defs; i++) {
    if (pfd_flags & flag_def[i].flag) {
      out << sep << flag_def[i].name;
      pfd_flags &= ~flag_def[i].flag;
      sep = "|";
      got_any = true;
    }
  }

  if (pfd_flags != 0 || !got_any) {
    out << sep << hex << "0x" << pfd_flags << dec;
  }

  return out.str();
}
