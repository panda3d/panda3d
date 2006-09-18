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
#include "modelPool.h"
#include "config_express.h"
#include "config_util.h"
#include "virtualFileSystem.h"
#include "filename.h"
#include "load_dso.h"
#include "string_utils.h"
#include "bamCache.h"
#include "bamCacheRecord.h"

bool Loader::_file_types_loaded = false;
TypeHandle Loader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Loader::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
Loader(const string &name, int num_threads) :
  AsyncTaskManager(name, num_threads)
{
  if (_num_threads < 0) {
    // -1 means the default number of threads.

    ConfigVariableInt loader_num_threads
      ("loader-num-threads", 1,
       PRC_DESC("The number of threads that will be started by the Loader class "
                "to load models asynchronously.  These threads will only be "
                "started if the asynchronous interface is used, and if threading "
                "support is compiled into Panda.  The default is one thread, "
                "which allows models to be loaded one at a time in a single "
                "asychronous thread.  You can set this higher, particularly if "
                "you have many CPU's available, to allow loading multiple models "
                "simultaneously."));

    _num_threads = loader_num_threads;
  }
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
  string extra_ext;
  bool pz_file = false;

#ifdef HAVE_ZLIB
  if (extension == "pz") {
    // The extension ".pz" is special.  This is an explicitly-named
    // compressed file.  We'll decompress it on the fly, if possible.
    // (This relies on the auto_unwrap parameter to vfs->read_file(),
    // which is different from the implicitly-named compressed file
    // action that you might get if vfs-implicit-pz is enabled.)
    extra_ext = extension;
    extension = Filename(filename.get_basename_wo_extension()).get_extension();
    pz_file = true;
  }
#endif  // HAVE_ZLIB

  int num_added = 0;

  if (!extension.empty()) {
    // If the extension is not empty, it specifies a single file type.
    LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
    LoaderFileType *requested_type =
      reg->get_type_from_extension(extension);

    if (requested_type != (LoaderFileType *)NULL &&
        (!pz_file || requested_type->supports_compressed())) {
      if (!filename.is_local()) {
        // Global filename, take it as it is.
        results.add_file(filename, requested_type);
        num_added++;

      } else {
        // Local filename, search along the path.
        DSearchPath::Results files;
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        num_added = vfs->find_all_files(filename, search_path, files);
        
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
        file = file.get_fullpath() + extra_ext;
          
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        if (vfs->exists(file)) {
          results.add_file(file, type);
          ++num_added;
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
          file = file.get_fullpath() + extra_ext;
          
          VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
          if (vfs->exists(file)) {
            results.add_file(file, type);
            ++num_added;
          }
        }
      }
    }
  }

  return num_added;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void Loader::
output(ostream &out) const {
  out << get_type() << " " << get_name();

  if (!_threads.empty()) {
    out << " (" << get_num_tasks() << " models pending)";
  }
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
  if (options.get_allow_ram_cache()) {
    // If we're allowing a RAM cache (and we don't have any other
    // funny options), use the ModelPool to load the file.
    PT(PandaNode) node = ModelPool::load_model(filename, options);
    if (node != (PandaNode *)NULL) {
      // But return a deep copy of the shared model.
      node = node->copy_subgraph();
    }
    return node;
  }

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
    bool pz_file = false;
#ifdef HAVE_ZLIB
    if (extension == "pz") {
      pz_file = true;
      extension = Filename(filename.get_basename_wo_extension()).get_extension();
    }
#endif  // HAVE_ZLIB
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
      } else if (pz_file && !requested_type->supports_compressed()) {
        loader_cat.error()
          << requested_type->get_name() << " file type (."
          << extension << ") does not support in-line compression.\n";
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

  BamCache *cache = BamCache::get_global_ptr();
  for (int i = 0; i < num_files; ++i) {
    const Filename &path = results.get_file(i);

    PT(BamCacheRecord) record;

    if (cache->get_active() && options.get_allow_disk_cache()) {
      // See if the texture can be found in the on-disk cache, if it is
      // active.
      record = cache->lookup(path, "bam");
      if (record != (BamCacheRecord *)NULL) {
        if (record->has_data()) {
          loader_cat.info()
            << "Model " << path << " found in disk cache.\n";
          return DCAST(PandaNode, record->extract_data());
        }
      }
    }

    LoaderFileType *type = results.get_file_type(i);
    PT(PandaNode) result = type->load_file(path, options, record);
    if (result != (PandaNode *)NULL){ 
      if (record != (BamCacheRecord *)NULL) {
        record->set_data(result, false);
        cache->store(record);
      }

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

