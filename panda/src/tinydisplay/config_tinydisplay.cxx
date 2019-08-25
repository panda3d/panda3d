/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_tinydisplay.cxx
 * @author drose
 * @date 2008-04-24
 */

#include "config_tinydisplay.h"
#include "tinyXGraphicsPipe.h"
#include "tinyXGraphicsWindow.h"
#include "tinyWinGraphicsPipe.h"
#include "tinyWinGraphicsWindow.h"
#include "tinySDLGraphicsPipe.h"
#include "tinySDLGraphicsWindow.h"
#include "tinyOffscreenGraphicsPipe.h"
#include "tinyGraphicsBuffer.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_TINYDISPLAY)
  #error Buildsystem error: BUILDING_TINYDISPLAY not defined
#endif

Configure(config_tinydisplay);
NotifyCategoryDef(tinydisplay, "display");

ConfigureFn(config_tinydisplay) {
  init_libtinydisplay();
}

ConfigVariableBool td_ignore_mipmaps
  ("td-ignore-mipmaps", false,
   PRC_DESC("Configure this true to disable use of mipmaps on the "
            "tinydisplay software renderer."));

ConfigVariableBool td_ignore_clamp
  ("td-ignore-clamp", false,
   PRC_DESC("Configure this true to disable texture clamp mode and other "
            "wrap modes other than repeat (all textures will repeat, which "
            "is a little cheaper)."));

ConfigVariableBool td_perspective_textures
  ("td-perspective-textures", true,
   PRC_DESC("Configure this false to disable use of perspective-correct "
            "textures on the tinydisplay software renderer, for a small "
            "performance gain."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libtinydisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  TinyGraphicsBuffer::init_type();
  TinyGraphicsStateGuardian::init_type();
  TinyGeomMunger::init_type();
  TinyTextureContext::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("TinyPanda");

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();

#ifdef HAVE_X11
  TinyXGraphicsPipe::init_type();
  TinyXGraphicsWindow::init_type();
  selection->add_pipe_type(TinyXGraphicsPipe::get_class_type(),
                           TinyXGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "native_window_system", "X11");
#endif

#ifdef WIN32
  TinyWinGraphicsPipe::init_type();
  TinyWinGraphicsWindow::init_type();
  selection->add_pipe_type(TinyWinGraphicsPipe::get_class_type(),
                           TinyWinGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "native_window_system", "Win");
#endif

#ifdef HAVE_SDL
  TinySDLGraphicsPipe::init_type();
  TinySDLGraphicsWindow::init_type();
  selection->add_pipe_type(TinySDLGraphicsPipe::get_class_type(),
                           TinySDLGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "SDL", "SDL");
#endif

  TinyOffscreenGraphicsPipe::init_type();
  selection->add_pipe_type(TinyOffscreenGraphicsPipe::get_class_type(),
                           TinyOffscreenGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "", "");
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_p3tinydisplay() {

#ifdef WIN32
  return TinyWinGraphicsPipe::get_class_type().get_index();
#endif

#ifdef HAVE_X11
  return TinyXGraphicsPipe::get_class_type().get_index();
#endif

#ifdef HAVE_SDL
  return TinySDLGraphicsPipe::get_class_type().get_index();
#endif

  return TinyOffscreenGraphicsPipe::get_class_type().get_index();
}
