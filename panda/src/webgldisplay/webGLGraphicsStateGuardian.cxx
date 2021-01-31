/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webGLGraphicsStateGuardian.cxx
 * @author rdb
 * @date 2015-04-01
 */

#include "webGLGraphicsStateGuardian.h"
#include "config_webgldisplay.h"

extern "C" {
  extern void* emscripten_GetProcAddress(const char *x);
}

TypeHandle WebGLGraphicsStateGuardian::_type_handle;

/**
 *
 */
WebGLGraphicsStateGuardian::
WebGLGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe) :
  GLES2GraphicsStateGuardian(engine, pipe)
{
  _context = 0;
  _have_context = false;
}

/**
 *
 */
WebGLGraphicsStateGuardian::
~WebGLGraphicsStateGuardian() {
  if (_context != 0) {
    const char *target = "#canvas";
    emscripten_set_webglcontextlost_callback(target, NULL, false, NULL);
    emscripten_set_webglcontextrestored_callback(target, NULL, false, NULL);

    if (emscripten_webgl_destroy_context(_context) != EMSCRIPTEN_RESULT_SUCCESS) {
      webgldisplay_cat.error() << "Failed to destroy WebGL context!\n";
    }
    _context = 0;
  }
}

/**
 * Selects a visual or fbconfig for all the windows and buffers that use this
 * gsg.  Also creates the GL context and obtains the visual.
 */
void WebGLGraphicsStateGuardian::
choose_pixel_format(const FrameBufferProperties &properties,
                    const char *target) {

  nassertv(_context == 0);

  EmscriptenWebGLContextAttributes attribs;
  emscripten_webgl_init_context_attributes(&attribs);

  attribs.alpha = (properties.get_alpha_bits() > 0);
  attribs.depth = (properties.get_depth_bits() > 0);
  attribs.stencil = (properties.get_stencil_bits() > 0);
  attribs.majorVersion = 2;
  attribs.minorVersion = 0;
  attribs.enableExtensionsByDefault = false;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE result;
  result = emscripten_webgl_create_context(target, &attribs);

  if (result <= 0) {
    // Fall back to WebGL 1.
    attribs.majorVersion = 1;
    result = emscripten_webgl_create_context(target, &attribs);
  }

  if (result > 0) {
    _context = result;
    _have_context = true;

    // We may lose the WebGL context at any time, at which time we have to be
    // prepared to drop all resources.
    emscripten_set_webglcontextlost_callback(target, (void *)this, false, &on_context_event);
    emscripten_set_webglcontextrestored_callback(target, (void *)this, false, &on_context_event);
  } else {
    webgldisplay_cat.error() << "Context creation failed.\n";
  }
}

/**
 * Resets all internal state as if the gsg were newly created.
 */
void WebGLGraphicsStateGuardian::
reset() {
  GLES2GraphicsStateGuardian::reset();
}

/**
 * WebGL may take the context away from us at any time.  We have to be ready
 * for that.
 */
EM_BOOL WebGLGraphicsStateGuardian::
on_context_event(int type, const void *, void *user_data) {
  WebGLGraphicsStateGuardian *gsg = (WebGLGraphicsStateGuardian *)user_data;
  nassertr(gsg != NULL, false);

  if (type == EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST) {
    webgldisplay_cat.info() << "WebGL context lost!\n";
    gsg->context_lost();
    return true;

  } else if (type == EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED) {
    webgldisplay_cat.info() << "WebGL context restored!\n";
    gsg->reset();
    return true;
  }

  return false;
}

/**
 * Called to indicate that we lost the WebGL context, and any resources we
 * prepared previously are gone.
 */
void WebGLGraphicsStateGuardian::
context_lost() {
  release_all();

  // Assign a new PreparedGraphicsObjects to prevent the GSG from to trying to
  // neatly clean up the old resources.
  _prepared_objects = new PreparedGraphicsObjects;
}

/**
 * This may be redefined by a derived class (e.g.  glx or wgl) to get whatever
 * further extensions strings may be appropriate to that interface, in
 * addition to the GL extension strings return by glGetString().
 */
void WebGLGraphicsStateGuardian::
get_extra_extensions() {
}

/**
 * Returns true if the indicated extension is reported by the GL system, false
 * otherwise.  The extension name is case-sensitive.
 */
bool WebGLGraphicsStateGuardian::
has_extension(const std::string &extension) const {
  nassertr(_context != 0, false);

  // If the GSG asks for it, that is probably a good reason to activate the
  // extension.
  EM_BOOL result = emscripten_webgl_enable_extension(_context, extension.c_str());
#ifndef NDEBUG
  if (result) {
    webgldisplay_cat.info() << "Activated extension: " << extension << "\n";
  }
#endif
  return result;
}

/**
 * Returns the pointer to the GL extension function with the indicated name.
 * It is the responsibility of the caller to ensure that the required
 * extension is defined in the OpenGL runtime prior to calling this; it is an
 * error to call this for a function that is not defined.
 */
void *WebGLGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  return emscripten_GetProcAddress(name);
}
