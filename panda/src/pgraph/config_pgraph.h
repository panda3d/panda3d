// Filename: config_pgraph.h
// Created by:  drose (21Feb02)
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

#ifndef CONFIG_PGRAPH_H
#define CONFIG_PGRAPH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableList.h"

class DSearchPath;

ConfigureDecl(config_pgraph, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pgraph, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(loader, EXPCL_PANDA, EXPTP_PANDA);

extern ConfigVariableBool fake_view_frustum_cull;
extern ConfigVariableBool allow_portal_cull;
extern ConfigVariableBool unambiguous_graph;
extern ConfigVariableBool allow_unrelated_wrt;
extern ConfigVariableBool paranoid_compose;
extern ConfigVariableBool compose_componentwise;
extern ConfigVariableBool uniquify_matrix;
extern ConfigVariableBool paranoid_const;
extern ConfigVariableBool auto_break_cycles;
extern ConfigVariableInt max_collect_vertices;
extern ConfigVariableInt max_collect_indices;

extern ConfigVariableBool polylight_info;
extern ConfigVariableDouble lod_fade_time;

extern ConfigVariableBool show_vertex_animation;

extern ConfigVariableBool m_dual;
extern ConfigVariableBool m_dual_opaque;
extern ConfigVariableBool m_dual_transparent;
extern ConfigVariableBool m_dual_flash;

extern ConfigVariableBool asynchronous_loads;

extern ConfigVariableList load_file_type;
extern ConfigVariableList cull_bin;

extern EXPCL_PANDA void init_libpgraph();

#endif
