// Filename: nodeTransitionCacheEntry.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransitionCacheEntry.h"

#include <indent.h>


////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionCacheEntry::
output(ostream &out) const {
  if (_trans != (NodeTransition *)NULL) {
    out << *_trans;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionCacheEntry::
write(ostream &out, int indent_level) const {
  if (_trans != (NodeTransition *)NULL) {
    _trans->write(out, indent_level);
  }
}
