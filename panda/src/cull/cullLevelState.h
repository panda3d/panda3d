// Filename: cullLevelState.h
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CULLLEVELSTATE_H
#define CULLLEVELSTATE_H

#include <pandabase.h>

#include "cullStateLookup.h"

////////////////////////////////////////////////////////////////////
//       Class : CullLevelState
// Description : This is the state information the
//               CullTraverser retains for each level during
//               traversal.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullLevelState {
public:
  CullStateLookup *_lookup;
};

#endif

