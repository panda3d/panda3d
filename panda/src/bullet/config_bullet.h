/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_bullet.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __CONFIG_BULLET_H__
#define __CONFIG_BULLET_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"

#include "configVariableSearchPath.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"

#include "bulletWorld.h"

NotifyCategoryDecl(bullet, EXPCL_PANDABULLET, EXPTP_PANDABULLET);

extern ConfigVariableInt bullet_max_objects;
extern ConfigVariableInt bullet_gc_lifetime;
extern ConfigVariableEnum<BulletWorld::BroadphaseAlgorithm> bullet_broadphase_algorithm;
extern ConfigVariableEnum<BulletWorld::FilterAlgorithm> bullet_filter_algorithm;
extern ConfigVariableDouble bullet_sap_extents;
extern ConfigVariableBool bullet_enable_contact_events;
extern ConfigVariableInt bullet_solver_iterations;
extern ConfigVariableBool bullet_additional_damping;
extern ConfigVariableDouble bullet_additional_damping_linear_factor;
extern ConfigVariableDouble bullet_additional_damping_angular_factor;
extern ConfigVariableDouble bullet_additional_damping_linear_threshold;
extern ConfigVariableDouble bullet_additional_damping_angular_threshold;

extern EXPCL_PANDABULLET void init_libbullet();

#endif // __CONFIG_BULLET_H__
