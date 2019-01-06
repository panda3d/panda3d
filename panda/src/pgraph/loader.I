/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loader.I
 * @author mike
 * @date 1997-01-09
 */

/**
 *
 */
INLINE Loader::Results::
Results() {
}

/**
 *
 */
INLINE Loader::Results::
Results(const Loader::Results &copy) :
  _files(copy._files)
{
}

/**
 *
 */
INLINE void Loader::Results::
operator = (const Loader::Results &copy) {
  _files = copy._files;
}

/**
 *
 */
INLINE Loader::Results::
~Results() {
}

/**
 * Removes all the files from the list.
 */
INLINE void Loader::Results::
clear() {
  _files.clear();
}

/**
 * Returns the number of files on the result list.
 */
INLINE int Loader::Results::
get_num_files() const {
  return _files.size();
}

/**
 * Returns the nth file on the result list.
 */
INLINE const Filename &Loader::Results::
get_file(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), _files[0]._path);
  return _files[n]._path;
}

/**
 * Returns the file type of the nth file on the result list.
 */
INLINE LoaderFileType *Loader::Results::
get_file_type(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), nullptr);
  return _files[n]._type;
}

/**
 * Adds a new file to the result list.
 */
INLINE void Loader::Results::
add_file(const Filename &file, LoaderFileType *type) {
  ConsiderFile cf;
  cf._path = file;
  cf._type = type;
  _files.push_back(cf);
}

/**
 * Specifies the task manager that is used for asynchronous loads.  The
 * default is the global task manager.
 */
INLINE void Loader::
set_task_manager(AsyncTaskManager *task_manager) {
  _task_manager = task_manager;
}

/**
 * Returns the task manager that is used for asynchronous loads.
 */
INLINE AsyncTaskManager *Loader::
get_task_manager() const {
  return _task_manager;
}

/**
 * Specifies the task chain that is used for asynchronous loads.  The default
 * is the initial name of the Loader object.
 */
INLINE void Loader::
set_task_chain(const std::string &task_chain) {
  _task_chain = task_chain;
}

/**
 * Returns the task chain that is used for asynchronous loads.
 */
INLINE const std::string &Loader::
get_task_chain() const {
  return _task_chain;
}

/**
 * Stop any threads used for asynchronous loads.
 */
INLINE void Loader::
stop_threads() {
  PT(AsyncTaskChain) chain = _task_manager->find_task_chain(_task_chain);
  if (chain != nullptr) {
    chain->stop_threads();
  }
}

/**
 * Removes a pending asynchronous load request.  Returns true if successful,
 * false otherwise.
 * @deprecated use task.cancel() to cancel the request instead.
 */
INLINE bool Loader::
remove(AsyncTask *task) {
  return _task_manager->remove(task);
}

/**
 * Loads the file immediately, waiting for it to complete.
 *
 * If search is true, the file is searched for along the model path;
 * otherwise, only the exact filename is loaded.
 */
INLINE PT(PandaNode) Loader::
load_sync(const Filename &filename, const LoaderOptions &options) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return load_file(filename, options);
}

/**
 * Begins an asynchronous load request.  To use this call, first call
 * make_async_request() to create a new ModelLoadRequest object with the
 * filename you wish to load, and then add that object to the Loader with
 * load_async.  This function will return immediately, and the model will be
 * loaded in the background.
 *
 * To determine when the model has completely loaded, you may poll
 * request->is_ready() from time to time, or set the done_event on the request
 * object and listen for that event.  When the model is ready, you may
 * retrieve it via request->get_model().
 */
INLINE void Loader::
load_async(AsyncTask *request) {
  request->set_task_chain(_task_chain);
  _task_manager->add(request);
}

/**
 * Saves the file immediately, waiting for it to complete.
 */
INLINE bool Loader::
save_sync(const Filename &filename, const LoaderOptions &options,
          PandaNode *node) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return save_file(filename, options, node);
}

/**
 * Begins an asynchronous save request.  To use this call, first call
 * make_async_save_request() to create a new ModelSaveRequest object with the
 * filename you wish to load, and then add that object to the Loader with
 * save_async.  This function will return immediately, and the model will be
 * loaded in the background.
 *
 * To determine when the model has completely loaded, you may poll
 * request->is_ready() from time to time, or set the done_event on the request
 * object and listen for that event.  When the request is ready, you may
 * retrieve the success or failure via request->get_success().
 */
INLINE void Loader::
save_async(AsyncTask *request) {
  request->set_task_chain(_task_chain);
  _task_manager->add(request);
}

/**
 * Returns a pointer to the global Loader.  This is the Loader that most code
 * should use for loading models.
 */
INLINE Loader *Loader::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    make_global_ptr();
  }
  return _global_ptr;
}
