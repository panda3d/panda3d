// Filename: config_graph.h
// Created by:  drose (01Oct99)
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
extern const bool EXPCL_PANDA paranoid_wrt;
extern const bool EXPCL_PANDA paranoid_graph;

#endif
