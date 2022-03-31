/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pfmprogs.h
 * @author drose
 * @date 2010-12-23
 */

#ifndef CONFIG_PFM_H
#define CONFIG_PFM_H

#include "pandatoolbase.h"

#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"

NotifyCategoryDeclNoExport(pfm);

extern ConfigVariableDouble pfm_bba_dist;

extern void init_libpfm();

#endif
