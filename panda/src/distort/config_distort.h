/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_distort.h
 * @author drose
 * @date 2001-12-11
 */

#ifndef CONFIG_DISTORT_H
#define CONFIG_DISTORT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDecl(distort, EXPCL_PANDAFX, EXPTP_PANDAFX);

extern ConfigVariableBool project_invert_uvs;

extern EXPCL_PANDAFX void init_libdistort();

#endif /* CONFIG_DISTORT_H */
