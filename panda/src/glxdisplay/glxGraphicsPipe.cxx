// Filename: glxGraphicsPipe.cxx
// Created by:  mike (09Jan97)
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

#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "frameBufferProperties.h"
#include "mutexHolder.h"

TypeHandle glxGraphicsPipe::_type_handle;

bool glxGraphicsPipe::_error_handlers_installed = false;
glxGraphicsPipe::ErrorHandlerFunc *glxGraphicsPipe::_prev_error_handler;
glxGraphicsPipe::IOErrorHandlerFunc *glxGraphicsPipe::_prev_io_error_handler;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
glxGraphicsPipe::
glxGraphicsPipe(const string &display) {
  string display_spec = display;
  if (display_spec.empty()) {
    display_spec = display_cfg;
  }
  if (display_spec.empty()) {
    display_spec = ExecutionEnvironment::get_environment_variable("DISPLAY");
  }
  if (display_spec.empty()) {
    display_spec = ":0.0";
  }

  setlocale(LC_ALL, "");

  _is_valid = false;
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _display = NULL;
  _screen = 0;
  _root = (Window)NULL;
  _im = (XIM)NULL;
  _hidden_cursor = None;

  install_error_handlers();

  _display = XOpenDisplay(display_spec.c_str());
  if (!_display) {
    glxdisplay_cat.error()
      << "Could not open display \"" << display_spec << "\".\n";
    return;
  }

  if (!XSupportsLocale()) {
    glxdisplay_cat.warning()
      << "X does not support locale " << setlocale(LC_ALL, NULL) << "\n";
  }
  XSetLocaleModifiers("");

  int errorBase, eventBase;
  if (!glXQueryExtension(_display, &errorBase, &eventBase)) {
    glxdisplay_cat.error()
      << "OpenGL GLX extension not supported on display \"" << display_spec
      << "\".\n";
    return;
  }

  _screen = DefaultScreen(_display);
  _root = RootWindow(_display, _screen);
  _display_width = DisplayWidth(_display, _screen);
  _display_height = DisplayHeight(_display, _screen);
  _is_valid = true;

  // Connect to an input method for supporting international text
  // entry.
  _im = XOpenIM(_display, NULL, NULL, NULL);
  if (_im == (XIM)NULL) {
    glxdisplay_cat.warning()
      << "Couldn't open input method.\n";
  }

  // What styles does the current input method support?
  /*
  XIMStyles *im_supported_styles;
  XGetIMValues(_im, XNQueryInputStyle, &im_supported_styles, NULL);

  for (int i = 0; i < im_supported_styles->count_styles; i++) {
    XIMStyle style = im_supported_styles->supported_styles[i];
    cerr << "style " << i << ". " << hex << style << dec << "\n";
  }

  XFree(im_supported_styles);
  */

  // Get the X atom number.
  _wm_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", false);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
glxGraphicsPipe::
~glxGraphicsPipe() {
  release_hidden_cursor();
  if (_im) {
    XCloseIM(_im);
  }
  if (_display) {
    XCloseDisplay(_display);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string glxGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               glxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) glxGraphicsPipe::
pipe_constructor() {
  return new glxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_gsg
//       Access: Protected, Virtual
//  Description: Creates a new GSG to use the pipe (but no windows
//               have been created yet for the GSG).  This method will
//               be called in the draw thread for the GSG.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) glxGraphicsPipe::
make_gsg(const FrameBufferProperties &properties,
         GraphicsStateGuardian *share_with) {
  if (!_is_valid) {
    return NULL;
  }

  glxGraphicsStateGuardian *share_gsg = NULL;
  GLXContext share_context = NULL;

  if (share_with != (GraphicsStateGuardian *)NULL) {
    if (!share_with->is_exact_type(glxGraphicsStateGuardian::get_class_type())) {
      glxdisplay_cat.error()
        << "Cannot share context between glxGraphicsStateGuardian and "
        << share_with->get_type() << "\n";
      return NULL;
    }

    DCAST_INTO_R(share_gsg, share_with, NULL);
    share_context = share_gsg->_context;
  }

  FrameBufferProperties new_properties = properties;
  GLXContext context = NULL;
  XVisualInfo *visual = NULL;

#ifdef HAVE_GLXFBCONFIG
  GLXFBConfig fbconfig = choose_fbconfig(new_properties);
  if (fbconfig != None) {
    context = 
      glXCreateNewContext(_display, fbconfig, GLX_RGBA_TYPE, share_context,
                          GL_TRUE);
    if (context == NULL) {
      fbconfig = None;
    }
  }
#endif  // HAVE_GLXFBCONFIG

  if (context == NULL) {
    // If we couldn't create a context with the fbconfig interface,
    // try falling back to the older XVisual interface.
    visual = choose_visual(new_properties);

    if (visual != (XVisualInfo *)NULL) {
      context = glXCreateContext(_display, visual, None, GL_TRUE);
    }
  }

  if (context == NULL) {
    glxdisplay_cat.error()
      << "Could not create GL context.\n";
    return NULL;
  }

#ifdef HAVE_GLXFBCONFIG
  if (visual == (XVisualInfo *)NULL) {
    // If we used the fbconfig to open the context, we still need to
    // get the associated XVisual.
    nassertr(fbconfig != None, NULL);
    visual = glXGetVisualFromFBConfig(_display, fbconfig);
  }

  // Now we can make a GSG.
  PT(glxGraphicsStateGuardian) gsg = 
    new glxGraphicsStateGuardian(new_properties, share_gsg, context, 
                                 visual, _display, _screen, fbconfig);

#else
  PT(glxGraphicsStateGuardian) gsg = 
    new glxGraphicsStateGuardian(new_properties, share_gsg, context, 
                                 visual, _display, _screen);
#endif  // HAVE_GLXFBCONFIG

  return gsg.p();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_window
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) glxGraphicsPipe::
make_window(GraphicsStateGuardian *gsg, const string &name) {
  if (!_is_valid) {
    return NULL;
  }

  return new glxGraphicsWindow(this, gsg, name);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_buffer
//       Access: Protected, Virtual
//  Description: Creates a new offscreen buffer on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsBuffer) glxGraphicsPipe::
make_buffer(GraphicsStateGuardian *gsg, const string &name,
            int x_size, int y_size, bool want_texture) {
  if (!_is_valid) {
    return NULL;
  }

#ifdef HAVE_GLXFBCONFIG
  return new glxGraphicsBuffer(this, gsg, name, x_size, y_size, want_texture);
#else
  return NULL;
#endif  // HAVE_GLXFBCONFIG
}

#ifdef HAVE_GLXFBCONFIG
////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::choose_fbconfig
//       Access: Private
//  Description: Selects an appropriate GLXFBConfig for the given
//               frame buffer properties.  Returns the selected
//               fbconfig if successful, or None otherwise.
//
//               If successful, this may modify properties to reflect
//               the actual visual chosen.
////////////////////////////////////////////////////////////////////
GLXFBConfig glxGraphicsPipe::
choose_fbconfig(FrameBufferProperties &properties) const {
  int frame_buffer_mode = 0;

  if (properties.has_frame_buffer_mode()) {
    frame_buffer_mode = properties.get_frame_buffer_mode();
  }

  int want_depth_bits = properties.get_depth_bits();
  int want_color_bits = properties.get_color_bits();
  int want_alpha_bits = properties.get_alpha_bits();
  int want_stencil_bits = properties.get_stencil_bits();
  int want_multisample_bits = properties.get_multisample_bits();

  GLXFBConfig fbconfig = 
    try_for_fbconfig(frame_buffer_mode, want_depth_bits, want_color_bits,
                     want_alpha_bits, want_stencil_bits, want_multisample_bits);

  // This is the severity level at which we'll report the details of
  // the fbconfig we actually do find.  Normally, it's debug-level
  // information: we don't care about that much detail.
  NotifySeverity show_fbconfig_severity = NS_debug;

  if (fbconfig == None) {
    glxdisplay_cat.info()
      << "glxGraphicsWindow::choose_fbconfig() - fbconfig with requested "
      << "capabilities not found; trying for lesser fbconfig.\n";

    // If we're unable to get the fbconfig we asked for, however, we
    // probably *do* care to know the details about what we actually
    // got, even if we don't have debug mode set.  So we'll report the
    // fbconfig at a higher level.
    show_fbconfig_severity = NS_info;

    bool special_size_request =
      (want_depth_bits != 1 || want_color_bits != 1);

    // We try to be smart about choosing a close match for the fbconfig.
    // First, we'll eliminate some of the more esoteric options one at
    // a time, then two at a time, and finally we'll try just the bare
    // minimum.

    if (special_size_request) {
      // Actually, first we'll eliminate all of the minimum sizes, to
      // try to open a window with all of the requested options, but
      // maybe not as many bits in some options as we'd like.
      fbconfig = try_for_fbconfig(frame_buffer_mode);
    }

    if (fbconfig == None) {
      // Ok, not good enough.  Now try to eliminate options, but keep
      // as many bits as we asked for.

      // This array keeps the bitmasks of options that we pull out of
      // the requested frame_buffer_mode, in order.

      static const int strip_properties[] = {
        // One esoteric option removed.
        FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stencil,
        FrameBufferProperties::FM_accum,
        FrameBufferProperties::FM_alpha,
        FrameBufferProperties::FM_stereo,

        // Two esoteric options removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo,
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,

        // All esoteric options removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,

        // All esoteric options, plus some we'd really really prefer,
        // removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample | FrameBufferProperties::FM_double_buffer,

        // A zero marks the end of the array.
        0
      };

      pset<int> tried_masks;
      tried_masks.insert(frame_buffer_mode);

      int i;
      for (i = 0; fbconfig == None && strip_properties[i] != 0; i++) {
        int new_frame_buffer_mode = frame_buffer_mode & ~strip_properties[i];
        if (tried_masks.insert(new_frame_buffer_mode).second) {
          fbconfig = try_for_fbconfig(new_frame_buffer_mode, want_depth_bits,
                                      want_color_bits, want_alpha_bits, 
                                      want_stencil_bits, want_multisample_bits);

        }
      }

      if (special_size_request) {
        tried_masks.clear();
        tried_masks.insert(frame_buffer_mode);

        if (fbconfig == None) {
          // Try once more, this time eliminating all of the size
          // requests.
          for (i = 0; fbconfig == None && strip_properties[i] != 0; i++) {
            int new_frame_buffer_mode = frame_buffer_mode & ~strip_properties[i];
            if (tried_masks.insert(new_frame_buffer_mode).second) {
              fbconfig = try_for_fbconfig(new_frame_buffer_mode);
            }
          }
        }
      }

      if (fbconfig == None) {
        // Here's our last-ditch desparation attempt: give us any GLX
        // fbconfig at all!
        fbconfig = try_for_fbconfig(0);
      }

      if (fbconfig == None) {
        // This is only an info message, because we can still fall
        // back to the XVisual interface.
        glxdisplay_cat.info()
          << "Could not get any GLX fbconfig.\n";
        return None;
      }
    }
  }

  glxdisplay_cat.info()
    << "Selected suitable GLX fbconfig.\n";

  // Now update our framebuffer_mode and bit depth appropriately.
  int render_mode, double_buffer, stereo, red_size, green_size, blue_size,
    alpha_size, ared_size, agreen_size, ablue_size, aalpha_size,
    depth_size, stencil_size, samples;
  
  glXGetFBConfigAttrib(_display, fbconfig, GLX_RGBA, &render_mode);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_DOUBLEBUFFER, &double_buffer);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_STEREO, &stereo);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_RED_SIZE, &red_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_GREEN_SIZE, &green_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_BLUE_SIZE, &blue_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_ALPHA_SIZE, &alpha_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_ACCUM_RED_SIZE, &ared_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_ACCUM_GREEN_SIZE, &agreen_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_ACCUM_BLUE_SIZE, &ablue_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_ACCUM_ALPHA_SIZE, &aalpha_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_DEPTH_SIZE, &depth_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_STENCIL_SIZE, &stencil_size);
  glXGetFBConfigAttrib(_display, fbconfig, GLX_SAMPLES, &samples);

  frame_buffer_mode = 0;
  if (double_buffer) {
    frame_buffer_mode |= FrameBufferProperties::FM_double_buffer;
  }
  if (stereo) {
    frame_buffer_mode |= FrameBufferProperties::FM_stereo;
  }
  if (!render_mode) {
    frame_buffer_mode |= FrameBufferProperties::FM_index;
  }
  if (stencil_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_stencil;
  }
  if (depth_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_depth;
  }
  if (alpha_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_alpha;
  }
  if (ared_size + agreen_size + ablue_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_accum;
  }
  if (samples != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_multisample;
  }

  properties.set_frame_buffer_mode(frame_buffer_mode);
  properties.set_color_bits(red_size + green_size + blue_size + alpha_size);
  properties.set_depth_bits(depth_size);

  if (glxdisplay_cat.is_on(show_fbconfig_severity)) {
    glxdisplay_cat.out(show_fbconfig_severity)
      << "GLX Fbconfig Info (# bits of each):" << endl
      << " RGBA: " << red_size << " " << green_size << " " << blue_size
      << " " << alpha_size << endl
      << " Accum RGBA: " << ared_size << " " << agreen_size << " "
      << ablue_size << " " << aalpha_size << endl
      << " Depth: " << depth_size << endl
      << " Stencil: " << stencil_size << endl
      << " Samples: " << samples << endl
      << " DoubleBuffer? " << double_buffer << endl
      << " Stereo? " << stereo << endl;
  }

  return fbconfig;
}
#endif  // HAVE_GLXFBCONFIG

#ifdef HAVE_GLXFBCONFIG
////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::try_for_fbconfig
//       Access: Private
//  Description: Attempt to get the requested fbconfig, if it is
//               available.  It's just a wrapper around
//               glXChooseFBConfig().  It returns the fbconfig
//               information if possible, or None if it is not.
////////////////////////////////////////////////////////////////////
GLXFBConfig glxGraphicsPipe::
try_for_fbconfig(int framebuffer_mode,
                 int want_depth_bits, int want_color_bits,
                 int want_alpha_bits, int want_stencil_bits,
                 int want_multisample_bits) const {
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  glxdisplay_cat.debug()
    << "Trying for fbconfig with: RGB(" << want_color_bits << ")";

  int want_color_component_bits = max(want_color_bits / 3, 1);

  attrib_list[n++] = GLX_RED_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_GREEN_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_BLUE_SIZE;
  attrib_list[n++] = want_color_component_bits;

  if (framebuffer_mode & FrameBufferProperties::FM_alpha) {
    glxdisplay_cat.debug(false) << " ALPHA(" << want_alpha_bits << ")";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_alpha_bits;
  }

  switch (framebuffer_mode & FrameBufferProperties::FM_buffer) {
  case FrameBufferProperties::FM_single_buffer:
    glxdisplay_cat.debug(false) << " SINGLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
    attrib_list[n++] = false;
    break;

  case FrameBufferProperties::FM_double_buffer:
  case FrameBufferProperties::FM_triple_buffer:
    glxdisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
    attrib_list[n++] = true;
    break;
  }

  if (framebuffer_mode & FrameBufferProperties::FM_stereo) {
    glxdisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
    attrib_list[n++] = true;
  } else {
    attrib_list[n++] = GLX_STEREO;
    attrib_list[n++] = false;
  }

  if (framebuffer_mode & FrameBufferProperties::FM_depth) {
    glxdisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }

  if (framebuffer_mode & FrameBufferProperties::FM_stencil) {
    glxdisplay_cat.debug(false) << " STENCIL(" << want_stencil_bits << ")";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = want_stencil_bits;
  }

  if (framebuffer_mode & FrameBufferProperties::FM_accum) {
    glxdisplay_cat.debug(false) << " ACCUM";
    attrib_list[n++] = GLX_ACCUM_RED_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_GREEN_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_BLUE_SIZE;
    attrib_list[n++] = want_color_component_bits;
    if (framebuffer_mode & FrameBufferProperties::FM_alpha) {
      attrib_list[n++] = GLX_ACCUM_ALPHA_SIZE;
      attrib_list[n++] = want_alpha_bits;
    }
  }

  if (framebuffer_mode & FrameBufferProperties::FM_multisample) {
    glxdisplay_cat.debug(false) << " MULTISAMPLE(" << want_multisample_bits << ")";
    attrib_list[n++] = GLX_SAMPLES;
    attrib_list[n++] = want_multisample_bits;
  }

  // Terminate the list
  nassertr(n < max_attrib_list, None);
  attrib_list[n] = (int)None;

  int num_configs = 0;
  GLXFBConfig *configs =
    glXChooseFBConfig(_display, _screen, attrib_list, &num_configs);

  if (glxdisplay_cat.is_debug()) {
    if (configs != NULL) {
      glxdisplay_cat.debug(false) 
        << ", " << num_configs << " matches found!\n";
    } else {
      glxdisplay_cat.debug(false) << ", no match.\n";
    }
  }

  if (configs == NULL || num_configs == 0) {
    return None;
  }

  // Pick the first matching fbconfig; this will be the "best" one
  // according to the GLX specifics.
  GLXFBConfig fbconfig = configs[0];
  XFree(configs);

  return fbconfig;
}
#endif  // HAVE_GLXFBCONFIG

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::choose visual
//       Access: Private
//  Description: Selects an appropriate X visual for the given frame
//               buffer properties.  Returns the visual pointer if
//               successful, or NULL otherwise.
//
//               If successful, this may modify properties to reflect
//               the actual visual chosen.
//
//               This is an older GLX interface, replaced by the new
//               fbconfig interface.  However, some implementations of
//               GLX may not support fbconfig, so we have to have this
//               code as a fallback.
////////////////////////////////////////////////////////////////////
XVisualInfo *glxGraphicsPipe::
choose_visual(FrameBufferProperties &properties) const {
  int frame_buffer_mode = 0;

  if (properties.has_frame_buffer_mode()) {
    frame_buffer_mode = properties.get_frame_buffer_mode();
  }

  int want_depth_bits = properties.get_depth_bits();
  int want_color_bits = properties.get_color_bits();
  int want_alpha_bits = properties.get_alpha_bits();
  int want_stencil_bits = properties.get_stencil_bits();
  int want_multisample_bits = properties.get_multisample_bits();

  XVisualInfo *visual = 
    try_for_visual(frame_buffer_mode, want_depth_bits, want_color_bits,
                   want_alpha_bits, want_stencil_bits, want_multisample_bits);

  // This is the severity level at which we'll report the details of
  // the visual we actually do find.  Normally, it's debug-level
  // information: we don't care about that much detail.
  NotifySeverity show_visual_severity = NS_debug;

  if (visual == NULL) {
    glxdisplay_cat.info()
      << "glxGraphicsWindow::choose_visual() - visual with requested\n"
      << "   capabilities not found; trying for lesser visual.\n";

    // If we're unable to get the visual we asked for, however, we
    // probably *do* care to know the details about what we actually
    // got, even if we don't have debug mode set.  So we'll report the
    // visual at a higher level.
    show_visual_severity = NS_info;

    bool special_size_request =
      (want_depth_bits != 1 || want_color_bits != 1);

    // We try to be smart about choosing a close match for the visual.
    // First, we'll eliminate some of the more esoteric options one at
    // a time, then two at a time, and finally we'll try just the bare
    // minimum.

    if (special_size_request) {
      // Actually, first we'll eliminate all of the minimum sizes, to
      // try to open a window with all of the requested options, but
      // maybe not as many bits in some options as we'd like.
      visual = try_for_visual(frame_buffer_mode);
    }

    if (visual == NULL) {
      // Ok, not good enough.  Now try to eliminate options, but keep
      // as many bits as we asked for.

      // This array keeps the bitmasks of options that we pull out of
      // the requested frame_buffer_mode, in order.

      static const int strip_properties[] = {
        // One esoteric option removed.
        FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stencil,
        FrameBufferProperties::FM_accum,
        FrameBufferProperties::FM_alpha,
        FrameBufferProperties::FM_stereo,

        // Two esoteric options removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo,
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
        FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,

        // All esoteric options removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,

        // All esoteric options, plus some we'd really really prefer,
        // removed.
        FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample | FrameBufferProperties::FM_double_buffer,

        // A zero marks the end of the array.
        0
      };

      pset<int> tried_masks;
      tried_masks.insert(frame_buffer_mode);

      int i;
      for (i = 0; visual == NULL && strip_properties[i] != 0; i++) {
        int new_frame_buffer_mode = frame_buffer_mode & ~strip_properties[i];
        if (tried_masks.insert(new_frame_buffer_mode).second) {
          visual = try_for_visual(new_frame_buffer_mode, want_depth_bits,
                                  want_color_bits, want_alpha_bits, 
                                  want_stencil_bits, want_multisample_bits);

        }
      }

      if (special_size_request) {
        tried_masks.clear();
        tried_masks.insert(frame_buffer_mode);

        if (visual == NULL) {
          // Try once more, this time eliminating all of the size
          // requests.
          for (i = 0; visual == NULL && strip_properties[i] != 0; i++) {
            int new_frame_buffer_mode = frame_buffer_mode & ~strip_properties[i];
            if (tried_masks.insert(new_frame_buffer_mode).second) {
              visual = try_for_visual(new_frame_buffer_mode);
            }
          }
        }
      }

      if (visual == NULL) {
        // Here's our last-ditch desparation attempt: give us any GLX
        // visual at all!
        visual = try_for_visual(0);
      }

      if (visual == NULL) {
        glxdisplay_cat.error()
          << "Could not get any GLX visual.\n";
        return NULL;
      }
    }
  }

  glxdisplay_cat.info()
    << "Got visual 0x" << hex << (int)visual->visualid << dec << ".\n";

  // Now update our framebuffer_mode and bit depth appropriately.
  int render_mode, double_buffer, stereo, red_size, green_size, blue_size,
    alpha_size, ared_size, agreen_size, ablue_size, aalpha_size,
    depth_size, stencil_size, samples;
  
  glXGetConfig(_display, visual, GLX_RGBA, &render_mode);
  glXGetConfig(_display, visual, GLX_DOUBLEBUFFER, &double_buffer);
  glXGetConfig(_display, visual, GLX_STEREO, &stereo);
  glXGetConfig(_display, visual, GLX_RED_SIZE, &red_size);
  glXGetConfig(_display, visual, GLX_GREEN_SIZE, &green_size);
  glXGetConfig(_display, visual, GLX_BLUE_SIZE, &blue_size);
  glXGetConfig(_display, visual, GLX_ALPHA_SIZE, &alpha_size);
  glXGetConfig(_display, visual, GLX_ACCUM_RED_SIZE, &ared_size);
  glXGetConfig(_display, visual, GLX_ACCUM_GREEN_SIZE, &agreen_size);
  glXGetConfig(_display, visual, GLX_ACCUM_BLUE_SIZE, &ablue_size);
  glXGetConfig(_display, visual, GLX_ACCUM_ALPHA_SIZE, &aalpha_size);
  glXGetConfig(_display, visual, GLX_DEPTH_SIZE, &depth_size);
  glXGetConfig(_display, visual, GLX_STENCIL_SIZE, &stencil_size);
  glXGetConfig(_display, visual, GLX_STENCIL_SIZE, &samples);

  frame_buffer_mode = 0;
  if (double_buffer) {
    frame_buffer_mode |= FrameBufferProperties::FM_double_buffer;
  }
  if (stereo) {
    frame_buffer_mode |= FrameBufferProperties::FM_stereo;
  }
  if (!render_mode) {
    frame_buffer_mode |= FrameBufferProperties::FM_index;
  }
  if (stencil_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_stencil;
  }
  if (depth_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_depth;
  }
  if (alpha_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_alpha;
  }
  if (ared_size + agreen_size + ablue_size != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_accum;
  }
  if (samples != 0) {
    frame_buffer_mode |= FrameBufferProperties::FM_multisample;
  }

  properties.set_frame_buffer_mode(frame_buffer_mode);
  properties.set_color_bits(red_size + green_size + blue_size + alpha_size);
  properties.set_depth_bits(depth_size);

  if (glxdisplay_cat.is_on(show_visual_severity)) {
    glxdisplay_cat.out(show_visual_severity)
      << "GLX Visual Info (# bits of each):" << endl
      << " RGBA: " << red_size << " " << green_size << " " << blue_size
      << " " << alpha_size << endl
      << " Accum RGBA: " << ared_size << " " << agreen_size << " "
      << ablue_size << " " << aalpha_size << endl
      << " Depth: " << depth_size << endl
      << " Stencil: " << stencil_size << endl
      << " Samples: " << samples << endl
      << " DoubleBuffer? " << double_buffer << endl
      << " Stereo? " << stereo << endl;
  }

  return visual;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::try_for_visual
//       Access: Private
//  Description: Attempt to get the requested visual, if it is
//               available.  It's just a wrapper around
//               glXChooseVisual().  It returns the visual information
//               if possible, or NULL if it is not.
//
//               This is an older GLX interface, replaced by the new
//               fbconfig interface.  However, some implementations of
//               GLX may not support fbconfig, so we have to have this
//               code as a fallback.
////////////////////////////////////////////////////////////////////
XVisualInfo *glxGraphicsPipe::
try_for_visual(int framebuffer_mode,
               int want_depth_bits, int want_color_bits,
               int want_alpha_bits, int want_stencil_bits,
               int want_multisample_bits) const {
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  glxdisplay_cat.debug()
    << "Trying for visual with: RGB(" << want_color_bits << ")";

  int want_color_component_bits = max(want_color_bits / 3, 1);

  attrib_list[n++] = GLX_RGBA;
  attrib_list[n++] = GLX_RED_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_GREEN_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_BLUE_SIZE;
  attrib_list[n++] = want_color_component_bits;

  if (framebuffer_mode & FrameBufferProperties::FM_alpha) {
    glxdisplay_cat.debug(false) << " ALPHA(" << want_alpha_bits << ")";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_alpha_bits;
  }
  if (framebuffer_mode & FrameBufferProperties::FM_double_buffer) {
    glxdisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
  }
  if (framebuffer_mode & FrameBufferProperties::FM_stereo) {
    glxdisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
  }
  if (framebuffer_mode & FrameBufferProperties::FM_depth) {
    glxdisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }
  if (framebuffer_mode & FrameBufferProperties::FM_stencil) {
    glxdisplay_cat.debug(false) << " STENCIL(" << want_stencil_bits << ")";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = want_stencil_bits;
  }
  if (framebuffer_mode & FrameBufferProperties::FM_accum) {
    glxdisplay_cat.debug(false) << " ACCUM";
    attrib_list[n++] = GLX_ACCUM_RED_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_GREEN_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_BLUE_SIZE;
    attrib_list[n++] = want_color_component_bits;
    if (framebuffer_mode & FrameBufferProperties::FM_alpha) {
      attrib_list[n++] = GLX_ACCUM_ALPHA_SIZE;
      attrib_list[n++] = want_alpha_bits;
    }
  }
  if (framebuffer_mode & FrameBufferProperties::FM_multisample) {
    glxdisplay_cat.debug(false) << " MULTISAMPLE(" << want_multisample_bits << ")";
    attrib_list[n++] = GLX_SAMPLES;
    attrib_list[n++] = want_multisample_bits;
  }

  // Terminate the list
  nassertr(n < max_attrib_list, NULL);
  attrib_list[n] = (int)None;

  XVisualInfo *vinfo = glXChooseVisual(_display, _screen, attrib_list);

  if (glxdisplay_cat.is_debug()) {
    if (vinfo != NULL) {
      glxdisplay_cat.debug(false) << ", match found!\n";
    } else {
      glxdisplay_cat.debug(false) << ", no match.\n";
    }
  }

  return vinfo;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_hidden_cursor
//       Access: Private
//  Description: Called once to make an invisible Cursor for return
//               from get_hidden_cursor().
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
make_hidden_cursor() {
  nassertv(_hidden_cursor == None);

  unsigned int x_size, y_size;
  XQueryBestCursor(_display, _root, 1, 1, &x_size, &y_size);

  Pixmap empty = XCreatePixmap(_display, _root, x_size, y_size, 1);

  XColor black;
  memset(&black, 0, sizeof(black));

  _hidden_cursor = XCreatePixmapCursor(_display, empty, empty, 
                                       &black, &black, x_size, y_size);
  XFreePixmap(_display, empty);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::release_hidden_cursor
//       Access: Private
//  Description: Called once to release the invisible cursor created
//               by make_hidden_cursor().
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
release_hidden_cursor() {
  if (_hidden_cursor != None) {
    XFreeCursor(_display, _hidden_cursor);
    _hidden_cursor = None;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::install_error_handlers
//       Access: Private, Static
//  Description: Installs new Xlib error handler functions if this is
//               the first time this function has been called.  These
//               error handler functions will attempt to reduce Xlib's
//               annoying tendency to shut down the client at the
//               first error.  Unfortunately, it is difficult to play
//               nice with the client if it has already installed its
//               own error handlers.
////////////////////////////////////////////////////////////////////
void glxGraphicsPipe::
install_error_handlers() {
  if (_error_handlers_installed) {
    return;
  }

  _prev_error_handler = (ErrorHandlerFunc *)XSetErrorHandler(error_handler);
  _prev_io_error_handler = (IOErrorHandlerFunc *)XSetIOErrorHandler(io_error_handler);
  _error_handlers_installed = true;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               non-fatal Xlib error.
////////////////////////////////////////////////////////////////////
int glxGraphicsPipe::
error_handler(Display *display, XErrorEvent *error) {
  static const int msg_len = 80;
  char msg[msg_len];
  XGetErrorText(display, error->error_code, msg, msg_len);
  glxdisplay_cat.error()
    << msg << "\n";

  if (glx_error_abort) {
    abort();
  }

  // We return to allow the application to continue running, unlike
  // the default X error handler which exits.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::io_error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               fatal Xlib error.
////////////////////////////////////////////////////////////////////
int glxGraphicsPipe::
io_error_handler(Display *display) {
  glxdisplay_cat.fatal()
    << "X fatal error on display " << (void *)display << "\n";

  // Unfortunately, we can't continue from this function, even if we
  // promise never to use X again.  We're supposed to terminate
  // without returning, and if we do return, the caller will exit
  // anyway.  Sigh.  Very poor design on X's part.
  return 0;
}
