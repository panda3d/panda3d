// Filename: loader.cxx
// Created by:  mike (09Jan97)
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

#include "loader.h"
#include "loaderFileType.h"
#include "loaderFileTypeRegistry.h"
#include "config_loader.h"

#include "event.h"
#include "pt_Event.h"
#include "throw_event.h"
#include "eventParameter.h"
#include "circBuffer.h"
#include "filename.h"
#include "load_dso.h"

#include "plist.h"
#include "pvector.h"
#include <algorithm>


bool Loader::_file_types_loaded = false;

////////////////////////////////////////////////////////////////////
//      Struct : LoaderToken
// Description : Holds a request for the loader (load or delete), as
//               well as the return information after the request has
//               completed.
////////////////////////////////////////////////////////////////////
class LoaderToken : public ReferenceCount {
public:
  INLINE LoaderToken(uint id, Filename path, const string &event_name,
        PandaNode *node=NULL) : _id(id), _node(node) {
    _path = path;
    _event_name = event_name;
  }
  uint _id;
  Filename _path;
  string _event_name;
  PT(PandaNode) _node;
};

////////////////////////////////////////////////////////////////////
//     Function: Loader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
Loader() : AsyncUtility() {
  _token_board = new LoaderTokenBoard;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
~Loader() {
  destroy_thread();
  delete _token_board;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::resolve_filename
//       Access: Public
//  Description: Looks for the given filename somewhere on the various
//               model paths.  (The filename extension is used to
//               determine which model paths are searched.)  If the
//               filename is found, updates the Filename to indicate
//               the full path; otherwise, leaves the Filename alone.
//
//               It is not necessary to call this before loading a
//               model; this is just a useful thing to have in case
//               you want to look for a file without loading it
//               immediately.
////////////////////////////////////////////////////////////////////
void Loader::
resolve_filename(Filename &filename) const {
  if (filename.is_fully_qualified()) {
    return;
  }

  string extension = filename.get_extension();

  if (extension.empty()) {
    resolve_unknown_file_type(filename);
    return;
  }

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();
  LoaderFileType *requested_type =
    reg->get_type_from_extension(extension);

  if (requested_type != (LoaderFileType *)NULL) {
    requested_type->resolve_filename(filename);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::request_load
//       Access: Public
//  Description: Requests an asynchronous load of a file.  The request
//               will be queued and served by the asynchronous thread.
//               If event_name is nonempty, it is the name of the
//               event that will be thrown (with the uint id as its
//               single parameter) when the loading is completed later.
//
//               The return value is an integer which can be used to
//               identify this particular request later to
//               fetch_load(), or 0 if there has been an error.
////////////////////////////////////////////////////////////////////
uint Loader::
request_load(const Filename &filename, const string &event_name) {
  if (!_file_types_loaded) {
    load_file_types();
  }

  PT(LoaderToken) tok;
  if (asynchronous_loads) {

    // Make sure we actually are threaded
    if (!_threaded) {
      loader_cat.info()
        << "Loader::request_load() - create_thread() was "
        << "never called!  Calling it now..." << endl;
      create_thread();
    }

    // We need to grab the lock in order to signal the condition variable
#ifdef HAVE_IPC
    _lock.lock();
#endif

      if (_token_board->_waiting.full()) {
        loader_cat.error()
          << "Loader::request_load() - Too many pending requests\n";
        return 0;
      }

      if (loader_cat.is_debug()) {
        loader_cat.debug()
          << "Load requested for file: " << filename << "\n";
      }

      tok = new LoaderToken(_next_token++, filename, event_name);
      _token_board->_waiting.push_back(tok);

#ifdef HAVE_IPC
      _request_cond->signal();
    _lock.unlock();
#endif

  } else {
    // If we're not running asynchronously, process the load request
    // directly now.
    if (_token_board->_waiting.full()) {
      loader_cat.error()
        << "Loader::request_load() - Too many pending requests\n";
      return 0;
    }

    if (loader_cat.is_debug()) {
      loader_cat.debug()
        << "Load requested for file: " << filename << "\n";
    }

    tok = new LoaderToken(_next_token++, filename, event_name);
    _token_board->_waiting.push_back(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::check_load
//       Access: Public
//  Description: Returns true if the indicated load-request has
//               completed and not yet been fetched, false otherwise.
////////////////////////////////////////////////////////////////////
bool Loader::
check_load(uint id) {
  return _token_board->is_done_token(id);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::fetch_load
//       Access: Public
//  Description: Returns the Node associated with the indicated id
//               number (returned by a previous call to request_load),
//               or NULL if the request has not yet completed.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
fetch_load(uint id) {
  PT(LoaderToken) tok = _token_board->get_done_token(id);
  if (tok.is_null()) {
    loader_cat.debug()
      << "Request to fetch id " << id << " which has not yet completed.\n";
    return NULL;
  }
  PT(PandaNode) node = tok->_node;
  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_file_types
//       Access: Private, Static
//  Description: Loads up all of the dynamic libraries named in a
//               load-file-type Configure variable.  Presumably this
//               will make the various file types available for
//               runtime loading.
////////////////////////////////////////////////////////////////////
void Loader::
load_file_types() {
  nassertv(load_file_type != (Config::ConfigTable::Symbol *)NULL);

  if (!_file_types_loaded) {
    Config::ConfigTable::Symbol::iterator ti;
    for (ti = load_file_type->begin(); ti != load_file_type->end(); ++ti) {
      Filename dlname = Filename::dso_filename("lib" + (*ti).Val() + ".so");
      loader_cat.info()
        << "loading file type module: " << dlname.to_os_specific() << endl;
      void *tmp = load_dso(dlname);
      if (tmp == (void *)NULL) {
        loader_cat.info()
          << "Unable to load: " << load_dso_error() << endl;
      }
    }
    _file_types_loaded = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::process_request
//       Access: Private
//  Description: Serves any requests on the token board, moving them
//               to the done queue.
////////////////////////////////////////////////////////////////////
bool Loader::
process_request() {
  if (_shutdown) {
    if (loader_cat.is_debug())
      loader_cat.debug()
          << "Loader shutting down...\n";
    return false;
  }

  // If there is actually a request token - process it
  while (!_token_board->_waiting.empty()) {
    PT(LoaderToken) tok = _token_board->_waiting.front();
    _token_board->_waiting.pop_front();
    tok->_node = load_file(tok->_path);
    if (tok->_node == NULL) {
      loader_cat.error()
        << "Loader::callback() - couldn't find file: "
        << tok->_path << "\n";
    } else {
      _token_board->_done.push_back(tok);

      // Throw a "done" event now.
      if (!tok->_event_name.empty()) {
        PT_Event done = new Event(tok->_event_name);
        done->add_parameter(EventParameter((int)tok->_id));
        throw_event(done);
      }
    }

    if (loader_cat.is_debug()) {
      loader_cat.debug()
        << "loading complete for " << tok->_path << "\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_file
//       Access: Private
//  Description: Loads a single scene graph file, if possible.
//               Returns the Node that is the root of the file, or
//               NULL if the file cannot be loaded.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
load_file(const Filename &filename) const {
  string extension = filename.get_extension();

  if (extension.empty()) {
    return load_unknown_file_type(filename);
  }

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();
  LoaderFileType *requested_type =
    reg->get_type_from_extension(extension);

  if (requested_type == (LoaderFileType *)NULL) {
    loader_cat.error()
      << "Extension of file " << filename
      << " is unrecognized; cannot load.\n";
    loader_cat.error(false)
      << "Currently known scene file types are:\n";
    reg->write_types(loader_cat.error(false), 2);
    return NULL;
  }

  Filename requested_filename = filename;
  if (!requested_filename.is_fully_qualified()) {
    // Ask the loader type to look for the file along its paths.
    requested_type->resolve_filename(requested_filename);
  }

  if (loader_cat.is_debug()) {
    loader_cat.debug()
      << "Loading " << requested_type->get_name() << " file: "
      << requested_filename << "\n";
  }

  PT(PandaNode) result = requested_type->load_file(requested_filename, true);
  return result;
}

class LoaderConsiderFile {
public:
  Filename _path;
  LoaderFileType *_type;

  bool operator < (const LoaderConsiderFile &other) const {
    return _path.compare_timestamps(other._path) > 0;
  }
};

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_unknown_file_type
//       Access: Private
//  Description: Attempts to guess which file is meant when a file
//               with no extension is given.  Looks around for a file
//               with a suitable extension for each of our known file
//               types, and loads the most recent file available of
//               any file type.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
load_unknown_file_type(const Filename &filename) const {
  typedef pvector<LoaderConsiderFile> Files;
  Files files;

  // First, build up a list of all of the possible files it could be.
  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();
  int num_types = reg->get_num_types();

  if (num_types == 0) {
    loader_cat.error()
      << "Can't load file " << filename
      << "; no scene file types are known.\n";
    return (PandaNode *)NULL;
  }

  for (int i = 0; i < num_types; i++) {
    LoaderConsiderFile consider;
    consider._type = reg->get_type(i);
    consider._path = filename;
    consider._path.set_extension(consider._type->get_extension());

    if (!consider._path.is_fully_qualified()) {
      // Ask the loader type to look for the file along its paths.
      consider._type->resolve_filename(consider._path);
    }

    if (consider._path.exists()) {
      files.push_back(consider);
    }
  }

  if (files.empty()) {
    loader_cat.error()
      << "Couldn't find file " << filename << " as:\n";
    for (int i = 0; i < num_types; i++) {
      Filename p = filename;
      p.set_extension(reg->get_type(i)->get_extension());
      loader_cat.error(false)
        << "  " << p << "\n";
    }
    return (PandaNode *)NULL;
  }

  // Now sort the list into order by timestamp, from newest to oldest.
  sort(files.begin(), files.end());

  // And try to load each file one at a time.
  Files::const_iterator fi;

  if (loader_cat.is_debug()) {
    loader_cat.debug()
      << "Loading " << filename << ", one of " << files.size()
      << " possible types:\n";
    for (fi = files.begin(); fi != files.end(); ++fi) {
      loader_cat.debug(false)
        << "  " << (*fi)._path << "\n";
    }
  }

  for (fi = files.begin(); fi != files.end(); ++fi) {
    const LoaderConsiderFile &consider = (*fi);
    PT(PandaNode) result = consider._type->load_file(consider._path, false);
    if (result != (PandaNode *)NULL) {
      return result;
    }
    if (loader_cat.is_debug()) {
      loader_cat.debug()
        << "Couldn't read " << consider._type->get_name()
        << " file " << consider._path << "\n";
    }
  }

  loader_cat.error()
    << "Cannot read " << files.front()._path << "\n";

  return (PandaNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::resolve_unknown_file_type
//       Access: Private
//  Description: Attempts to guess which file is meant when a file
//               with no extension is given.  Looks around for a file
//               with a suitable extension for each of our known file
//               types, and updates the filename if a suitable match
//               is found.
////////////////////////////////////////////////////////////////////
void Loader::
resolve_unknown_file_type(Filename &filename) const {
  typedef pvector<LoaderConsiderFile> Files;
  Files files;

  // First, build up a list of all of the possible files it could be.
  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();
  int num_types = reg->get_num_types();

  if (num_types == 0) {
    // No known file types!
    return;
  }

  for (int i = 0; i < num_types; i++) {
    LoaderConsiderFile consider;
    consider._type = reg->get_type(i);
    consider._path = filename;
    consider._path.set_extension(consider._type->get_extension());

    if (!consider._path.is_fully_qualified()) {
      // Ask the loader type to look for the file along its paths.
      consider._type->resolve_filename(consider._path);
    }

    if (consider._path.exists()) {
      files.push_back(consider);
    }
  }

  if (files.empty()) {
    // Couldn't find it anywhere.
    return;
  }

  // Now sort the list into order by timestamp, from newest to oldest.
  sort(files.begin(), files.end());

  // And get the first one.
  filename = files.front()._path;
}
