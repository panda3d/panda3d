/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelPool.I
 * @author drose
 * @date 2002-03-12
 */

/**
 * Returns true if the model has ever been loaded, false otherwise.  Note that
 * this does not guarantee that the model is still up-to-date.
 */
INLINE bool ModelPool::
has_model(const Filename &filename) {
  return get_ptr()->ns_has_model(filename);
}

/**
 * Loads the given filename up as a model, if it has not already been loaded,
 * and returns true to indicate success, or false to indicate failure.  If
 * this returns true, it is probable that a subsequent call to load_model()
 * with the same model name will return a valid PandaNode.
 *
 * However, even if this returns true, it is still possible for a subsequent
 * call to load_model() to fail.  This can happen if cache-check-timestamps is
 * true, and the on-disk file is subsequently modified to replace it with an
 * invalid model.
 */
INLINE bool ModelPool::
verify_model(const Filename &filename) {
  return load_model(filename) != nullptr;
}

/**
 * Returns the model that has already been previously loaded, or NULL
 * otherwise.  If verify is true, it will check if the file is still up-to-
 * date (and hasn't been modified in the meantime), and if not, will still
 * return NULL.
 */
INLINE ModelRoot *ModelPool::
get_model(const Filename &filename, bool verify) {
  return get_ptr()->ns_get_model(filename, verify);
}

/**
 * Loads the given filename up as a model, if it has not already been loaded,
 * and returns the new model.  If a model with the same filename was
 * previously loaded, returns that one instead (unless cache-check-timestamps
 * is true and the file has recently changed).  If the model file cannot be
 * found, or cannot be loaded for some reason, returns NULL.
 */
INLINE ModelRoot *ModelPool::
load_model(const Filename &filename, const LoaderOptions &options) {
  return get_ptr()->ns_load_model(filename, options);
}

/**
 * Adds the indicated already-loaded model to the pool.  The model will
 * replace any previously-loaded model in the pool that had the same filename.
 *
 * @deprecated Use the one-parameter add_model(model) instead.
 */
INLINE void ModelPool::
add_model(const Filename &filename, ModelRoot *model) {
  get_ptr()->ns_add_model(filename, model);
}

/**
 * Removes the indicated model from the pool, indicating it will never be
 * loaded again; the model may then be freed.  If this function is never
 * called, a reference count will be maintained on every model every loaded,
 * and models will never be freed.
 *
 * @deprecated Use release_model(model) instead.
 */
INLINE void ModelPool::
release_model(const Filename &filename) {
  get_ptr()->ns_release_model(filename);
}

/**
 * Adds the indicated already-loaded model to the pool.  The model will
 * replace any previously-loaded model in the pool that had the same filename.
 */
INLINE void ModelPool::
add_model(ModelRoot *model) {
  get_ptr()->ns_add_model(model);
}

/**
 * Removes the indicated model from the pool, indicating it will never be
 * loaded again; the model may then be freed.  If this function (and
 * garbage_collect()) is never called, a reference count will be maintained on
 * every model every loaded, and models will never be freed.
 *
 * The model's get_fullpath() value should not have been changed during its
 * lifetime, or this function may fail to locate it in the pool.
 */
INLINE void ModelPool::
release_model(ModelRoot *model) {
  get_ptr()->ns_release_model(model);
}

/**
 * Releases all models in the pool and restores the pool to the empty state.
 */
INLINE void ModelPool::
release_all_models() {
  get_ptr()->ns_release_all_models();
}

/**
 * Releases only those models in the pool that have a reference count of
 * exactly 1; i.e.  only those models that are not being used outside of the
 * pool.  Returns the number of models released.
 */
INLINE int ModelPool::
garbage_collect() {
  return get_ptr()->ns_garbage_collect();
}

/**
 * Lists the contents of the model pool to the indicated output stream.
 */
INLINE void ModelPool::
list_contents(std::ostream &out) {
  get_ptr()->ns_list_contents(out);
}

/**
 * Lists the contents of the model pool to cout.
 */
INLINE void ModelPool::
list_contents() {
  get_ptr()->ns_list_contents(std::cout);
}

/**
 * The constructor is not intended to be called directly; there's only
 * supposed to be one ModelPool in the universe and it constructs itself.
 */
INLINE ModelPool::
ModelPool() {
}
