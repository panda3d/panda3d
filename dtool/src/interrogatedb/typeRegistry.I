// Filename: typeRegistry.I
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::freshen_derivations
//       Access: Private
//  Description: Rebuilds the derivation data structures after some
//               derivation relationship has been modified, so that
//               class relationships can quickly be determined.
////////////////////////////////////////////////////////////////////
INLINE void TypeRegistry::
freshen_derivations() {
  if (!_derivations_fresh) {
    rebuild_derivations();
    _derivations_fresh = true;
  }
}
