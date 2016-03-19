/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ode.h
 * @author joswilso
 * @date 2006-12-27
 */

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
