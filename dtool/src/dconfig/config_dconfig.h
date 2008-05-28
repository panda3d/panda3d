// Filename: config_dconfig.h
// Created by:  drose (15May00)
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

#ifndef CONFIG_DCONFIG_H
#define CONFIG_DCONFIG_H

#ifdef WIN32_VC
/* C4231: extern before template instantiation */
/* MPG - For some reason, this one only works if it's here */
#pragma warning (disable : 4231)
#endif

#include "dtoolbase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(dconfig, EXPCL_DTOOLCONFIG, EXPTP_DTOOLCONFIG);
NotifyCategoryDecl(microconfig, EXPCL_DTOOLCONFIG, EXPTP_DTOOLCONFIG);

#endif
