// Filename: load_prc_file.cxx
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

#include "load_prc_file.h"
#include "configPageManager.h"
#include "virtualFileSystem.h"
#include "config_express.h"
#include "config_util.h"

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
ConfigPage *
load_prc_file(const string &filename) {
  Filename path = filename;
  path.set_text();

  ConfigPageManager *cp_mgr = ConfigPageManager::get_global_ptr();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, cp_mgr->get_search_path()) ||
    vfs->resolve_filename(path, get_model_path());
  
  istream *file = vfs->open_read_file(path);
  if (file == (istream *)NULL) {
    util_cat.error()
      << "Unable to open " << path << "\n";
    return NULL;
  }
  
  util_cat.info()
    << "Reading " << path << "\n";
  
  ConfigPage *page = cp_mgr->make_explicit_page(path);
  bool read_ok = page->read_prc(*file);
  vfs->close_read_file(file);
  
  if (read_ok) {
    return page;
    
  } else {
    util_cat.info()
      << "Unable to read " << path << "\n";
    cp_mgr->delete_explicit_page(page);
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: unload_prc_file
//  Description: Unloads (and deletes) a ConfigPage that represents a
//               prc file that was previously loaded by
//               load_prc_file().  Returns true if successful, false
//               if the file was unknown.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEXPRESS bool
unload_prc_file(ConfigPage *page) {
  ConfigPageManager *cp_mgr = ConfigPageManager::get_global_ptr();
  return cp_mgr->delete_explicit_page(page);
}

