// Filename: config_cftalk.h
// Created by:  drose (26Mar09)
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

#ifndef CONFIG_CFTALK_H
#define CONFIG_CFTALK_H

#include "pandabase.h"
#include "windowProperties.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableString.h"
#include "configVariableList.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "configVariableFilename.h"
#include "coordinateSystem.h"
#include "dconfig.h"

#include "pvector.h"

ConfigureDecl(config_cftalk, EXPCL_CFTALK, EXPTP_CFTALK);
NotifyCategoryDecl(cftalk, EXPCL_CFTALK, EXPTP_CFTALK);

extern EXPCL_CFTALK void init_libcftalk();

#endif  /* CONFIG_CFTALK_H */
