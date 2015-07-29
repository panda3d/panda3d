// Filename: config_prc.cxx
// Created by:  drose (20Oct04)
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

#include "config_prc.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "pandaFileStreamBuf.h"

NotifyCategoryDef(prc, "");

ALIGN_16BYTE ConfigVariableBool assert_abort
("assert-abort", false,
 PRC_DESC("Set this true to trigger a core dump and/or stack trace when the first assertion fails"));
