// Filename: loader.I
// Created by:  mike (09Jan97)
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


////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
Results(const Loader::Results &copy) :
  _files(copy._files)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
operator = (const Loader::Results &copy) {
  _files = copy._files;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Loader::Results::
~Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::clear
//       Access: Published
//  Description: Removes all the files from the list.
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
clear() {
  _files.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_num_files
//       Access: Published
//  Description: Returns the number of files on the result list.
////////////////////////////////////////////////////////////////////
INLINE int Loader::Results::
get_num_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_file
//       Access: Published
//  Description: Returns the nth file on the result list.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Loader::Results::
get_file(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), _files[0]._path);
  return _files[n]._path;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::get_file_type
//       Access: Published
//  Description: Returns the file type of the nth file on the result
//               list.
////////////////////////////////////////////////////////////////////
INLINE LoaderFileType *Loader::Results::
get_file_type(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), NULL);
  return _files[n]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Results::add_file
//       Access: Published
//  Description: Adds a new file to the result list.
////////////////////////////////////////////////////////////////////
INLINE void Loader::Results::
add_file(const Filename &file, LoaderFileType *type) {
  ConsiderFile cf;
  cf._path = file;
  cf._type = type;
  _files.push_back(cf);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::set_task_manager
//       Access: Published
//  Description: Specifies the task manager that is used for
//               asynchronous loads.  The default is the global task
//               manager.
////////////////////////////////////////////////////////////////////
INLINE void Loader::
set_task_manager(AsyncTaskManager *task_manager) {
  _task_manager = task_manager;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::get_task_manager
//       Access: Published
//  Description: Returns the task manager that is used for
//               asynchronous loads.
////////////////////////////////////////////////////////////////////
INLINE AsyncTaskManager *Loader::
get_task_manager() const {
  return _task_manager;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::set_task_chain
//       Access: Published
//  Description: Specifies the task chain that is used for
//               asynchronous loads.  The default is the initial name
//               of the Loader object.
////////////////////////////////////////////////////////////////////
INLINE void Loader::
set_task_chain(const string &task_chain) {
  _task_chain = task_chain;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::get_task_chain
//       Access: Published
//  Description: Returns the task chain that is used for
//               asynchronous loads.
////////////////////////////////////////////////////////////////////
INLINE const string &Loader::
get_task_chain() const {
  return _task_chain;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::stop_threads
//       Access: Published
//  Description: Stop any threads used for asynchronous loads.
////////////////////////////////////////////////////////////////////
INLINE void Loader::
stop_threads() {
  PT(AsyncTaskChain) chain = _task_manager->find_task_chain(_task_chain);
  if (chain != (AsyncTaskChain *)NULL) {
    chain->stop_threads();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::remove
//       Access: Published
//  Description: Removes a pending asynchronous load request.  Returns
//               true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Loader::
remove(AsyncTask *task) {
  return _task_manager->remove(task);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_sync
//       Access: Published
//  Description: Loads the file immediately, waiting for it to
//               complete.
//
//               If search is true, the file is searched for along the
//               model path; otherwise, only the exact filename is
//               loaded.
////////////////////////////////////////////////////////////////////
INLINE PT(PandaNode) Loader::
load_sync(const Filename &filename, const LoaderOptions &options) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return load_file(filename, options);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_async
//       Access: Published
//  Description: Begins an asynchronous load request.  To use this
//               call, first call make_async_request() to create a new
//               ModelLoadRequest object with the filename you wish to
//               load, and then add that object to the Loader with
//               load_async.  This function will return immediately,
//               and the model will be loaded in the background.
//
//               To determine when the model has completely loaded,
//               you may poll request->is_ready() from time to time,
//               or set the done_event on the request object and
//               listen for that event.  When the model is ready, you
//               may retrieve it via request->get_model().
////////////////////////////////////////////////////////////////////
INLINE void Loader::
load_async(AsyncTask *request) {
  request->set_task_chain(_task_chain);
  _task_manager->add(request);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::save_sync
//       Access: Published
//  Description: Saves the file immediately, waiting for it to
//               complete.
////////////////////////////////////////////////////////////////////
INLINE bool Loader::
save_sync(const Filename &filename, const LoaderOptions &options,
          PandaNode *node) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return save_file(filename, options, node);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::save_async
//       Access: Published
//  Description: Begins an asynchronous save request.  To use this
//               call, first call make_async_save_request() to create
//               a new ModelSaveRequest object with the filename you
//               wish to load, and then add that object to the Loader
//               with save_async.  This function will return
//               immediately, and the model will be loaded in the
//               background.
//
//               To determine when the model has completely loaded,
//               you may poll request->is_ready() from time to time,
//               or set the done_event on the request object and
//               listen for that event.  When the request is ready,
//               you may retrieve the success or failure via
//               request->get_success().
////////////////////////////////////////////////////////////////////
INLINE void Loader::
save_async(AsyncTask *request) {
  request->set_task_chain(_task_chain);
  _task_manager->add(request);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::get_global_ptr
//       Access: Published
//  Description: Returns a pointer to the global Loader.  This is the
//               Loader that most code should use for loading models.
////////////////////////////////////////////////////////////////////
INLINE Loader *Loader::
get_global_ptr() {
  if (_global_ptr == (Loader *)NULL) {
    make_global_ptr();
  }
  return _global_ptr;
}
