// Filename: config_iphone.h
// Created by:  drose (08Apr09)
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

#ifndef CONFIG_IPHONE_H
#define CONFIG_IPHONE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(iphone, EXPCL_MISC, EXPTP_MISC);

extern EXPCL_MISC void init_libiphone();

#endif  // CONFIG_IPHONE_H
