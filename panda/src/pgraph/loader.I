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
//               call, first create a new ModelLoadRequest object with
//               the filename you wish to load, and then add that
//               object to the Loader with load_async.  This function
//               will return immediately, and the model will be loaded
//               in the background.
//
//               To determine when the model has completely loaded,
//               you may poll request->is_ready() from time to time,
//               or set the done_event on the request object and
//               listen for that event.  When the model is ready, you
//               may retrieve it via request->get_model().
//
//               If threading support is not enabled, or the Loader
//               was created with 0 threads (that is,
//               get_num_threads() returns 0), then this will be the
//               same as a load_sync() call: the model will be loaded
//               within the current thread, and this method will not
//               return until the model has fully loaded.
////////////////////////////////////////////////////////////////////
INLINE void Loader::
load_async(AsyncTask *request) {
  add(request);
  poll();
}
