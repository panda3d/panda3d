// Filename: config_dconfig.h
// Created by:  drose (15May00)
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
