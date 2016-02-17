/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_collide.h
 * @author drose
 * @date 2000-04-24
 */

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
extern EXPCL_PANDA_COLLIDE ConfigVariableBool pushers_horizontal;

extern EXPCL_PANDA_COLLIDE void init_libcollide();

#endif
