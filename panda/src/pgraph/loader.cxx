// Filename: loader.cxx
// Created by:  mike (09Jan97)
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

#include "loader.h"
#include "loaderFileType.h"
#include "loaderFileTypeRegistry.h"
#include "config_pgraph.h"

#include "config_express.h"
#include "config_util.h"
#include "virtualFileSystem.h"
#include "event.h"
#include "pt_Event.h"
#include "throw_event.h"
#include "eventParameter.h"
#include "circBuffer.h"
#include "filename.h"
#include "load_dso.h"
#include "string_utils.h"
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
  INLINE LoaderToken(uint id, const string &event_name, const Filename &path, 
                     bool search, PandaNode *node=NULL) : 
    _id(id), 
    _event_name(event_name), 
    _path(path),
    _search(search),
    _node(node) 
  { }
  uint _id;
  string _event_name;
  Filename _path;
  bool _search;
  PT(PandaNode) _node;
};

////////////////////////////////////////////////////////////////////
//     Function: Loader::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
Loader() : AsyncUtility() {
  _token_board = new LoaderTokenBoard;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
~Loader() {
  destroy_thread();
  delete _token_board;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::find_all_files
//       Access: Published
//  Description: Searches along the given search path for the given
//               file name, and fills up the results list with all
//               possible matches and their associated types, in
//               order.
////////////////////////////////////////////////////////////////////
int Loader::
find_all_files(const Filename &filename, const DSearchPath &search_path,
               Loader::Results &results) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  string extension = filename.get_extension();

  int num_added = 0;

  if (!extension.empty()) {
    // If the extension is not empty, it specifies a single file type.
    LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
    LoaderFileType *requested_type =
      reg->get_type_from_extension(extension);

    if (requested_type != (LoaderFileType *)NULL) {
      if (!filename.is_local()) {
        // Global filename, take it as it is.
        results.add_file(filename, requested_type);
        num_added++;

      } else {
        // Local filename, search along the path.
        DSearchPath::Results files;
        if (use_vfs) {
          VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
          num_added = vfs->find_all_files(filename, search_path, files);
        } else {
          num_added = search_path.find_all_files(filename, files);
        }
        
        for (int i = 0; i < num_added; ++i) {
          results.add_file(files.get_file(i), requested_type);
        }
      }
    }
  } else {
    // If the extension *is* empty, we have to search for all possible
    // file types.
    LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
    int num_types = reg->get_num_types();

    if (!filename.is_local()) {
      // Global filename, take it as it is.
      for (int t = 0; t < num_types; ++t) {
        LoaderFileType *type = reg->get_type(t);
        Filename file(filename);
        file.set_extension(type->get_extension());
          
        if (use_vfs) {
          VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
          if (vfs->exists(file)) {
            results.add_file(file, type);
            ++num_added;
          }
        } else {
          if (file.exists()) {
            results.add_file(file, type);
            ++num_added;
          }
        }
      }
    } else {
      // Local filename, look it up on the model path.
      int num_dirs = search_path.get_num_directories();
      for (int i = 0; i < num_dirs; ++i) {
        const Filename &directory = search_path.get_directory(i);
        
        for (int t = 0; t < num_types; ++t) {
          LoaderFileType *type = reg->get_type(t);
          Filename file(directory, filename);
          file.set_extension(type->get_extension());
          
          if (use_vfs) {
            VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
            if (vfs->exists(file)) {
              results.add_file(file, type);
              ++num_added;
            }
          } else {
            if (file.exists()) {
              results.add_file(file, type);
              ++num_added;
            }
          }
        }
      }
    }
  }

  return num_added;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::request_load
//       Access: Published
//  Description: Requests an asynchronous load of a file.  The request
//               will be queued and served by the asynchronous thread.
//               If event_name is nonempty, it is the name of the
//               event that will be thrown (with the uint id as its
//               single parameter) when the loading is completed later.
//
//               The return value is an integer which can be used to
//               identify this particular request later to
//               fetch_load(), or 0 if there has been an error.
//
//               If search is true, the file is searched for along the
//               model path; otherwise, only the exact filename is
//               loaded.
////////////////////////////////////////////////////////////////////
uint Loader::
request_load(const string &event_name, const Filename &filename, bool search) {
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
#ifdef OLD_HAVE_IPC
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

      tok = new LoaderToken(_next_token++, event_name, filename, search);
      _token_board->_waiting.push_back(tok);

#ifdef OLD_HAVE_IPC
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

    tok = new LoaderToken(_next_token++, event_name, filename, search);
    _token_board->_waiting.push_back(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::check_load
//       Access: Published
//  Description: Returns true if the indicated load-request has
//               completed and not yet been fetched, false otherwise.
////////////////////////////////////////////////////////////////////
bool Loader::
check_load(uint id) {
  return _token_board->is_done_token(id);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::fetch_load
//       Access: Published
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
  if (!_file_types_loaded) {
    int num_unique_values = load_file_type.get_num_unique_values();

    for (int i = 0; i < num_unique_values; i++) {
      string param = load_file_type.get_unique_value(i);

      vector_string words;
      extract_words(param, words);

      if (words.size() == 1) {
        // Exactly one word: load the named library immediately.
        string name = words[0];
        Filename dlname = Filename::dso_filename("lib" + name + ".so");
        loader_cat.info()
          << "loading file type module: " << name << endl;
        void *tmp = load_dso(dlname);
        if (tmp == (void *)NULL) {
          loader_cat.warning()
            << "Unable to load " << dlname.to_os_specific()
            << ": " << load_dso_error() << endl;
        }
        
      } else if (words.size() > 1) {
        // Multiple words: the first n words are filename extensions,
        // and the last word is the name of the library to load should
        // any of those filename extensions be encountered.
        LoaderFileTypeRegistry *registry = LoaderFileTypeRegistry::get_global_ptr();
        size_t num_extensions = words.size() - 1;
        string library_name = words[num_extensions];
        
        for (size_t i = 0; i < num_extensions; i++) {
          string extension = words[i];
          if (extension[0] == '.') {
            extension = extension.substr(1);
          }
          
          registry->register_deferred_type(extension, library_name);
        }
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
    tok->_node = load_file(tok->_path, tok->_search);
    if (tok->_node == (PandaNode *)NULL) {
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
//
//               If search is true, the file is searched for along the
//               model path; otherwise, only the exact filename is
//               loaded.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
load_file(const Filename &filename, const LoaderOptions &options) const {
  Results results;
  int num_files;

  bool search = (options.get_flags() & LoaderOptions::LF_search) != 0;

  if (search) {
    // Look for the file along the model path.
    num_files = find_all_files(filename, get_model_path(), results);
  } else {
    // Look for the file only where it is.
    num_files = find_all_files(filename, DSearchPath(Filename(".")), results);
  }

  if (num_files == 0) {
    // Couldn't find the file.  Either it doesn't exist, or it's an
    // unknown file type.  Report a useful message either way.
    string extension = filename.get_extension();
    if (!extension.empty()) {
      LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
      LoaderFileType *requested_type =
        reg->get_type_from_extension(extension);
      if (requested_type == (LoaderFileType *)NULL) {
        loader_cat.error()
          << "Extension of file " << filename
          << " is unrecognized; cannot load.\n";
        loader_cat.error(false)
          << "Currently known scene file types are:\n";
        reg->write(loader_cat.error(false), 2);
        return NULL;
      }
    }

    if (search) {
      loader_cat.error()
        << "Couldn't load file " << filename << ": not found on model path "
        << "(which is currently: \"" << get_model_path() << "\")\n";
    } else {
      loader_cat.error()
        << "Couldn't load file ./" << filename << ": does not exist.\n";
    }
    return NULL;
  }

  for (int i = 0; i < num_files; ++i) {
    const Filename &path = results.get_file(i);
    LoaderFileType *type = results.get_file_type(i);
    PT(PandaNode) result = type->load_file(path, options);
    if (result != (PandaNode *)NULL) {
      return result;
    }
  }

  // None of the matching files could be loaded.  Oh well.
  if (search) {
    loader_cat.error()
      << "Couldn't load file " << filename
      << ": all matching files on model path invalid "
      << "(the model path is currently: \"" << get_model_path() << "\")\n";
  } else {
    loader_cat.error()
      << "Couldn't load file " << filename
      << ": invalid.\n";
  }
  return NULL;
}

