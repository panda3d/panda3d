// Filename: fltToEggLevelState.I
// Created by:  drose (18Apr01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FltToEggLevelState::
FltToEggLevelState() {
  _flt_object = (FltObject *)NULL;
  _egg_parent = (EggGroupNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FltToEggLevelState::
FltToEggLevelState(const FltToEggLevelState &copy) :
  _flt_object(copy._flt_object),
  _egg_parent(copy._egg_parent)
{
  // We don't bother to copy the _parents map.
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FltToEggLevelState::
operator = (const FltToEggLevelState &copy) {
  _flt_object = copy._flt_object;
  _egg_parent = copy._egg_parent;
  // We don't bother to copy the _parents map.
}
