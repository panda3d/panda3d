// Filename: modelPool.I
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


////////////////////////////////////////////////////////////////////
//     Function: ModelPool::has_model
//       Access: Public, Static
//  Description: Returns true if the model has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ModelPool::
has_model(const string &filename) {
  return get_ptr()->ns_has_model(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::verify_model
//       Access: Public, Static
//  Description: Loads the given filename up as a model, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_model() with the same model name will
//               return a valid Node pointer.
////////////////////////////////////////////////////////////////////
INLINE bool ModelPool::
verify_model(const string &filename) {
  return load_model(filename) != (PandaNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::load_model
//       Access: Public, Static
//  Description: Loads the given filename up as a model, if it has
//               not already been loaded, and returns the new model.
//               If a model with the same filename was previously
//               loaded, returns that one instead.  If the model
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *ModelPool::
load_model(const string &filename) {
  return get_ptr()->ns_load_model(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::add_model
//       Access: Public, Static
//  Description: Adds the indicated already-loaded model to the
//               pool.  The model will always replace any
//               previously-loaded model in the pool that had the
//               same filename.
////////////////////////////////////////////////////////////////////
INLINE void ModelPool::
add_model(const string &filename, PandaNode *model) {
  get_ptr()->ns_add_model(filename, model);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::release_model
//       Access: Public, Static
//  Description: Removes the indicated model from the pool,
//               indicating it will never be loaded again; the model
//               may then be freed.  If this function is never called,
//               a reference count will be maintained on every model
//               every loaded, and models will never be freed.
////////////////////////////////////////////////////////////////////
INLINE void ModelPool::
release_model(const string &filename) {
  get_ptr()->ns_release_model(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::release_all_models
//       Access: Public, Static
//  Description: Releases all models in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void ModelPool::
release_all_models() {
  get_ptr()->ns_release_all_models();
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::garbage_collect
//       Access: Public, Static
//  Description: Releases only those models in the pool that have a
//               reference count of exactly 1; i.e. only those
//               models that are not being used outside of the pool.
//               Returns the number of models released.
////////////////////////////////////////////////////////////////////
INLINE int ModelPool::
garbage_collect() {
  return get_ptr()->ns_garbage_collect();
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::list_contents
//       Access: Public, Static
//  Description: Lists the contents of the model pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void ModelPool::
list_contents(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelPool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one ModelPool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE ModelPool::
ModelPool() {
}
