// Filename: config_chan.h
// Created by:  drose (28Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CHAN_H
#define CONFIG_CHAN_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

// Configure variables for chan package.
NotifyCategoryDecl(chan, EXPCL_PANDA, EXPTP_PANDA);

EXPCL_PANDA extern bool compress_channels;
EXPCL_PANDA extern int compress_chan_quality;
EXPCL_PANDA extern bool read_compressed_channels;

#endif
