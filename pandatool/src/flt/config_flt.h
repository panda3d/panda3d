// Filename: config_flt.h
// Created by:  drose (24Aug00)
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

#ifndef CONFIG_FLT_H
#define CONFIG_FLT_H

#include "pandatoolbase.h"

#include "notifyCategoryProxy.h"

NotifyCategoryDeclNoExport(flt);

extern const bool flt_error_abort;

extern void init_libflt();

static const int header_size = 4;

#endif
