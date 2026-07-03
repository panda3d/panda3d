/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_rmlui.cxx
 * @author rdb
 * @date 2011-11-04
 */

#include "config_rmlui.h"
#include "rmlEvent.h"
#include "rmlInputHandler.h"
#include "rmlRegion.h"

#ifndef CPPPARSER
#include "rmlFileInterface.h"
#include "rmlSystemInterface.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/StyleTypes.h>
#endif

#ifdef COMPILE_IN_DEFAULT_FONT
#ifdef HAVE_FREETYPE
#include "default_font.h"
#endif
#endif

#include "pandaSystem.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDARMLUI)
  #error Buildsystem error: BUILDING_PANDARMLUI not defined
#endif

Configure(config_rmlui);
NotifyCategoryDef(rmlui, "");

ConfigVariableDouble rmlui_text_gamma
("rmlui-text-gamma", 1.0,
 PRC_DESC("Stem-darkening exponent applied to the alpha coverage of generated "
          "font glyph atlases.  1.0 leaves FreeType's anti-aliasing unchanged; "
          "values above 1.0 thicken glyph edges for heavier, more legible text "
          "closer to native macOS / Windows font smoothing.  Set to 1.0 to "
          "disable."));

ConfigVariableInt rmlui_layer_pool_size
("rmlui-layer-pool-size", 8,
 PRC_DESC("Number of offscreen layer buffers pre-allocated per RmlUi context "
          "for the render-to-texture layer stack (box-shadow, backdrop-filter, "
          "masks, nested filters).  The pool grows automatically between frames "
          "if exceeded; raise this to avoid a one-frame dropped effect the first "
          "time a frame needs more than the default."));

ConfigureFn(config_rmlui) {
  init_librmlui();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_librmlui() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  RmlEvent::init_type();
  RmlInputHandler::init_type();
  RmlRegion::init_type();

  if (rmlui_cat->is_debug()) {
    rmlui_cat->debug() << "Initializing RmlUi library.\n";
  }

  static RmlFileInterface fi;
  Rml::SetFileInterface(&fi);

  static RmlSystemInterface si;
  Rml::SetSystemInterface(&si);

  Rml::Initialise();

#ifdef COMPILE_IN_DEFAULT_FONT
#ifdef HAVE_FREETYPE
  Rml::LoadFontFace(
    Rml::Span<const Rml::byte>(
      reinterpret_cast<const Rml::byte *>(default_font_data),
      default_font_size),
    "Panda Default",
    Rml::Style::FontStyle::Normal,
    Rml::Style::FontWeight::Normal,
    true);  // fallback_face = true
#endif
#endif

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("RmlUi");
}
