// Filename: loaderFileType.cxx
// Created by:  drose (20Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "loaderFileType.h"

TypeHandle LoaderFileType::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileType::
LoaderFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileType::
~LoaderFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.  It is not necessary to call this before
//               calling load_file(), but it doesn't hurt; this is
//               useful for when the loader needs to know the full
//               pathname to the exact file it will be loading.
////////////////////////////////////////////////////////////////////
void LoaderFileType::
resolve_filename(Filename &) const {
}
