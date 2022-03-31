/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_prc.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIG_PRC_H
#define CONFIG_PRC_H

#include "dtoolbase.h"
#include "notifyCategoryProxy.h"

class ConfigVariableBool;

NotifyCategoryDecl(prc, EXPCL_DTOOL_PRC, EXPTP_DTOOL_PRC);

// This is aligned to match the shadowed definition in notify.cxx.
extern ALIGN_16BYTE ConfigVariableBool assert_abort;

#endif
