/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glxdisplay.cxx
 * @author cary
 * @date 1999-10-07
 */

#include "config_glxdisplay.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsPipe.h"
#include "glxGraphicsPixmap.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsStateGuardian.h"
#include "posixGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_GLXDISPLAY)
  #error Buildsystem error: BUILDING_PANDA_GLXDISPLAY not defined
#endif

Configure(config_glxdisplay);
NotifyCategoryDef(glxdisplay, "display");

ConfigureFn(config_glxdisplay) {
  init_libglxdisplay();
}

ConfigVariableBool glx_get_proc_address
("glx-get-proc-address", true,
 PRC_DESC("Set this to true to allow the use of glxGetProcAddress(), if "
          "it is available, to query the OpenGL extension functions.  This "
          "is the standard way to query extension functions."));


ConfigVariableBool glx_get_os_address
("glx-get-os-address", true,
 PRC_DESC("Set this to true to allow Panda to query the OpenGL library "
          "directly using standard operating system calls to locate "
          "addresses of extension functions.  This will be done only "
          "if glxGetProcAddress() cannot be used for some reason."));

ConfigVariableBool glx_support_fbconfig
("glx-support-fbconfig", true,
 PRC_DESC("Set this true to enable the use of the advanced FBConfig "
          "interface (as opposed to the older XVisual interface) "
          "if it is available, to select a graphics visual and "
          "create an OpenGL context."));

ConfigVariableBool glx_support_pbuffer
("glx-support-pbuffer", true,
 PRC_DESC("Set this true to enable the use of X pbuffer-based offscreen "
          "buffers, if available.  This is usually preferred over "
          "pixmap-based buffers, but not all drivers support them."));

ConfigVariableBool glx_support_pixmap
("glx-support-pixmap", false,
 PRC_DESC("Set this true to enable the use of X pixmap-based offscreen "
          "buffers.  This is false by default because pixmap-based buffers "
          "are usually slower than pbuffer-based buffers."));


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libglxdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

#ifdef HAVE_GLXFBCONFIG
  glxGraphicsBuffer::init_type();
#endif  // HAVE_GLXFBCONFIG
  glxGraphicsPipe::init_type();
  glxGraphicsPixmap::init_type();
  glxGraphicsBuffer::init_type();
  glxGraphicsWindow::init_type();
  glxGraphicsStateGuardian::init_type();
  PosixGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(glxGraphicsPipe::get_class_type(),
                           glxGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "GLX");
}
