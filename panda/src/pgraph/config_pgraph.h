// Filename: config_pgraph.h
// Created by:  drose (21Feb02)
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

#ifndef CONFIG_PGRAPH_H
#define CONFIG_PGRAPH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

class DSearchPath;

ConfigureDecl(config_pgraph, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pgraph, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(loader, EXPCL_PANDA, EXPTP_PANDA);

extern const bool fake_view_frustum_cull;
extern const bool unambiguous_graph;
extern const bool paranoid_compose;
extern const bool compose_componentwise;

extern const bool m_dual;
extern const bool m_dual_opaque;
extern const bool m_dual_transparent;
extern const bool m_dual_flash;

extern const bool asynchronous_loads;
const DSearchPath &get_bam_path();

extern Config::ConfigTable::Symbol *load_file_type;

extern EXPCL_PANDA void init_libpgraph();

#endif
