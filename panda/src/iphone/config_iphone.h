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

NotifyCategoryDecl(iphone, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern ConfigVariableBool iphone_autorotate_view;

extern EXPCL_PANDAGL void init_libiphone();
extern "C" EXPCL_PANDAGL int get_pipe_type_iphone();

#endif  // CONFIG_IPHONE_H
