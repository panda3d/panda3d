// Filename: qpmodelPool.cxx
// Created by:  drose (12Mar02)
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

#include "qpmodelPool.h"
#include "loader.h"
#include "config_loader.h"


qpModelPool *qpModelPool::_global_ptr = (qpModelPool *)NULL;

static Loader _qpmodel_loader;

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_has_model
//       Access: Private
//  Description: The nonstatic implementation of has_model().
////////////////////////////////////////////////////////////////////
bool qpModelPool::
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
//     Function: qpModelPool::ns_load_model
//       Access: Private
//  Description: The nonstatic implementation of load_model().
////////////////////////////////////////////////////////////////////
PandaNode *qpModelPool::
ns_load_model(const string &filename) {
  Models::const_iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    // This model was previously loaded.
    return (*ti).second;
  }

  loader_cat.info()
    << "Loading model " << filename << "\n";
  PT(PandaNode) node = _qpmodel_loader.qpload_sync(filename);
  if (node.is_null()) {
    // This model was not found.
    return (PandaNode *)NULL;
  }

  _models[filename] = node;
  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_add_model
//       Access: Private
//  Description: The nonstatic implementation of add_model().
////////////////////////////////////////////////////////////////////
void qpModelPool::
ns_add_model(const string &filename, PandaNode *model) {
  // We blow away whatever model was there previously, if any.
  _models[filename] = model;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_release_model
//       Access: Private
//  Description: The nonstatic implementation of release_model().
////////////////////////////////////////////////////////////////////
void qpModelPool::
ns_release_model(const string &filename) {
  Models::iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    _models.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_release_all_models
//       Access: Private
//  Description: The nonstatic implementation of release_all_models().
////////////////////////////////////////////////////////////////////
void qpModelPool::
ns_release_all_models() {
  _models.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int qpModelPool::
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
      num_released++;
    } else {
      new_set.insert(new_set.end(), *ti);
    }
  }

  _models.swap(new_set);
  return num_released;
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void qpModelPool::
ns_list_contents(ostream &out) {
  out << _models.size() << " models:\n";
  Models::iterator ti;
  for (ti = _models.begin(); ti != _models.end(); ++ti) {
    out << "  " << (*ti).first
        << " (count = " << (*ti).second->get_ref_count() << ")\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpModelPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one qpModelPool object in the system.
////////////////////////////////////////////////////////////////////
qpModelPool *qpModelPool::
get_ptr() {
  if (_global_ptr == (qpModelPool *)NULL) {
    _global_ptr = new qpModelPool;
  }
  return _global_ptr;
}
