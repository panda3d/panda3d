/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_framework.cxx
 * @author drose
 * @date 2000-09-06
 */

#include "config_framework.h"

#include "dconfig.h"
#include "windowFramework.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_FRAMEWORK)
  #error Buildsystem error: BUILDING_FRAMEWORK not defined
#endif

Configure(config_framework);
NotifyCategoryDef(framework, "");

ConfigVariableDouble aspect_ratio
("aspect-ratio", 0.0);
ConfigVariableBool show_frame_rate_meter
("show-frame-rate-meter", false);
ConfigVariableBool show_scene_graph_analyzer_meter
("show-scene-graph-analyzer-meter", false);
ConfigVariableBool print_pipe_types
("print-pipe-types", true);
ConfigVariableString window_type
("window-type", "onscreen");

ConfigVariableString record_session
("record-session", "");
ConfigVariableString playback_session
("playback-session", "");


ConfigureFn(config_framework) {
  WindowFramework::init_type();
}
