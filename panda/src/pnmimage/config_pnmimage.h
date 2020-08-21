/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pnmimage.h
 * @author drose
 * @date 2000-03-19
 */

#ifndef CONFIG_PNMIMAGE_H
#define CONFIG_PNMIMAGE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(pnmimage, EXPCL_PANDA_PNMIMAGE, EXPTP_PANDA_PNMIMAGE);

extern EXPCL_PANDA_PNMIMAGE ConfigVariableBool pfm_force_littleendian;
extern EXPCL_PANDA_PNMIMAGE ConfigVariableBool pfm_reverse_dimensions;
extern EXPCL_PANDA_PNMIMAGE ConfigVariableBool pfm_resize_gaussian;
extern EXPCL_PANDA_PNMIMAGE ConfigVariableBool pfm_resize_quick;
extern EXPCL_PANDA_PNMIMAGE ConfigVariableDouble pfm_resize_radius;

extern EXPCL_PANDA_PNMIMAGE void init_libpnmimage();

#endif
