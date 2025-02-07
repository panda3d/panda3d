/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsPipe.cxx
 * @author rdb
 * @date 2009-05-21
 */

#include "eglGraphicsBuffer.h"
#include "eglGraphicsPipe.h"
#include "eglGraphicsPixmap.h"
#include "eglGraphicsWindow.h"
#include "eglGraphicsStateGuardian.h"
#include "config_egldisplay.h"
#include "frameBufferProperties.h"

#include <EGL/eglext.h>

static ConfigVariableInt egl_device_index
("egl-device-index", -1,
 PRC_DESC("Selects which EGL device index is used to create the EGL display in "
          "a headless configuration.  The special value -1 selects the default "
          "device."));

TypeHandle eglGraphicsPipe::_type_handle;

/**
 *
 */
eglGraphicsPipe::
eglGraphicsPipe() {
  // Check for client extensions.
  vector_string extensions;
  bool supports_platform_device = false;
  bool supports_device_enumeration = false;
  const char *ext_ptr = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  if (ext_ptr != nullptr) {
    extract_words(ext_ptr, extensions);

    if (egldisplay_cat.is_debug()) {
      std::ostream &out = egldisplay_cat.debug()
        << "Supported EGL client extensions:\n";

      for (const std::string &extension : extensions) {
        out << "  " << extension << "\n";
      }
    }

    if (std::find(extensions.begin(), extensions.end(), "EGL_EXT_platform_device") != extensions.end()) {
      supports_platform_device = true;
    }
    if (std::find(extensions.begin(), extensions.end(), "EGL_EXT_device_enumeration") != extensions.end()) {
      supports_device_enumeration = true;
    }
  }
  else if (egldisplay_cat.is_debug()) {
    eglGetError();
    egldisplay_cat.debug()
      << "EGL client extensions not supported.\n";
  }

  EGLint major, minor;

  int index = egl_device_index.get_value();
  if (index >= 0 && supports_platform_device && supports_device_enumeration) {
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
      (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
      (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");

    EGLint num_devices = 0;
    if (eglQueryDevicesEXT != nullptr &&
        eglQueryDevicesEXT(0, nullptr, &num_devices) &&
        num_devices > 0) {
      EGLDeviceEXT *devices = (EGLDeviceEXT *)alloca(sizeof(EGLDeviceEXT) * num_devices);
      eglQueryDevicesEXT(num_devices, devices, &num_devices);

      if (index >= num_devices) {
        egldisplay_cat.error()
          << "Requested EGL device index " << index << " does not exist ("
          << "there are only " << num_devices << " devices)\n";
        _is_valid = false;
        return;
      }

      if (egldisplay_cat.is_debug()) {
        egldisplay_cat.debug()
          << "Found " << num_devices << " EGL devices, using device index "
          << index << ".\n";
      }

      _egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[index], nullptr);

      if (_egl_display && !eglInitialize(_egl_display, &major, &minor)) {
        egldisplay_cat.error()
          << "Couldn't initialize EGL platform display " << index << ": "
          << get_egl_error_string(eglGetError()) << "\n";
        _egl_display = EGL_NO_DISPLAY;
      }
    }
  }
  else {
    //NB. if the X11 display failed to open, _display will be 0, which is a valid
    // input to eglGetDisplay - it means to open the default display.
  #ifdef USE_X11
    _egl_display = eglGetDisplay((NativeDisplayType) _display);
  #else
    _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  #endif
    if (_egl_display && !eglInitialize(_egl_display, &major, &minor)) {
      egldisplay_cat.warning()
        << "Couldn't initialize the default EGL display: "
        << get_egl_error_string(eglGetError()) << "\n";
      _egl_display = EGL_NO_DISPLAY;
    }

    if (!_egl_display && supports_platform_device && supports_device_enumeration) {
      PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");

      EGLint num_devices = 0;
      if (eglQueryDevicesEXT != nullptr &&
          eglQueryDevicesEXT(0, nullptr, &num_devices) &&
          num_devices > 0) {
        EGLDeviceEXT *devices = (EGLDeviceEXT *)alloca(sizeof(EGLDeviceEXT) * num_devices);
        eglQueryDevicesEXT(num_devices, devices, &num_devices);

        if (egldisplay_cat.is_debug()) {
          egldisplay_cat.debug()
            << "Found " << num_devices << " EGL devices.\n";
        }

        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
          (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

        if (eglGetPlatformDisplayEXT != nullptr) {
          for (EGLint i = 0; i < num_devices && !_egl_display; ++i) {
            _egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[i], nullptr);

            if (_egl_display && !eglInitialize(_egl_display, &major, &minor)) {
              egldisplay_cat.warning()
                << "Couldn't initialize EGL platform display " << i << ": "
                << get_egl_error_string(eglGetError()) << "\n";
              _egl_display = EGL_NO_DISPLAY;
            }
          }
        }
      }
    }
  }

  if (!_egl_display) {
    egldisplay_cat.error()
      << "Failed to find or initialize a suitable EGL display connection.\n";
    _is_valid = false;
    return;
  }

  if (egldisplay_cat.is_debug()) {
    egldisplay_cat.debug()
      << "Successfully initialized EGL display, got version " << major << "." << minor << "\n";
  }

#if defined(OPENGLES_1) || defined(OPENGLES_2)
  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    egldisplay_cat.error()
      << "Couldn't bind EGL to the OpenGL ES API: "
      << get_egl_error_string(eglGetError()) << "\n";
#else
  if (!eglBindAPI(EGL_OPENGL_API)) {
    egldisplay_cat.error()
      << "Couldn't bind EGL to the OpenGL API: "
      << get_egl_error_string(eglGetError()) << "\n";
#endif
    _is_valid = false;
    return;
  }

  // Even if we don't have an X11 display, we can still render headless.
  _is_valid = true;
}

/**
 *
 */
eglGraphicsPipe::
~eglGraphicsPipe() {
  if (_egl_display) {
    if (!eglTerminate(_egl_display)) {
      egldisplay_cat.error() << "Failed to terminate EGL display: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
  }
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string eglGraphicsPipe::
get_interface_name() const {
#if defined(OPENGLES_1) || defined(OPENGLES_2)
  return "OpenGL ES";
#else
  return "OpenGL";
#endif
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default eglGraphicsPipe.
 */
PT(GraphicsPipe) eglGraphicsPipe::
pipe_constructor() {
  return new eglGraphicsPipe;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) eglGraphicsPipe::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return nullptr;
  }

  eglGraphicsStateGuardian *eglgsg = nullptr;
  if (gsg != nullptr) {
    DCAST_INTO_R(eglgsg, gsg, nullptr);
  }

  bool support_rtt;
  support_rtt = false;
  /*
    Currently, no support for eglGraphicsBuffer render-to-texture.
  if (eglgsg) {
     support_rtt =
      eglgsg -> get_supports_render_texture() &&
      support_render_texture;
  }
  */

  // First thing to try: an eglGraphicsWindow

  if (retry == 0) {
#ifdef USE_X11
    if (!_display) {
      return nullptr;
    }
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }
    return new eglGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
#else
    return nullptr;
#endif
  }

  // Second thing to try: a GL(ES(2))GraphicsBuffer
  if (retry == 1) {
    if (host == nullptr ||
  // (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return nullptr;
    }
    if (host->get_engine() != engine) {
      return nullptr;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() > 0 ||
          fb_prop.get_back_buffers() > 0 ||
          fb_prop.get_accum_bits() > 0) {
        return nullptr;
      }
    }
    // Early success - if we are sure that this buffer WILL meet specs, we can
    // precertify it.
    if (eglgsg != nullptr &&
        eglgsg->is_valid() &&
        !eglgsg->needs_reset() &&
        eglgsg->_supports_framebuffer_object &&
        eglgsg->_glDrawBuffers != nullptr &&
        fb_prop.is_basic()) {
      precertify = true;
    }
#ifdef OPENGLES_2
    return new GLES2GraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#elif defined(OPENGLES_1)
    return new GLESGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#else
    return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                flags, gsg, host);
#endif
  }

  // Third thing to try: a eglGraphicsBuffer
  if (retry == 2) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return nullptr;
    }

    if (!support_rtt) {
      if (((flags&BF_rtt_cumulative)!=0)||
          ((flags&BF_can_bind_every)!=0)) {
        // If we require Render-to-Texture, but can't be sure we support it,
        // bail.
        return nullptr;
      }
    }

    return new eglGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Fourth thing to try: an eglGraphicsPixmap.
  if (retry == 3) {
#ifdef USE_X11
    if (!_display) {
      return nullptr;
    }
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return nullptr;
    }

    if (((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }

    return new eglGraphicsPixmap(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
#else
    return nullptr;
#endif
  }

  // Nothing else left to try.
  return nullptr;
}
