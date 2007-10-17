// Filename: config_collide.h
// Created by:  drose (24Apr00)
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

#ifndef CONFIG_COLLIDE_H
#define CONFIG_COLLIDE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(collide, EXPCL_PANDA_COLLIDE, EXPTP_PANDA_COLLIDE);

extern EXPCL_PANDA_COLLIDE ConfigVariableBool respect_prev_transform;
extern EXPCL_PANDA_COLLIDE ConfigVariableBool respect_effective_normal;
extern EXPCL_PANDA_COLLIDE ConfigVariableBool allow_collider_multiple;
extern EXPCL_PANDA_COLLIDE ConfigVariableBool flatten_collision_nodes;
extern EXPCL_PANDA_COLLIDE ConfigVariableDouble collision_parabola_bounds_threshold;
extern EXPCL_PANDA_COLLIDE ConfigVariableInt collision_parabola_bounds_sample;
extern EXPCL_PANDA_COLLIDE ConfigVariableInt fluid_cap_amount;

extern EXPCL_PANDA_COLLIDE void init_libcollide();

#endif
