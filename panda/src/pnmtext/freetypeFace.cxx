// Filename: freetypeFace.cxx
// Created by:  gogg (16Nov09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "freetypeFace.h"

#ifdef HAVE_FREETYPE

TypeHandle FreetypeFace::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFace::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FreetypeFace::
FreetypeFace() {
  _face = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFace::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FreetypeFace::
~FreetypeFace() {
  if (_face != NULL){
    FT_Done_Face(_face);
  }
}

#endif  // HAVE_FREETYPE
