// Filename: showHideTransition.I
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition::
ShowHideTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::all_off
//       Access: Public
//  Description: This named constructor returns a ShowHideTransition
//               that's preconfigured to disable visibility for all
//               cameras.
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition ShowHideTransition::
all_off() {
  ShowHideTransition t;
  t.set_default_dir(TD_off);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::all_on
//       Access: Public
//  Description: This named constructor returns a ShowHideTransition
//               that's preconfigured to enable visibility for all
//               cameras.
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition ShowHideTransition::
all_on() {
  ShowHideTransition t;
  t.set_default_dir(TD_on);
  return t;
}
