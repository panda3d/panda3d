// Filename: config_effects.h
// Created by:  joswilso (27Dec06)
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

#ifndef CONFIG_ODE_H
#define CONFIG_ODE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

#include "dconfig.h"

NotifyCategoryDecl(ode, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odeworld, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odebody, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odejoint, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odespace, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odegeom, EXPCL_PANDAODE, EXPTP_PANDAODE);
NotifyCategoryDecl(odetrimeshdata, EXPCL_PANDAODE, EXPTP_PANDAODE);

extern EXPCL_PANDAODE void init_libode();

#endif /* CONFIG_ODE_H */
