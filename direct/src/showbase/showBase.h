// Filename: showBase.h
// Created by:  shochet (02Feb00)
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
#include "nodePath.h"

ConfigureDecl(config_showbase, EXPCL_DIRECT, EXPTP_DIRECT);
typedef Config::Config<ConfigureGetConfig_config_showbase> ConfigShowbase;

class CollisionTraverser;
class Camera;
class GraphicsEngine;

BEGIN_PUBLISH

EXPCL_DIRECT DSearchPath &get_particle_path();

EXPCL_DIRECT void throw_new_frame();

EXPCL_DIRECT ConfigShowbase &get_config_showbase();


// klunky interface since we cant pass array from python->C++
EXPCL_DIRECT void add_fullscreen_testsize(int xsize, int ysize);
EXPCL_DIRECT void runtest_fullscreen_sizes(GraphicsWindow *win);
EXPCL_DIRECT bool query_fullscreen_testresult(int xsize, int ysize);

END_PUBLISH

#endif
