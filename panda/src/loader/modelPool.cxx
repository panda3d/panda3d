// Filename: modelPool.cxx
// Created by:  drose (25Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "modelPool.h"
#include "loader.h"
#include "config_loader.h"


ModelPool *ModelPool::_global_ptr = (ModelPool *)NULL;

static Loader _model_loader;

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
PT_Node ModelPool::
ns_load_model(const string &filename) {
  Models::const_iterator ti;
  ti = _models.find(filename);
  if (ti != _models.end()) {
    // This model was previously loaded.
    return (*ti).second;
  }

  loader_cat.info()
    << "Loading model " << filename << "\n";
  PT(Node) node = _model_loader.load_sync(filename);
  if (node.is_null()) {
    // This model was not found.
    return (Node *)NULL;
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
ns_add_model(const string &filename, Node *model) {
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
    Node *node = (*ti).second;
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
//     Function: ModelPool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void ModelPool::
ns_list_contents(ostream &out) {
  out << _models.size() << " models:\n";
  Models::iterator ti;
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
