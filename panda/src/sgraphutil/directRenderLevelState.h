// Filename: directRenderLevelState.h
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DIRECTRENDERLEVELSTATE_H
#define DIRECTRENDERLEVELSTATE_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : DirectRenderLevelState
// Description : This is the state information the
//               DirectRenderTraverser retains for each level during
//               traversal.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectRenderLevelState {
public:
  bool _decal_mode;
};

#endif

