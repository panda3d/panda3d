// Filename: config_net.h
// Created by:  drose (25Feb00)
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

#ifndef CONFIG_NET_H
#define CONFIG_NET_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

// Configure variables for net package.

NotifyCategoryDecl(net, EXPCL_PANDA, EXPTP_PANDA);

extern int get_net_max_write_queue();
extern int get_net_max_response_queue();
extern bool get_net_error_abort();

extern EXPCL_PANDA void init_libnet();

#endif

