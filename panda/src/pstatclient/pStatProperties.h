// Filename: pStatProperties.h
// Created by:  drose (17May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATPROPERTIES_H
#define PSTATPROPERTIES_H

#include <pandabase.h>


class PStatClient;
class PStatCollectorDef;

EXPCL_PANDA int get_current_pstat_major_version();
EXPCL_PANDA int get_current_pstat_minor_version();

#ifdef DO_PSTATS
void initialize_collector_def(PStatClient *client, PStatCollectorDef *def);
#endif  // DO_PSTATS

#endif

