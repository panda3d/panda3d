// Filename: config_chan.h
// Created by:  drose (28Feb00)
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

#ifndef CONFIG_CHAN_H
#define CONFIG_CHAN_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

// Configure variables for chan package.
NotifyCategoryDecl(chan, EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN);

EXPCL_PANDA_CHAN extern ConfigVariableBool compress_channels;
EXPCL_PANDA_CHAN extern ConfigVariableInt compress_chan_quality;
EXPCL_PANDA_CHAN extern ConfigVariableBool read_compressed_channels;
EXPCL_PANDA_CHAN extern ConfigVariableBool interpolate_frames;
EXPCL_PANDA_CHAN extern ConfigVariableBool restore_initial_pose;
EXPCL_PANDA_CHAN extern ConfigVariableInt async_bind_priority;

#endif
