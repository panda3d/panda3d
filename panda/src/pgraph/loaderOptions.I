// Filename: loaderOptions.I
// Created by:  drose (05Oct05)
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
//     Function: LoaderOptions::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LoaderOptions::
LoaderOptions(int flags) : 
  _flags(flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LoaderOptions::
LoaderOptions(const LoaderOptions &copy) :
  _flags(copy._flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
operator = (const LoaderOptions &copy) {
  _flags = copy._flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::set_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
set_flags(int flags) {
  _flags = flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::get_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int LoaderOptions::
get_flags() const {
  return _flags;
}

