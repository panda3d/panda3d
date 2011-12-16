// Filename: config_linmath.h
// Created by:  drose (23Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_LINMATH_H
#define CONFIG_LINMATH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "lsimpleMatrix.h"

NotifyCategoryDecl(linmath, EXPCL_PANDA_LINMATH, EXPTP_PANDA_LINMATH);

extern EXPCL_PANDA_LINMATH ConfigVariableBool paranoid_hpr_quat;
extern EXPCL_PANDA_LINMATH ConfigVariableBool temp_hpr_fix;
extern EXPCL_PANDA_LINMATH ConfigVariableBool no_singular_invert;

extern EXPCL_PANDA_LINMATH void init_liblinmath();

#endif
