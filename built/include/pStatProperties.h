/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatProperties.h
 * @author drose
 * @date 2001-05-17
 */

#ifndef PSTATPROPERTIES_H
#define PSTATPROPERTIES_H

#include "pandabase.h"


class PStatClient;
class PStatCollectorDef;

EXPCL_PANDA_PSTATCLIENT int get_current_pstat_major_version();
EXPCL_PANDA_PSTATCLIENT int get_current_pstat_minor_version();

#ifdef DO_PSTATS
void initialize_collector_def(const PStatClient *client, PStatCollectorDef *def);
#endif  // DO_PSTATS

#endif
