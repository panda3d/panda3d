// Filename: modelPool.cxx
// Created by:  drose (12Mar02)
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

#include "modelPool.h"
#include "loader.h"
#include "config_pgraph.h"
#include "lightMutexHolder.h"
#include "virtualFileSystem.h"


ModelPool *ModelPool::_global_ptr = (ModelPool *)NULL;

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
ns_has_model(const Filename &filename) {
  LightMutexHolder holder(_lock);
  Models::const_iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end() && (*ti).second != (ModelRoot *)NULL) {
    // This model was previously loaded.
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_get_model
//       Access: Private
//  Description: The nonstatic implementation of get_model().
////////////////////////////////////////////////////////////////////
ModelRoot *ModelPool::
ns_get_model(const Filename &filename, bool verify) {

  PT(ModelRoot) cached_model;
  bool got_cached_model = false;

  {
    LightMutexHolder holder(_lock);
    Models::const_iterator ti;
    ti = _models.find(filename);
    if (ti != _models.end()) {
      // This filename was previously loaded.
      cached_model = (*ti).second;
      got_cached_model = true;
    }
  }

  if (got_cached_model && verify) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "ModelPool found " << cached_model << " for " << filename << "\n";
    }

    if (cached_model == NULL) {
      // This filename was previously attempted, but it did not
      // exist (or the model could not be loaded for some reason).
      if (cache_check_timestamps) {
        // Check to see if there is a file there now.
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        if (vfs->exists(filename)) {
          // There is, so try to load it.
          got_cached_model = false;
        }
      }
    } else {
      // This filename was previously attempted, and successfully
      // loaded.
      if (cache_check_timestamps && cached_model->get_timestamp() != 0 &&
          !cached_model->get_fullpath().empty()) {
        // Compare the timestamp to the file on-disk.
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        PT(VirtualFile) vfile = vfs->get_file(cached_model->get_fullpath());
        if (vfile == NULL) {
          // The file has disappeared!  Look further along the model-path.
          got_cached_model = false;

        } else if (vfile->get_timestamp() > cached_model->get_timestamp()) {
          // The file still exists, but it has a newer timestamp than
          // the one we previously loaded.  Force it to re-load.
          got_cached_model = false;
        }
      }
    }
  }

  if (got_cached_model) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "ModelPool returning " << cached_model << " for " << filename << "\n";
    }
    return cached_model;
  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_load_model
//       Access: Private
//  Description: The nonstatic implementation of load_model().
////////////////////////////////////////////////////////////////////
ModelRoot *ModelPool::
ns_load_model(const Filename &filename, const LoaderOptions &options) {

  // First check if it has already been loaded and is still current.
  PT(ModelRoot) cached_model = ns_get_model(filename, true);
  if (cached_model != (ModelRoot *)NULL) {
    return cached_model;
  }

  // Look on disk for the current file.
  LoaderOptions new_options(options);
  new_options.set_flags((new_options.get_flags() | LoaderOptions::LF_no_ram_cache) &
                        ~LoaderOptions::LF_search);

  Loader *model_loader = Loader::get_global_ptr();
  PT(PandaNode) panda_node = model_loader->load_sync(filename, new_options);
  PT(ModelRoot) node;

  if (panda_node.is_null()) {
    // This model was not found.

  } else {
    if (panda_node->is_of_type(ModelRoot::get_class_type())) {
      node = DCAST(ModelRoot, panda_node);

    } else {
      // We have to construct a ModelRoot node to put it under.
      node = new ModelRoot(filename);
      node->add_child(panda_node);
    }
    node->set_fullpath(filename);
  }

  {
    LightMutexHolder holder(_lock);

    // Look again, in case someone has just loaded the model in
    // another thread.
    Models::const_iterator ti;
    ti = _models.find(filename);
    if (ti != _models.end() && (*ti).second != cached_model) {
      // This model was previously loaded.
      return (*ti).second;
    }

    _models[filename] = node;
  }

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_add_model
//       Access: Private
//  Description: The nonstatic implementation of add_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_add_model(const Filename &filename, ModelRoot *model) {
  LightMutexHolder holder(_lock);
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "ModelPool storing " << model << " for " << filename << "\n";
  }
  // We blow away whatever model was there previously, if any.
  _models[filename] = model;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_release_model
//       Access: Private
//  Description: The nonstatic implementation of release_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_release_model(const Filename &filename) {
  LightMutexHolder holder(_lock);
  Models::iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    _models.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_add_model
//       Access: Private
//  Description: The nonstatic implementation of add_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_add_model(ModelRoot *model) {
  LightMutexHolder holder(_lock);
  // We blow away whatever model was there previously, if any.
  _models[model->get_fullpath()] = model;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_release_model
//       Access: Private
//  Description: The nonstatic implementation of release_model().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_release_model(ModelRoot *model) {
  LightMutexHolder holder(_lock);
  Models::iterator ti;
  ti = _models.find(model->get_fullpath());
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
  LightMutexHolder holder(_lock);
  _models.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int ModelPool::
ns_garbage_collect() {
  LightMutexHolder holder(_lock);

  int num_released = 0;
  Models new_set;

  Models::iterator ti;
  for (ti = _models.begin(); ti != _models.end(); ++ti) {
    ModelRoot *node = (*ti).second;
    if (node == (ModelRoot *)NULL ||
        node->get_model_ref_count() == 1) {
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
  LightMutexHolder holder(_lock);

  out << "model pool contents:\n";

  Models::const_iterator ti;
  int num_models = 0;
  for (ti = _models.begin(); ti != _models.end(); ++ti) {
    if ((*ti).second != NULL) {
      ++num_models;
      out << (*ti).first << "\n"
          << "  (count = " << (*ti).second->get_model_ref_count()
          << ")\n";
    }
  }

  out << "total number of models: " << num_models << " (plus "
      << _models.size() - num_models << " entries for nonexistent files)\n";
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
