// Filename: config_distort.h
// Created by:  drose (11Dec01)
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

#ifndef CONFIG_DISTORT_H
#define CONFIG_DISTORT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(distort, EXPCL_PANDAFX, EXPTP_PANDAFX);

extern const bool project_invert_uvs;

extern EXPCL_PANDAFX void init_libdistort();

#endif /* CONFIG_DISTORT_H */
