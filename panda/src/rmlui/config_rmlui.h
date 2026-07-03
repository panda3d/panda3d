/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_rmlui.h
 * @author rdb
 * @date 2011-11-04
 */

#ifndef CONFIG_RMLUI_H
#define CONFIG_RMLUI_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"

NotifyCategoryDecl(rmlui, EXPCL_PANDARMLUI, EXPTP_PANDARMLUI);

// Stem-darkening exponent applied to the alpha coverage of generated font
// glyph atlases.  1.0 leaves FreeType's anti-aliasing unchanged; values > 1.0
// thicken/darken glyph edges, giving text more weight (closer to the heavier
// font smoothing of native macOS / Windows text).  Defaults to 1.0 (off).
extern ConfigVariableDouble rmlui_text_gamma;

// Number of offscreen layer buffers pre-allocated per context for the
// PushLayer / CompositeLayers render-to-texture stack (box-shadow,
// backdrop-filter, masks, nested filters).  The pool grows automatically
// between frames if a frame needs more, but pre-sizing it avoids a one-frame
// dropped effect on the first frame that exceeds the pool.  Default 8.
extern ConfigVariableInt rmlui_layer_pool_size;

extern EXPCL_PANDARMLUI void init_librmlui();

#endif
