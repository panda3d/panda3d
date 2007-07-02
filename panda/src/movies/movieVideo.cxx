// Filename: movieVideo.cxx
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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

#include "movieVideo.h"

TypeHandle MovieVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Constructor
//       Access: Protected
//  Description: Normally, the MovieVideo constructor is not
//               called directly; these are created by calling
//               the MoviePool::load functions.  Furthermore,
//               MovieVideo itself is just an abstract base class.
////////////////////////////////////////////////////////////////////
MovieVideo::
MovieVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieVideo::
~MovieVideo() {
}
 
