/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file showBase.h
 * @author shochet
 * @date 2000-02-02
 */

#ifndef SHOWBASE_H
#define SHOWBASE_H

#include "directbase.h"

#include "eventHandler.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "animControl.h"
#include "pointerTo.h"
#include "dconfig.h"
#include "dSearchPath.h"
#include "configVariableSearchPath.h"
#include "nodePath.h"

ConfigureDecl(config_showbase, EXPCL_DIRECT_SHOWBASE, EXPTP_DIRECT_SHOWBASE);

class CollisionTraverser;
class Camera;
class GraphicsEngine;

BEGIN_PUBLISH

EXPCL_DIRECT_SHOWBASE ConfigVariableSearchPath &get_particle_path();

EXPCL_DIRECT_SHOWBASE void throw_new_frame();

EXPCL_DIRECT_SHOWBASE void init_app_for_gui();

// klunky interface since we cant pass array from python->C++
EXPCL_DIRECT_SHOWBASE void add_fullscreen_testsize(int xsize, int ysize);
EXPCL_DIRECT_SHOWBASE void runtest_fullscreen_sizes(GraphicsWindow *win);
EXPCL_DIRECT_SHOWBASE bool query_fullscreen_testresult(int xsize, int ysize);

// to handle windows stickykeys
EXPCL_DIRECT_SHOWBASE void store_accessibility_shortcut_keys();
EXPCL_DIRECT_SHOWBASE void allow_accessibility_shortcut_keys(bool allowKeys);

#ifdef IS_OSX
EXPCL_DIRECT_SHOWBASE void activate_osx_application();
#endif

END_PUBLISH


#if 0
class TempGridZoneManager {
PUBLISHED:
  TempGridZoneManager() {}
  ~TempGridZoneManager() {}

  unsigned int add_grid_zone(
      unsigned int x,
      unsigned int y,
      unsigned int width,
      unsigned int height,
      unsigned int zoneBase,
      unsigned int xZoneResolution,
      unsigned int yZoneResolution);
  int get_zone_list(int x, int y);

protected:
  class GridZone {
  public:
    unsigned int base;
    unsigned int resolution;
    GridZone(
        unsigned int x,
        unsigned int y,
        unsigned int width,
        unsigned int height,
        unsigned int zoneBase,
        unsigned int xZoneResolution,
        unsigned int yZoneResolution) {
      base=zoneBase;
      resolution=zoneResolution;
    }
  };
  Set<GridZone> _grids;
};
#endif

#endif
