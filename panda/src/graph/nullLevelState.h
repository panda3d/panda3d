// Filename: nullLevelState.h
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NULLLEVELSTATE_H
#define NULLLEVELSTATE_H

#include <pandabase.h>

///////////////////////////////////////////////////////////////////
// 	 Class : NullLevelState
// Description : This is an empty class designed to be passed to a
//               traverser that doesn't care about tracking states
//               between levels.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullLevelState {
public:
  INLINE NullLevelState() { }
  INLINE NullLevelState(const NullLevelState &) { }
  INLINE void operator = (const NullLevelState &) { }
};

#endif

