// Filename: ppSubroutine.cxx
// Created by:  drose (10Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "ppSubroutine.h"

PPSubroutine::Subroutines PPSubroutine::_subroutines;

////////////////////////////////////////////////////////////////////
//     Function: PPSubroutine::define_sub
//       Access: Public, Static
//  Description: Adds a subroutine to the global list with the
//               indicated name.  The subroutine pointer must have
//               been recently allocated, and ownership of the pointer
//               will be passed to the global list; it may later
//               delete it if another subroutine is defined with the
//               same name.
////////////////////////////////////////////////////////////////////
void PPSubroutine::
define_sub(const string &name, PPSubroutine *sub) {
  Subroutines::iterator si;
  si = _subroutines.find(name);
  if (si == _subroutines.end()) {
    _subroutines.insert(Subroutines::value_type(name, sub));
  } else {
    delete (*si).second;
    (*si).second = sub;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPSubroutine::get_sub
//       Access: Public, Static
//  Description: Returns the previously-defined subroutine with the
//               given name, or NULL if there is no such subroutine
//               with that name.
////////////////////////////////////////////////////////////////////
const PPSubroutine *PPSubroutine::
get_sub(const string &name) {
  Subroutines::const_iterator si;
  si = _subroutines.find(name);
  if (si == _subroutines.end()) {
    return NULL;
  } else {
    return (*si).second;
  }
}
