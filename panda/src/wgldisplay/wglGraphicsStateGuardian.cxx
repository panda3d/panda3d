// Filename: wglGraphicsStateGuardian.cxx
// Created by:  drose (27Jan03)
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

#include "wglGraphicsStateGuardian.h"
#include "string_utils.h"

TypeHandle wglGraphicsStateGuardian::_type_handle;

const char * const wglGraphicsStateGuardian::_twindow_class_name = "wglGraphicsStateGuardian";
bool wglGraphicsStateGuardian::_twindow_class_registered = false;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
wglGraphicsStateGuardian(const FrameBufferProperties &properties,
                         wglGraphicsStateGuardian *share_with,
                         int pfnum) : 
  GLGraphicsStateGuardian(properties),
  _share_with(share_with),
  _pfnum(pfnum)
{
  _made_context = false;
  _context = (HGLRC)NULL;

  _twindow = (HWND)0;
  _twindow_dc = (HDC)0;

  _supports_pbuffer = false;
  _supports_pixel_format = false;
  _supports_wgl_multisample = false;
  _supports_render_texture = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
~wglGraphicsStateGuardian() {
  release_twindow();
  if (_context != (HGLRC)NULL) {
    wglDeleteContext(_context);
    _context = (HGLRC)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
reset() {
  GLGraphicsStateGuardian::reset();

  _supports_pbuffer = has_extension("WGL_ARB_pbuffer");
  _supports_pixel_format = has_extension("WGL_ARB_pixel_format");
  _supports_wgl_multisample = has_extension("WGL_ARB_multisample");
  _supports_render_texture = has_extension("WGL_ARB_render_texture");

  _wglCreatePbufferARB = 
    (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
  _wglGetPbufferDCARB = 
    (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
  _wglReleasePbufferDCARB = 
    (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
  _wglDestroyPbufferARB = 
    (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
  _wglQueryPbufferARB = 
    (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");

  if (_supports_pbuffer) {
    if (_wglCreatePbufferARB == NULL ||
        _wglGetPbufferDCARB == NULL ||
        _wglReleasePbufferDCARB == NULL ||
        _wglDestroyPbufferARB == NULL ||
        _wglQueryPbufferARB == NULL) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pbuffer extension, but does not define all functions.\n";
      _supports_pbuffer = false;
    }
  }

  _wglGetPixelFormatAttribivARB =
    (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
  _wglGetPixelFormatAttribfvARB =
    (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
  _wglChoosePixelFormatARB =
    (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

  if (_supports_pixel_format) {
    if (_wglGetPixelFormatAttribivARB == NULL ||
        _wglGetPixelFormatAttribfvARB == NULL ||
        _wglChoosePixelFormatARB == NULL) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pixel_format extension, but does not define all functions.\n";
      _supports_pixel_format = false;
    }
  }

  _wglBindTexImageARB = 
    (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
  _wglReleaseTexImageARB = 
    (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");
  _wglSetPbufferAttribARB = 
    (PFNWGLSETPBUFFERATTRIBARBPROC)wglGetProcAddress("wglSetPbufferAttribARB");

  if (_supports_render_texture) {
    if (_wglBindTexImageARB == NULL ||
        _wglReleaseTexImageARB == NULL ||
        _wglSetPbufferAttribARB == NULL) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_render_texture, but does not define all functions.\n";
      _supports_render_texture = false;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_extra_extensions
//       Access: Protected, Virtual
//  Description: This may be redefined by a derived class (e.g. glx or
//               wgl) to get whatever further extensions strings may
//               be appropriate to that interface, in addition to the
//               GL extension strings return by glGetString().
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
get_extra_extensions() {
  // This is a little bit tricky, since the query function is itself
  // an extension.

  // Look for the ARB flavor first, which wants one parameter, the HDC
  // of the drawing context.
  PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 
    (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
  if (wglGetExtensionsStringARB != NULL) {
    HDC hdc = wglGetCurrentDC();
    if (hdc != 0) {
      save_extensions((const char *)wglGetExtensionsStringARB(hdc));
      return;
    }
  }

  // If that failed, look for the EXT flavor, which wants no
  // parameters.
  PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = 
    (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
  if (wglGetExtensionsStringEXT != NULL) {
    save_extensions((const char *)wglGetExtensionsStringEXT());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *wglGraphicsStateGuardian::
get_extension_func(const char *prefix, const char *name) {
  string fullname = string(prefix) + string(name);
  return wglGetProcAddress(fullname.c_str());
}


////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::make_context
//       Access: Private
//  Description: Creates a suitable context for rendering into the
//               given window.  This should only be called from the
//               draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
make_context(HDC hdc) {
  // We should only call this once for a particular GSG.
  nassertv(!_made_context);

  _made_context = true;

  // Attempt to create a context.
  _context = wglCreateContext(hdc);

  if (_context == NULL) {
    wgldisplay_cat.error()
      << "Could not create GL context.\n";
    return;
  }

  // Now share texture context with the indicated GSG.
  if (_share_with != (wglGraphicsStateGuardian *)NULL) {
    HGLRC share_context = _share_with->get_share_context();
    if (share_context == NULL) {
      // Whoops, the target context hasn't yet made its own context.
      // In that case, it will share context with us.
      _share_with->redirect_share_pool(this);

    } else {
      if (!wglShareLists(share_context, _context)) {
        wgldisplay_cat.error()
          << "Could not share texture contexts between wglGraphicsStateGuardians.\n";
        // Too bad we couldn't detect this error sooner.  Now there's
        // really no way to tell the application it's hosed.

      } else {
        _prepared_objects = _share_with->get_prepared_objects();
      }
    }

    _share_with = (wglGraphicsStateGuardian *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_share_context
//       Access: Private
//  Description: Returns a wgl context handle for the purpose of
//               sharing texture context with this GSG.  This will
//               either be the GSG's own context handle, if it exists
//               yet, or the context handle of some other GSG that
//               this GSG is planning to share with.  If this returns
//               NULL, none of the GSG's in this share pool have yet
//               created their context.
////////////////////////////////////////////////////////////////////
HGLRC wglGraphicsStateGuardian::
get_share_context() const {
  if (_made_context) {
    return _context;
  }
  if (_share_with != (wglGraphicsStateGuardian *)NULL) {
    return _share_with->get_share_context();
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::redirect_share_pool
//       Access: Private
//  Description: Directs the GSG (along with all GSG's it is planning
//               to share a texture context with) to share texture
//               context with the indicated GSG.
//
//               This assumes that this GSG's context has not yet been
//               created, and neither have any of the GSG's it is
//               planning to share texture context with; but the
//               graphics context for the indicated GSG has already
//               been created.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
redirect_share_pool(wglGraphicsStateGuardian *share_with) {
  nassertv(!_made_context);
  if (_share_with != (wglGraphicsStateGuardian *)NULL) {
    _share_with->redirect_share_pool(share_with);
  } else {
    _share_with = share_with;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::make_twindow
//       Access: Private
//  Description: Creates an invisible window to associate with the GL
//               context, even if we are not going to use it.  This is
//               necessary because in the Windows OpenGL API, we have
//               to create window before we can create a GL
//               context--even before we can ask about what GL
//               extensions are available!
////////////////////////////////////////////////////////////////////
bool wglGraphicsStateGuardian::
make_twindow() {
  release_twindow();

  DWORD window_style = 0;

  register_twindow_class();
  HINSTANCE hinstance = GetModuleHandle(NULL);
  _twindow = CreateWindow(_twindow_class_name, "twindow", window_style, 
                          0, 0, 1, 1, NULL, NULL, hinstance, 0);
  
  if (!_twindow) {
    wgldisplay_cat.error()
      << "CreateWindow() failed!" << endl;
    return false;
  }

  ShowWindow(_twindow, SW_HIDE);

  _twindow_dc = GetDC(_twindow);

  PIXELFORMATDESCRIPTOR pixelformat;
  if (!SetPixelFormat(_twindow_dc, _pfnum, &pixelformat)) {
    wgldisplay_cat.error()
      << "SetPixelFormat(" << _pfnum << ") failed after window create\n";
    release_twindow();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::release_twindow
//       Access: Private
//  Description: Closes and frees the resources associated with the
//               temporary window created by a previous call to
//               make_twindow().
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
release_twindow() {
  if (_twindow_dc) {
    ReleaseDC(_twindow, _twindow_dc);
    _twindow_dc = 0;
  }
  if (_twindow) {
    DestroyWindow(_twindow);
    _twindow = 0;
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::register_twindow_class
//       Access: Private, Static
//  Description: Registers a Window class for the twindow created by
//               all wglGraphicsPipes.  This only needs to be done
//               once per session.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
register_twindow_class() {
  if (_twindow_class_registered) {
    return;
  }

  WNDCLASS wc;

  HINSTANCE instance = GetModuleHandle(NULL);

  // Clear before filling in window structure!
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = instance;
  wc.lpszClassName = _twindow_class_name;
  
  if (!RegisterClass(&wc)) {
    wgldisplay_cat.error()
      << "could not register window class!" << endl;
    return;
  }
  _twindow_class_registered = true;
}
