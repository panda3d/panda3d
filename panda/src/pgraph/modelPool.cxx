// Filename: modelPool.cxx
// Created by:  drose (12Mar02)
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

#include "modelPool.h"
#include "loader.h"
#include "config_pgraph.h"


ModelPool *ModelPool::_global_ptr = (ModelPool *)NULL;

static Loader model_loader;

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::write
//       Access: Published, Static
//  Description: Lists the contents of the model pool to the
//               indicated output stream.
//               Helps with debugging.
////////////////////////////////////////////////////////////////////
void ModelPool::
write(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_has_model
//       Access: Private
//  Description: The nonstatic implementation of has_model().
////////////////////////////////////////////////////////////////////
bool ModelPool::
ns_has_model(const string &filename) {
  Models::const_iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    // This model was previously loaded.
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_load_model
//       Access: Private
//  Description: The nonstatic implementation of load_model().
////////////////////////////////////////////////////////////////////
PandaNode *ModelPool::
ns_load_model(const string &filename) {
  Models::const_iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    // This model was previously loaded.
    return (*ti).second;
  }

  loader_cat.info()
    << "Loading model " << filename << "\n";
  PT(PandaNode) node = model_loader.load_sync(filename);
  if (node.is_null()) {
    // This model was not found.
    return (PandaNode *)NULL;
  }

  _models[filename] = node;
  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_add_model
//       Access: Private
//  Description: The nonstatic implementation of add_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_add_model(const string &filename, PandaNode *model) {
  // We blow away whatever model was there previously, if any.
  _models[filename] = model;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_release_model
//       Access: Private
//  Description: The nonstatic implementation of release_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_release_model(const string &filename) {
  Models::iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    _models.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_release_all_models
//       Access: Private
//  Description: The nonstatic implementation of release_all_models().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_release_all_models() {
  _models.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int ModelPool::
ns_garbage_collect() {
  int num_released = 0;
  Models new_set;

  Models::iterator ti;
  for (ti = _models.begin(); ti != _models.end(); ++ti) {
    PandaNode *node = (*ti).second;
    if (node->get_ref_count() == 1) {
      if (loader_cat.is_debug()) {
        loader_cat.debug()
          << "Releasing " << (*ti).first << "\n";
      }
      ++num_released;
    } else {
      new_set.insert(new_set.end(), *ti);
    }
  }

  _models.swap(new_set);
  return num_released;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_list_contents(ostream &out) const {
  out << _models.size() << " models:\n";
  Models::const_iterator ti;
  for (ti = _models.begin(); ti != _models.end(); ++ti) {
    out << "  " << (*ti).first
        << " (count = " << (*ti).second->get_ref_count() << ")\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one ModelPool object in the system.
////////////////////////////////////////////////////////////////////
ModelPool *ModelPool::
get_ptr() {
  if (_global_ptr == (ModelPool *)NULL) {
    _global_ptr = new ModelPool;
  }
  return _global_ptr;
}
