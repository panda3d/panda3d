// Filename: pStatProperties.h
// Created by:  drose (17May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATPROPERTIES_H
#define PSTATPROPERTIES_H

#include <pandabase.h>

#ifdef DO_PSTATS

class PStatClient;
class PStatCollectorDef;

void initialize_collector_def(PStatClient *client, PStatCollectorDef *def);

#endif  // DO_PSTATS

#endif

