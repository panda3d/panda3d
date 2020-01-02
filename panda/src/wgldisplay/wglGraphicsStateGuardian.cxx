/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsStateGuardian.cxx
 * @author drose
 * @date 2003-01-27
 */

#include "wglGraphicsStateGuardian.h"
#include "config_wgldisplay.h"
#include "wglGraphicsBuffer.h"
#include "wglGraphicsPipe.h"
#include "string_utils.h"
#include <atomic>

TypeHandle wglGraphicsStateGuardian::_type_handle;

const char * const wglGraphicsStateGuardian::_twindow_class_name = "wglGraphicsStateGuardian";
bool wglGraphicsStateGuardian::_twindow_class_registered = false;

/**
 *
 */
wglGraphicsStateGuardian::
wglGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                         wglGraphicsStateGuardian *share_with) :
  GLGraphicsStateGuardian(engine, pipe),
  _share_with(share_with)
{
  _made_context = false;
  _context = (HGLRC)nullptr;

  _twindow = (HWND)0;
  _twindow_dc = (HDC)0;

  _pfnum = -1;
  _pfnum_supports_pbuffer = false;
  _pfnum_properties.clear();

  _supports_pbuffer = false;
  _supports_pixel_format = false;
  _supports_wgl_multisample = false;
  _supports_wgl_render_texture = false;

  _wglCreateContextAttribsARB = nullptr;

  get_gamma_table();
}

/**
 *
 */
wglGraphicsStateGuardian::
~wglGraphicsStateGuardian() {
  release_twindow();
  if (_context != (HGLRC)nullptr) {
    wglDeleteContext(_context);
    _context = (HGLRC)nullptr;
  }
}

/**
 * This is called by wglGraphicsWindow when it finds it cannot use the pfnum
 * determined by the GSG.  Assuming this pfnum corresponds to an "advanced"
 * frame buffer determined by wglChoosePixelFormatARB, this asks the GSG to
 * swap out that pfnum for the earlier, "preliminary" pfnum determined via
 * DescribePixelFormat().
 *
 * This is a one-way operation.  Once called, you can never go back to the
 * advanced pfnum.
 *
 * This method returns true if a change was successfully made, or false if
 * there was no second tier to fall back to.
 */
bool wglGraphicsStateGuardian::
fail_pfnum() {
  if (_pfnum == _pre_pfnum) {
    return false;
  }

  _pfnum = _pre_pfnum;
  _pfnum_supports_pbuffer = false;
  _pfnum_properties = _pre_pfnum_properties;
  return true;
}

/**
 * Gets the FrameBufferProperties to match the indicated pixel format
 * descriptor.
 */
void wglGraphicsStateGuardian::
get_properties(FrameBufferProperties &properties, HDC hdc, int pfnum) {

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;

  DescribePixelFormat(hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  properties.clear();
  properties.set_all_specified();

  if (((pfd.dwFlags & PFD_SUPPORT_OPENGL) == 0)||
      ((pfd.dwFlags & PFD_DRAW_TO_WINDOW) == 0)) {
    // Return without setting either RGB or Indexed Color.  This indicates a
    // window that can't do anything at all.
    return;
  }

  if (pfd.iPixelType == PFD_TYPE_COLORINDEX) {
    properties.set_indexed_color(1);
    properties.set_color_bits(pfd.cColorBits);
    properties.set_alpha_bits(pfd.cAlphaBits);
  } else {
    properties.set_rgb_color(1);
    properties.set_rgba_bits(pfd.cRedBits, pfd.cGreenBits,
                             pfd.cBlueBits, pfd.cAlphaBits);
  }

  if (pfd.dwFlags & PFD_DOUBLEBUFFER) {
    properties.set_back_buffers(1);
  }
  if (pfd.dwFlags & PFD_STEREO) {
    properties.set_stereo(1);
  }
  if (pfd.dwFlags & PFD_GENERIC_FORMAT) {
    properties.set_force_software(1);
  } else {
    properties.set_force_hardware(1);
  }

  if (pfd.cDepthBits != 0) {
    properties.set_depth_bits(pfd.cDepthBits);
  }
  if (pfd.cStencilBits != 0) {
    properties.set_stencil_bits(pfd.cStencilBits);
  }

  // The basic API doesn't do accum or multisample.
}

/**
 * Gets the FrameBufferProperties to match the indicated pixel format
 * descriptor, using the WGL extensions.
 */
bool wglGraphicsStateGuardian::
get_properties_advanced(FrameBufferProperties &properties,
                        HDC window_dc, int pfnum) {

  static const int max_attrib_list = 32;
  int iattrib_list[max_attrib_list];
  int ivalue_list[max_attrib_list];
  int ni = 0;

  int acceleration_i, pixel_type_i, double_buffer_i, stereo_i,
    color_bits_i, red_bits_i, green_bits_i, blue_bits_i, alpha_bits_i,
    accum_bits_i, depth_bits_i, stencil_bits_i, multisamples_i,
    srgb_capable_i;

  iattrib_list[acceleration_i = ni++] = WGL_ACCELERATION_ARB;
  iattrib_list[pixel_type_i = ni++] = WGL_PIXEL_TYPE_ARB;
  iattrib_list[double_buffer_i = ni++] = WGL_DOUBLE_BUFFER_ARB;
  iattrib_list[stereo_i = ni++] = WGL_STEREO_ARB;
  iattrib_list[color_bits_i = ni++] = WGL_COLOR_BITS_ARB;
  iattrib_list[red_bits_i = ni++] = WGL_RED_BITS_ARB;
  iattrib_list[green_bits_i = ni++] = WGL_GREEN_BITS_ARB;
  iattrib_list[blue_bits_i = ni++] = WGL_BLUE_BITS_ARB;
  iattrib_list[alpha_bits_i = ni++] = WGL_ALPHA_BITS_ARB;
  iattrib_list[accum_bits_i = ni++] = WGL_ACCUM_BITS_ARB;
  iattrib_list[depth_bits_i = ni++] = WGL_DEPTH_BITS_ARB;
  iattrib_list[stencil_bits_i = ni++] = WGL_STENCIL_BITS_ARB;
  iattrib_list[srgb_capable_i = ni++] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;

  if (_supports_wgl_multisample) {
    iattrib_list[multisamples_i = ni++] = WGL_SAMPLES_ARB;
  }

  // Terminate the list.
  nassertr(ni <= max_attrib_list, false);

  if (!_wglGetPixelFormatAttribivARB(window_dc, pfnum, 0,
                                     ni, iattrib_list, ivalue_list)) {
    return false;
  }

  properties.clear();
  properties.set_all_specified();

  if (ivalue_list[acceleration_i] == WGL_NO_ACCELERATION_ARB) {
    properties.set_force_software(true);
  } else {
    properties.set_force_hardware(true);
  }

  if (ivalue_list[pixel_type_i] == WGL_TYPE_COLORINDEX_ARB) {
    properties.set_indexed_color(true);
    properties.set_color_bits(ivalue_list[color_bits_i]);
    properties.set_alpha_bits(ivalue_list[alpha_bits_i]);
  } else {
    properties.set_rgb_color(true);
    properties.set_rgba_bits(ivalue_list[red_bits_i],
                             ivalue_list[green_bits_i],
                             ivalue_list[blue_bits_i],
                             ivalue_list[alpha_bits_i]);
  }

  if (ivalue_list[double_buffer_i]) {
    properties.set_back_buffers(1);
  }

  if (ivalue_list[stereo_i]) {
    properties.set_stereo(true);
  }

  if (ivalue_list[srgb_capable_i]) {
    properties.set_srgb_color(true);
  }

  if (ivalue_list[accum_bits_i] != 0) {
    properties.set_accum_bits(ivalue_list[accum_bits_i]);
  }

  if (ivalue_list[depth_bits_i] != 0) {
    properties.set_depth_bits(ivalue_list[depth_bits_i]);
  }

  if (ivalue_list[stencil_bits_i] != 0) {
    properties.set_stencil_bits(ivalue_list[stencil_bits_i]);
  }

  if (_supports_wgl_multisample) {
    if (ivalue_list[multisamples_i] != 0) {
      properties.set_multisamples(ivalue_list[multisamples_i]);
    }
  }

  return true;
}

/**
 * Selects a pixel format for all the windows and buffers that use this gsg.
 */
void wglGraphicsStateGuardian::
choose_pixel_format(const FrameBufferProperties &properties,
                    bool need_pbuffer) {

  // Choose best format available using DescribePixelFormat.  In the process,
  // we need a DC to examine the available pixel formats.  We'll use the
  // screen DC.

  if (gl_force_pixfmt.has_value()) {
    wgldisplay_cat.info()
      << "overriding pixfmt choice with gl-force-pixfmt("
      << gl_force_pixfmt << ")\n";
    _pfnum = gl_force_pixfmt;
    _pfnum_properties = properties;
    _pfnum_supports_pbuffer = true;
    return;
  }

  int  best_pfnum = 0;
  int  best_quality = 0;
  FrameBufferProperties best_prop;

  HDC hdc = GetDC(nullptr);

  int max_pfnum = DescribePixelFormat(hdc, 1, 0, nullptr);

  for (int pfnum = 0; pfnum<max_pfnum; ++pfnum) {
    FrameBufferProperties pfprop;
    get_properties(pfprop, hdc, pfnum);
    int quality = pfprop.get_quality(properties);
    if (quality > best_quality) {
      best_pfnum = pfnum;
      best_quality = quality;
      best_prop = pfprop;
    }
  }

  ReleaseDC(nullptr, hdc);

  _pfnum = best_pfnum;
  _pfnum_supports_pbuffer = false;
  _pfnum_properties = best_prop;
  _pre_pfnum = _pfnum;
  _pre_pfnum_properties = _pfnum_properties;

  if (best_quality == 0) {
    wgldisplay_cat.error()
      << "Could not find a usable pixel format.\n";
    return;
  }

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "Preliminary pixfmt #" << _pfnum << " = "
      << _pfnum_properties << "\n";
  }

  // See whether or not the wgl extensions are available.  This routine is
  // called before "reset".  So the extensions list is empty.  We need to
  // create a twindow, make it current, fetch the extensions temporarily, get
  // the few extensions we need, then clear the extensions list again in
  // preparation for the reset.

  HDC twindow_dc = get_twindow_dc();
  if (twindow_dc == 0) {
    return;
  }

  HGLRC twindow_ctx = wglCreateContext(twindow_dc);
  if (twindow_ctx == 0) {
    return;
  }

  if (!wglGraphicsPipe::wgl_make_current(twindow_dc, twindow_ctx, nullptr)) {
    wgldisplay_cat.error()
      << "Failed to make WGL context current.\n";
    wglDeleteContext(twindow_ctx);
    return;
  }

  _extensions.clear();
  save_extensions((const char *)GLP(GetString)(GL_EXTENSIONS));
  get_extra_extensions();
  _supports_pixel_format = has_extension("WGL_ARB_pixel_format");
  _supports_wgl_multisample = has_extension("WGL_ARB_multisample");

  if (has_extension("WGL_ARB_create_context")) {
    _wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
  } else {
    _wglCreateContextAttribsARB = nullptr;
  }

  _extensions.clear();

  if (!_supports_pixel_format) {
    wglDeleteContext(twindow_ctx);
    return;
  }

  _wglGetPixelFormatAttribivARB =
    (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
  _wglGetPixelFormatAttribfvARB =
    (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
  _wglChoosePixelFormatARB =
    (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

  if (_wglGetPixelFormatAttribivARB == nullptr ||
      _wglGetPixelFormatAttribfvARB == nullptr ||
      _wglChoosePixelFormatARB == nullptr) {
    wgldisplay_cat.error()
      << "Driver claims to support WGL_ARB_pixel_format extension, but does not define all functions.\n";
    wglDeleteContext(twindow_ctx);
    return;
  }

  // Use the wgl extensions to find a better format.

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  float fattrib_list[max_attrib_list];
  int ni = 0;
  int nf = 0;

  iattrib_list[ni++] = WGL_SUPPORT_OPENGL_ARB;
  iattrib_list[ni++] = true;
  iattrib_list[ni++] = WGL_PIXEL_TYPE_ARB;
  iattrib_list[ni++] = WGL_TYPE_RGBA_ARB;

  if (need_pbuffer) {
    iattrib_list[ni++] = WGL_DRAW_TO_PBUFFER_ARB;
    iattrib_list[ni++] = true;
    if (_pfnum_properties.get_alpha_bits()) {
      iattrib_list[ni++] = WGL_BIND_TO_TEXTURE_RGBA_ARB;
      iattrib_list[ni++] = true;
    } else {
      iattrib_list[ni++] = WGL_BIND_TO_TEXTURE_RGB_ARB;
      iattrib_list[ni++] = true;
    }
  }

  nassertv(ni < max_attrib_list && nf < max_attrib_list);
  iattrib_list[ni] = 0;
  fattrib_list[nf] = 0;

  static const int max_pformats = 1024;
  int pformat[max_pformats];
  memset(pformat, 0, sizeof(pformat));
  int nformats = 0;

  if (!_wglChoosePixelFormatARB(twindow_dc, iattrib_list, fattrib_list,
                                max_pformats, pformat, (unsigned int *)&nformats)) {
    nformats = 0;
  }
  nformats = std::min(nformats, max_pformats);

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "Found " << nformats << " advanced formats: [";
    for (int i = 0; i < nformats; i++) {
      wgldisplay_cat.debug(false)
        << " " << pformat[i];
    }
    wgldisplay_cat.debug(false)
      << " ]\n";
  }

  if (nformats > 0) {
    if (need_pbuffer) {
      best_quality = 0;
    }

    for (int i = 0; i < nformats; i++) {
      FrameBufferProperties pfprop;
      if (get_properties_advanced(pfprop, twindow_dc, pformat[i])) {
        int quality = pfprop.get_quality(properties);
        if (quality > best_quality) {
          best_pfnum = pformat[i];
          best_quality = quality;
          best_prop = pfprop;
        }
      }
    }

    if (!properties.get_srgb_color()) {
      best_prop.set_srgb_color(false);
    }

    _pfnum = best_pfnum;
    _pfnum_supports_pbuffer = need_pbuffer;
    _pfnum_properties = best_prop;

    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat.debug()
        << "Selected advanced pixfmt #" << _pfnum << " = "
        << _pfnum_properties << "\n";
    }
  }

  wglDeleteContext(twindow_ctx);
  release_twindow();
}

/**
 * Resets all internal state as if the gsg were newly created.
 */
void wglGraphicsStateGuardian::
reset() {
  GLGraphicsStateGuardian::reset();

  _supports_swap_control = has_extension("WGL_EXT_swap_control");

  if (_supports_swap_control) {
    _wglSwapIntervalEXT =
      (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (_wglSwapIntervalEXT == nullptr) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_EXT_swap_control extension, but does not define all functions.\n";
      _supports_swap_control = false;
    }
  }

  if (_supports_swap_control) {
    // Set the video-sync setting up front, if we have the extension that
    // supports it.
    _wglSwapIntervalEXT(sync_video ? 1 : 0);
  }

  _supports_pbuffer = has_extension("WGL_ARB_pbuffer");

  if (_supports_pbuffer) {
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

    if (_wglCreatePbufferARB == nullptr ||
        _wglGetPbufferDCARB == nullptr ||
        _wglReleasePbufferDCARB == nullptr ||
        _wglDestroyPbufferARB == nullptr ||
        _wglQueryPbufferARB == nullptr) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pbuffer extension, but does not define all functions.\n";
      _supports_pbuffer = false;
    }
  }

  _supports_pixel_format = has_extension("WGL_ARB_pixel_format");

  if (_supports_pixel_format) {
    _wglGetPixelFormatAttribivARB =
      (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    _wglGetPixelFormatAttribfvARB =
      (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    _wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    if (_wglGetPixelFormatAttribivARB == nullptr ||
        _wglGetPixelFormatAttribfvARB == nullptr ||
        _wglChoosePixelFormatARB == nullptr) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pixel_format extension, but does not define all functions.\n";
      _supports_pixel_format = false;
    }
  }

  _supports_wgl_multisample = has_extension("WGL_ARB_multisample");

  _supports_wgl_render_texture = has_extension("WGL_ARB_render_texture");

  if (_supports_wgl_render_texture) {
    _wglBindTexImageARB =
      (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
    _wglReleaseTexImageARB =
      (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");
    _wglSetPbufferAttribARB =
      (PFNWGLSETPBUFFERATTRIBARBPROC)wglGetProcAddress("wglSetPbufferAttribARB");
    if (_wglBindTexImageARB == nullptr ||
        _wglReleaseTexImageARB == nullptr ||
        _wglSetPbufferAttribARB == nullptr) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_render_texture, but does not define all functions.\n";
      _supports_wgl_render_texture = false;
    }
  }
}

/**
 * This may be redefined by a derived class (e.g.  glx or wgl) to get whatever
 * further extensions strings may be appropriate to that interface, in
 * addition to the GL extension strings return by glGetString().
 */
void wglGraphicsStateGuardian::
get_extra_extensions() {
  // This is a little bit tricky, since the query function is itself an
  // extension.

  // Look for the ARB flavor first, which wants one parameter, the HDC of the
  // drawing context.
  PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
    (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
  if (wglGetExtensionsStringARB != nullptr) {
    HDC hdc = wglGetCurrentDC();
    if (hdc != 0) {
      save_extensions((const char *)wglGetExtensionsStringARB(hdc));
      return;
    }
  }

  // If that failed, look for the EXT flavor, which wants no parameters.
  PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT =
    (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
  if (wglGetExtensionsStringEXT != nullptr) {
    save_extensions((const char *)wglGetExtensionsStringEXT());
  }
}

/**
 * Returns the pointer to the GL extension function with the indicated name.
 * It is the responsibility of the caller to ensure that the required
 * extension is defined in the OpenGL runtime prior to calling this; it is an
 * error to call this for a function that is not defined.
 */
void *wglGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  return (void*) wglGetProcAddress(name);
}

/**
 * Creates a suitable context for rendering into the given window.  This
 * should only be called from the draw thread.
 */
void wglGraphicsStateGuardian::
make_context(HDC hdc) {
  // We should only call this once for a particular GSG.
  nassertv(!_made_context);

  _made_context = true;

  // Attempt to create a context.
  wglGraphicsPipe::_current_valid = false;

  if (_wglCreateContextAttribsARB != nullptr) {
    // We have a fancier version of wglCreateContext that allows us to specify
    // what kind of OpenGL context we would like.
    int attrib_list[32];
    int n = 0;
    attrib_list[0] = 0;

    if (gl_version.get_num_words() > 0) {
      attrib_list[n++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
      attrib_list[n++] = gl_version[0];
      if (gl_version.get_num_words() > 1) {
        attrib_list[n++] = WGL_CONTEXT_MINOR_VERSION_ARB;
        attrib_list[n++] = gl_version[1];
      }
    }
    int flags = 0;
    if (gl_debug) {
      flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
    }
    if (gl_forward_compatible) {
      flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
      if (gl_version.get_num_words() == 0 || gl_version[0] < 2) {
        wgldisplay_cat.error()
          << "gl-forward-compatible requires gl-version >= 3 0\n";
      }
    }
    if (flags != 0) {
      attrib_list[n++] = WGL_CONTEXT_FLAGS_ARB;
      attrib_list[n++] = flags;
    }
#ifndef SUPPORT_FIXED_FUNCTION
    attrib_list[n++] = WGL_CONTEXT_PROFILE_MASK_ARB;
    attrib_list[n++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
#endif
    attrib_list[n] = 0;

    _context = _wglCreateContextAttribsARB(hdc, 0, attrib_list);
  } else {
    _context = wglCreateContext(hdc);
  }

  if (_context == nullptr) {
    wgldisplay_cat.error()
      << "Could not create GL context.\n";
    _is_valid = false;
    return;
  }

  // Now share texture context with the indicated GSG.
  if (_share_with != nullptr) {
    HGLRC share_context = _share_with->get_share_context();
    if (share_context == nullptr) {
      // Whoops, the target context hasn't yet made its own context.  In that
      // case, it will share context with us.
      _share_with->redirect_share_pool(this);

    } else {
      if (!wglShareLists(share_context, _context)) {
        wgldisplay_cat.error()
          << "Could not share texture contexts between wglGraphicsStateGuardians.\n";
        // Too bad we couldn't detect this error sooner.  Now there's really
        // no way to tell the application it's hosed.
  _is_valid = false;

      } else {
        _prepared_objects = _share_with->get_prepared_objects();
      }
    }

    _share_with = nullptr;
  }
}

/**
 * Returns a wgl context handle for the purpose of sharing texture context
 * with this GSG.  This will either be the GSG's own context handle, if it
 * exists yet, or the context handle of some other GSG that this GSG is
 * planning to share with.  If this returns NULL, none of the GSG's in this
 * share pool have yet created their context.
 */
HGLRC wglGraphicsStateGuardian::
get_share_context() const {
  if (_made_context) {
    return _context;
  }
  if (_share_with != nullptr) {
    return _share_with->get_share_context();
  }
  return nullptr;
}

/**
 * Directs the GSG (along with all GSG's it is planning to share a texture
 * context with) to share texture context with the indicated GSG.
 *
 * This assumes that this GSG's context has not yet been created, and neither
 * have any of the GSG's it is planning to share texture context with; but the
 * graphics context for the indicated GSG has already been created.
 */
void wglGraphicsStateGuardian::
redirect_share_pool(wglGraphicsStateGuardian *share_with) {
  nassertv(!_made_context);
  if (_share_with != nullptr) {
    _share_with->redirect_share_pool(share_with);
  } else {
    _share_with = share_with;
  }
}

/**
 * Creates an invisible window to associate with the GL context, even if we
 * are not going to use it.  This is necessary because in the Windows OpenGL
 * API, we have to create window before we can create a GL context--even
 * before we can ask about what GL extensions are available!
 */
bool wglGraphicsStateGuardian::
make_twindow() {
  release_twindow();

  DWORD window_style = 0;

  register_twindow_class();
  HINSTANCE hinstance = GetModuleHandle(nullptr);
  _twindow = CreateWindow(_twindow_class_name, "twindow", window_style,
                          0, 0, 1, 1, nullptr, nullptr, hinstance, 0);

  if (!_twindow) {
    wgldisplay_cat.error()
      << "CreateWindow() failed!" << std::endl;
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

/**
 * Closes and frees the resources associated with the temporary window created
 * by a previous call to make_twindow().
 */
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

/**
 * Registers a Window class for the twindow created by all wglGraphicsPipes.
 * This only needs to be done once per session.
 */
void wglGraphicsStateGuardian::
register_twindow_class() {
  if (_twindow_class_registered) {
    return;
  }

  WNDCLASS wc;

  HINSTANCE instance = GetModuleHandle(nullptr);

  // Clear before filling in window structure!
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = instance;
  wc.lpszClassName = _twindow_class_name;

  if (!RegisterClass(&wc)) {
    wgldisplay_cat.error()
      << "could not register window class!" << std::endl;
    return;
  }
  _twindow_class_registered = true;
}

#define GAMMA_1 (255.0 * 256.0)

static bool _gamma_table_initialized = false;
static unsigned short _original_gamma_table [256 * 3];

void _create_gamma_table_wgl (PN_stdfloat gamma, unsigned short *original_red_table, unsigned short *original_green_table, unsigned short *original_blue_table, unsigned short *red_table, unsigned short *green_table, unsigned short *blue_table) {
  int i;
  double gamma_correction;

  if (gamma <= 0.0) {
    // avoid divide by zero and negative exponents
    gamma = 1.0;
  }
  gamma_correction = 1.0 / (double) gamma;

  for (i = 0; i < 256; i++) {
    double r;
    double g;
    double b;

    if (original_red_table) {
      r = (double) original_red_table [i] / GAMMA_1;
      g = (double) original_green_table [i] / GAMMA_1;
      b = (double) original_blue_table [i] / GAMMA_1;
    }
    else {
      r = ((double) i / 255.0);
      g = r;
      b = r;
    }

    r = pow (r, gamma_correction);
    g = pow (g, gamma_correction);
    b = pow (b, gamma_correction);

    if (r > 1.00) {
      r = 1.0;
    }
    if (g > 1.00) {
      g = 1.0;
    }
    if (b > 1.00) {
      b = 1.0;
    }

    r = r * GAMMA_1;
    g = g * GAMMA_1;
    b = b * GAMMA_1;

    red_table [i] = r;
    green_table [i] = g;
    blue_table [i] = b;
  }
}

/**
 * Static function for getting the original gamma.
 */
bool wglGraphicsStateGuardian::
get_gamma_table(void) {
  bool get;

  get = false;
  if (_gamma_table_initialized == false) {
    HDC hdc = GetDC(nullptr);

    if (hdc) {
      if (GetDeviceGammaRamp (hdc, (LPVOID) _original_gamma_table)) {
        _gamma_table_initialized = true;
        get = true;
      }

      ReleaseDC (nullptr, hdc);
    }
  }

  return get;
}

/**
 * Static function for setting gamma which is needed for atexit.
 */
bool wglGraphicsStateGuardian::
static_set_gamma(bool restore, PN_stdfloat gamma) {
  bool set;
  HDC hdc = GetDC(nullptr);

  set = false;
  if (hdc) {
    unsigned short ramp [256 * 3];

    if (restore && _gamma_table_initialized) {
      _create_gamma_table_wgl (gamma, &_original_gamma_table [0], &_original_gamma_table [256], &_original_gamma_table [512], &ramp [0], &ramp [256], &ramp [512]);
    }
    else {
      _create_gamma_table_wgl (gamma, 0, 0, 0, &ramp [0], &ramp [256], &ramp [512]);
    }

    if (SetDeviceGammaRamp (hdc, ramp)) {
      set = true;

      // Register an atexit handler
      static std::atomic_flag gamma_modified = ATOMIC_FLAG_INIT;
      if (!gamma_modified.test_and_set()) {
        atexit(atexit_function);
      }
    }

    ReleaseDC (nullptr, hdc);
  }

  return set;
}

/**
 * Non static version of setting gamma.  Returns true on success.
 */
bool wglGraphicsStateGuardian::
set_gamma(PN_stdfloat gamma) {
  bool set;

  set = static_set_gamma(false, gamma);
  if (set) {
    _gamma = gamma;
  }

  return set;
}

/**
 * Restore original gamma.
 */
void wglGraphicsStateGuardian::
restore_gamma() {
  static_set_gamma(true, 1.0f);
}

/**
 * This function is passed to the atexit function.
 */
void wglGraphicsStateGuardian::
atexit_function(void) {
  static_set_gamma(true, 1.0);
}
