// Filename: allTransitionsWrapper.cxx
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "allTransitionsWrapper.h"
#include "nodeRelation.h"

#include <indent.h>

NodeTransitionCache AllTransitionsWrapper::_empty_cache;

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AllTransitionsWrapper::
output(ostream &out) const {
  if (_cache != (NodeTransitionCache *)NULL) {
    out << *_cache;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AllTransitionsWrapper::
write(ostream &out, int indent_level) const {
  if (_cache != (NodeTransitionCache *)NULL) {
    _cache->write(out, indent_level);
  }
}

