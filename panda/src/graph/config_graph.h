// Filename: config_graph.h
// Created by:  drose (01Oct99)
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

#ifndef CONFIG_GRAPH_H
#define CONFIG_GRAPH_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(graph, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(wrt, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for graph package.
extern const bool EXPCL_PANDA cache_wrt;
extern const bool EXPCL_PANDA ambiguous_wrt_abort;
extern const bool EXPCL_PANDA paranoid_wrt;
extern const bool EXPCL_PANDA paranoid_graph;

extern EXPCL_PANDA void init_libgraph();

#endif
