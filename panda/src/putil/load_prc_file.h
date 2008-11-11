// Filename: load_prc_file.h
// Created by:  drose (22Oct04)
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

#ifndef LOAD_PRC_FILE_H
#define LOAD_PRC_FILE_H

#include "pandabase.h"
#include "filename.h"

class ConfigPage;
class HashVal;

BEGIN_PUBLISH
////////////////////////////////////////////////////////////////////
//     Function: load_prc_file
//  Description: A convenience function for loading explicit prc files
//               from a disk file or from within a multifile (via the
//               virtual file system).  Save the return value and pass
//               it to unload_prc_file() if you ever want to unload
//               this file later.
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
EXPCL_PANDA_PUTIL ConfigPage *
load_prc_file(const Filename &filename);

////////////////////////////////////////////////////////////////////
//     Function: load_prc_file_data
//  Description: Another convenience function to load a prc file from
//               an explicit string, which represents the contents of
//               the prc file.
//
//               The first parameter is an arbitrary name to assign to
//               this in-memory prc file.  Supply a filename if the
//               data was read from a file, or use any other name that
//               is meaningful to you.  The name is only used when the
//               set of loaded prc files is listed.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA_PUTIL ConfigPage *
load_prc_file_data(const string &name, const string &data);

////////////////////////////////////////////////////////////////////
//     Function: unload_prc_file
//  Description: Unloads (and deletes) a ConfigPage that represents a
//               prc file that was previously loaded by
//               load_prc_file().  Returns true if successful, false
//               if the file was unknown.
//
//               After this function has been called, the ConfigPage
//               pointer is no longer valid and should not be used
//               again.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA_PUTIL bool
unload_prc_file(ConfigPage *page);

#ifdef HAVE_OPENSSL

////////////////////////////////////////////////////////////////////
//     Function: hash_prc_variables
//  Description: Fills HashVal with the hash from the current prc file
//               state as reported by
//               ConfigVariableManager::write_prc_variables().
////////////////////////////////////////////////////////////////////
EXPCL_PANDA_PUTIL void
hash_prc_variables(HashVal &hash);

#endif  // HAVE_OPENSSL


END_PUBLISH

#endif
