// Filename: config_pnmtext.h
// Created by:  drose (08Sep03)
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

#ifndef CONFIG_PNMTEXT_H
#define CONFIG_PNMTEXT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

class DSearchPath;

NotifyCategoryDecl(pnmtext, EXPCL_PANDA, EXPTP_PANDA);

extern const float text_point_size;
extern const float text_pixels_per_unit;
extern const float text_scale_factor;

extern EXPCL_PANDA void init_libpnmtext();

#endif
