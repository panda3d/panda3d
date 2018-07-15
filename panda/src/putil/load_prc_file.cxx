/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_prc_file.cxx
 * @author drose
 * @date 2004-10-22
 */

#include "load_prc_file.h"
#include "configPageManager.h"
#include "configVariableManager.h"
#include "virtualFileSystem.h"
#include "config_express.h"
#include "config_putil.h"
#include "hashVal.h"

/**
 * A convenience function for loading explicit prc files from a disk file or
 * from within a multifile (via the virtual file system).  Save the return
 * value and pass it to unload_prc_file() if you ever want to unload this file
 * later.
 *
 * The filename is first searched along the default prc search path, and then
 * also along the model path, for convenience.
 *
 * This function is defined in putil instead of in dtool with the read of the
 * prc stuff, so that it can take advantage of the virtual file system (which
 * is defined in express), and the model path (which is in putil).
 */
ConfigPage *
load_prc_file(const Filename &filename) {
  Filename path = filename;
  path.set_text();

  ConfigPageManager *cp_mgr = ConfigPageManager::get_global_ptr();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, cp_mgr->get_search_path()) ||
    vfs->resolve_filename(path, get_model_path());

  std::istream *file = vfs->open_read_file(path, true);
  if (file == nullptr) {
    util_cat.error()
      << "Unable to open " << path << "\n";
    return nullptr;
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
    return nullptr;
  }
}

/**
 * Another convenience function to load a prc file from an explicit string,
 * which represents the contents of the prc file.
 *
 * The first parameter is an arbitrary name to assign to this in-memory prc
 * file.  Supply a filename if the data was read from a file, or use any other
 * name that is meaningful to you.  The name is only used when the set of
 * loaded prc files is listed.
 */
EXPCL_PANDA_PUTIL ConfigPage *
load_prc_file_data(const std::string &name, const std::string &data) {
  std::istringstream strm(data);

  ConfigPageManager *cp_mgr = ConfigPageManager::get_global_ptr();

  ConfigPage *page = cp_mgr->make_explicit_page(name);
  bool read_ok = page->read_prc(strm);

  if (read_ok) {
    page->set_trust_level(1);  // temp hack
    return page;

  } else {
    util_cat.info()
      << "Unable to read explicit prc data " << name << "\n";
    cp_mgr->delete_explicit_page(page);
    return nullptr;
  }
}

/**
 * Unloads (and deletes) a ConfigPage that represents a prc file that was
 * previously loaded by load_prc_file().  Returns true if successful, false if
 * the file was unknown.
 *
 * After this function has been called, the ConfigPage pointer is no longer
 * valid and should not be used again.
 */
bool
unload_prc_file(ConfigPage *page) {
  ConfigPageManager *cp_mgr = ConfigPageManager::get_global_ptr();
  return cp_mgr->delete_explicit_page(page);
}


#ifdef HAVE_OPENSSL

/**
 * Fills HashVal with the hash from the current prc file state as reported by
 * ConfigVariableManager::write_prc_variables().
 */
void
hash_prc_variables(HashVal &hash) {
  std::ostringstream strm;
  ConfigVariableManager *cv_mgr = ConfigVariableManager::get_global_ptr();
  cv_mgr->write_prc_variables(strm);
  hash.hash_string(strm.str());
}

#endif  // HAVE_OPENSSL
