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
#include "pandaFileStreamBuf.h"

NotifyCategoryDef(prc, "");

ConfigVariableBool assert_abort
("assert-abort", false,
 PRC_DESC("Set this true to trigger a core dump and/or stack trace when the first assertion fails"));

#ifdef USE_PANDAFILESTREAM

ConfigVariableEnum<PandaFileStreamBuf::NewlineMode> newline_mode
("newline-mode", PandaFileStreamBuf::NM_native,
  PRC_DESC("Controls how newlines are written by Panda applications writing "
  "to a text file.  The default, \"native\", means to write newlines "
  "appropriate to the current platform.  You may also specify \"binary\", "
  "to avoid molesting the file data, or one of \"msdos\", \"unix\", "
  "or \"mac\"."));
  
#endif  // USE_PANDAFILESTREAM
