// Filename: config_notify.h
// Created by:  drose (29Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NOTIFY_H
#define CONFIG_NOTIFY_H

#include "dtoolbase.h"
#include "dconfig.h"

ConfigureDecl(config_notify, EXPCL_DTOOLCONFIG, EXPTP_DTOOLCONFIG);

bool get_assert_abort();
bool get_notify_timestamp();

#endif /* __CONFIG_NOTIFY_H__ */
