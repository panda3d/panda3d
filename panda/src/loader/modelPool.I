// Filename: modelPool.I
// Created by:  drose (25Aug00)
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
  return load_model(filename) != (Node *)NULL;
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
INLINE PT_Node ModelPool::
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
add_model(const string &filename, Node *model) {
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
//     Function: ModelPool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one ModelPool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE ModelPool::
ModelPool() {
}
