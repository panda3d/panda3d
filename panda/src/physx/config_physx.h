/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_physx.h
 * @author enn0x
 * @date 2009-09-01
 */

#ifndef CONFIG_PHYSX_H
#define CONFIG_PHYSX_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "dconfig.h"

#include "physxEnums.h"

ConfigureDecl(config_physx, EXPCL_PANDAPHYSX, EXPTP_PANDAPHYSX);
NotifyCategoryDecl(physx, EXPCL_PANDAPHYSX, EXPTP_PANDAPHYSX);

extern EXPCL_PANDAPHYSX ConfigVariableBool physx_want_vrd;
extern EXPCL_PANDAPHYSX ConfigVariableString physx_vrd_host;
extern EXPCL_PANDAPHYSX ConfigVariableInt physx_vrd_port;
extern EXPCL_PANDAPHYSX ConfigVariableInt physx_internal_threads;
extern EXPCL_PANDAPHYSX ConfigVariableEnum<PhysxEnums::PhysxUpAxis> physx_up_axis;

extern EXPCL_PANDAPHYSX void init_libphysx();

#endif // CONFIG_PHYSX_H
