// Filename: load_prc_file.h
// Created by:  drose (22Oct04)
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

#ifndef LOAD_EGG_FILE_H
#define LOAD_EGG_FILE_H

#include "pandabase.h"

class ConfigPage;

BEGIN_PUBLISH
////////////////////////////////////////////////////////////////////
//     Function: load_prc_file
//  Description: A convenience function for loading explicit prc files
//               from a disk file or from within a multifile (via the
//               virtual file system).  Save the return value and pass
//               it to unload_prc_file() if you ever want to load this
//               file later.
//
//               The filename is first searched along the default prc
//               search path, and then also along the model path, for
//               convenience.
//
//               This function is defined in putil instead of in dtool
//               with the read of the prc stuff, so that it can take
//               advantage of the virtual file system (which is
//               defined in express), and the model path (which is in
//               putil).
////////////////////////////////////////////////////////////////////
EXPCL_PANDA ConfigPage *
load_prc_file(const string &filename);

////////////////////////////////////////////////////////////////////
//     Function: unload_prc_file
//  Description: Unloads (and deletes) a ConfigPage that represents a
//               prc file that was previously loaded by
//               load_prc_file().  Returns true if successful, false
//               if the file was unknown.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA bool
unload_prc_file(ConfigPage *page);
END_PUBLISH

#endif
