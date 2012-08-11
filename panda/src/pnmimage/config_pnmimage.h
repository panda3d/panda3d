// Filename: config_pnmimage.h
// Created by:  drose (19Mar00)
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

#ifndef CONFIG_PNMIMAGE_H
#define CONFIG_PNMIMAGE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableBool.h"

NotifyCategoryDecl(pnmimage, EXPCL_PANDA_PNMIMAGE, EXPTP_PANDA_PNMIMAGE);

extern ConfigVariableBool pfm_force_littleendian;
extern ConfigVariableBool pfm_reverse_dimensions;
extern ConfigVariableInt pfm_vis_max_vertices;
extern ConfigVariableInt pfm_vis_max_indices;

extern EXPCL_PANDA_PNMIMAGE void init_libpnmimage();

#endif
