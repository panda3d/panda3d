// Filename: pStatClientVersion.cxx
// Created by:  drose (21May01)
// 
////////////////////////////////////////////////////////////////////

#include "pStatClientVersion.h"
#include "pStatProperties.h"


////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatClientVersion::
PStatClientVersion() {
  _major_version = get_current_pstat_major_version();
  _minor_version = get_current_pstat_minor_version();
}
