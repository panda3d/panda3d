// Filename: loader.cxx
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

#include "loader.h"
#include "loaderFileType.h"
#include "loaderFileTypeRegistry.h"
#include "config_pgraph.h"
#include "modelPool.h"
#include "modelLoadRequest.h"
#include "modelSaveRequest.h"
#include "config_express.h"
#include "config_util.h"
#include "virtualFileSystem.h"
#include "filename.h"
#include "load_dso.h"
#include "string_utils.h"
#include "bamCache.h"
#include "bamCacheRecord.h"
#include "sceneGraphReducer.h"
#include "renderState.h"
#include "bamFile.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"

bool Loader::_file_types_loaded = false;
PT(Loader) Loader::_global_ptr;
TypeHandle Loader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Loader::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Loader::
Loader(const string &name) :
  Namable(name)
{
  _task_manager = AsyncTaskManager::get_global_ptr();
  _task_chain = name;

  if (_task_manager->find_task_chain(_task_chain) == NULL) {
    PT(AsyncTaskChain) chain = _task_manager->make_task_chain(_task_chain);

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
    chain->set_num_threads(loader_num_threads);

    ConfigVariableEnum<ThreadPriority> loader_thread_priority
      ("loader-thread-priority", TP_low,
       PRC_DESC("The default thread priority to assign to the threads created "
                "for asynchronous loading.  The default is 'low'; you may "
                "also specify 'normal', 'high', or 'urgent'."));
    chain->set_thread_priority(loader_thread_priority);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::make_async_request
//       Access: Published
//  Description: Returns a new AsyncTask object suitable for adding to
//               load_async() to start an asynchronous model load.
////////////////////////////////////////////////////////////////////
PT(AsyncTask) Loader::
make_async_request(const Filename &filename, const LoaderOptions &options) {
  return new ModelLoadRequest(string("model:")+filename.get_basename(),
                              filename, options, this);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::make_async_save_request
//       Access: Published
//  Description: Returns a new AsyncTask object suitable for adding to
//               save_async() to start an asynchronous model save.
////////////////////////////////////////////////////////////////////
PT(AsyncTask) Loader::
make_async_save_request(const Filename &filename, const LoaderOptions &options,
                        PandaNode *node) {
  return new ModelSaveRequest(string("model_save:")+filename.get_basename(),
                              filename, options, node, this);
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_bam_stream
//       Access: Published
//  Description: Attempts to read a bam file from the indicated stream
//               and return the scene graph defined there.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
load_bam_stream(istream &in) {
  BamFile bam_file;
  if (!bam_file.open_read(in)) {
    return NULL;
  }

  return bam_file.read_node();
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Loader::
output(ostream &out) const {
  out << get_type() << " " << get_name();

  int num_tasks = _task_manager->make_task_chain(_task_chain)->get_num_tasks();
  if (num_tasks != 0) {
    out << " (" << num_tasks << " models pending)";
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
  Filename this_filename(filename);
  LoaderOptions this_options(options);

  bool report_errors = (this_options.get_flags() & LoaderOptions::LF_report_errors) != 0;

  string extension = this_filename.get_extension();
  if (extension.empty()) {
    // If the filename has no filename extension, append the default
    // extension specified in the Config file.
    this_filename = this_filename.get_fullpath() + default_model_extension.get_value();
    extension = this_filename.get_extension();
  }

  bool pz_file = false;
#ifdef HAVE_ZLIB
  if (extension == "pz") {
    pz_file = true;
    extension = Filename(this_filename.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB

  if (extension.empty()) {
    if (report_errors) {
      loader_cat.error()
        << "Cannot load " << this_filename
        << " without filename extension.  Loading of model filenames with an "
        "implicit extension is deprecated in Panda3D.  Please "
        "correct the filename reference.  If necessary, you may put the "
        "line \"default-model-extension .bam\" or \"default-model-extension .egg\" "
        "in your Config.prc to globally assume a particular model "
        "filename extension.\n";
    }
    return NULL;
  }

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  LoaderFileType *requested_type =
    reg->get_type_from_extension(extension);
  if (requested_type == (LoaderFileType *)NULL) {
    if (report_errors) {
      loader_cat.error()
        << "Extension of file " << this_filename
        << " is unrecognized; cannot load.\n";
      loader_cat.error(false)
        << "Currently known scene file types are:\n";
      reg->write(loader_cat.error(false), 2);
    }
    return NULL;
  } else if (!requested_type->supports_load()) {
    if (report_errors) {
      loader_cat.error()
        << requested_type->get_name() << " file type (."
        << extension << ") does not support loading.\n";
    }
    return NULL;
  } else if (pz_file && !requested_type->supports_compressed()) {
    if (report_errors) {
      loader_cat.error()
        << requested_type->get_name() << " file type (."
        << extension << ") does not support in-line compression.\n";
    }
    return NULL;
  }

  bool search = (this_options.get_flags() & LoaderOptions::LF_search) != 0;
  if (!filename.is_local()) {
    // If we have a global filename, we don't search the model path.
    search = false;
  }

  // Now that we've decided whether to search for the file, don't try
  // to search again.
  this_options.set_flags(this_options.get_flags() & ~LoaderOptions::LF_search);

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (search) {
    // Look for the file along the model path.
    const ConfigVariableSearchPath &model_path = get_model_path();
    int num_dirs = model_path.get_num_directories();
    for (int i = 0; i < num_dirs; ++i) {
      Filename pathname(model_path.get_directory(i), this_filename);
      PT(PandaNode) result = try_load_file(pathname, this_options,
                                           requested_type);
      if (result != (PandaNode *)NULL) {
        return result;
      }
    }

    if (report_errors) {
      bool any_exist = false;
      for (int i = 0; i < num_dirs; ++i) {
        Filename pathname(model_path.get_directory(i), this_filename);
        if (vfs->exists(pathname)) {
          any_exist = true;
          break;
        }
      }

      if (any_exist) {
        loader_cat.error()
          << "Couldn't load file " << this_filename
          << ": all matching files on model path invalid "
          << "(the model path is currently: \"" << get_model_path() << "\")\n";
      } else {
        loader_cat.error()
          << "Couldn't load file " << this_filename
          << ": not found on model path "
          << "(currently: \"" << get_model_path() << "\")\n";
      }
    }

  } else {
    // Look for the file only where it is.
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    PT(PandaNode) result = try_load_file(this_filename, this_options, requested_type);
    if (result != (PandaNode *)NULL) {
      return result;
    }
    if (report_errors) {
      if (vfs->exists(this_filename)) {
        loader_cat.error()
          << "Couldn't load file " << this_filename << ": invalid.\n";
      } else {
        loader_cat.error()
          << "Couldn't load file " << this_filename << ": does not exist.\n";
      }
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::try_load_file
//       Access: Private
//  Description: The implementatin of load_file(), this tries a single
//               possible file without searching further along the
//               path.
////////////////////////////////////////////////////////////////////
PT(PandaNode) Loader::
try_load_file(const Filename &pathname, const LoaderOptions &options,
              LoaderFileType *requested_type) const {
  BamCache *cache = BamCache::get_global_ptr();

  bool allow_ram_cache = requested_type->get_allow_ram_cache(options);

  if (allow_ram_cache) {
    // If we're allowing a RAM cache, use the ModelPool to load the
    // file.
    PT(PandaNode) node = ModelPool::get_model(pathname, true);
    if (node != (PandaNode *)NULL) {
      if ((options.get_flags() & LoaderOptions::LF_allow_instance) == 0) {
        if (loader_cat.is_debug()) {
          loader_cat.debug()
            << "Model " << pathname << " found in ModelPool.\n";
        }
        // But return a deep copy of the shared model.
        node = node->copy_subgraph();
      }
      return node;
    }
  }

  bool report_errors = ((options.get_flags() & LoaderOptions::LF_report_errors) != 0 || loader_cat.is_debug());

  PT(BamCacheRecord) record;
  if (cache->get_cache_models() && requested_type->get_allow_disk_cache(options)) {
    // See if the model can be found in the on-disk cache, if it is
    // active.
    record = cache->lookup(pathname, "bam");
    if (record != (BamCacheRecord *)NULL) {
      if (record->has_data()) {
        if (report_errors) {
          loader_cat.info()
            << "Model " << pathname << " found in disk cache.\n";
        }
        PT(PandaNode) result = DCAST(PandaNode, record->get_data());

        if (premunge_data) {
          SceneGraphReducer sgr;
          sgr.premunge(result, RenderState::make_empty());
        }

        if (result->is_of_type(ModelRoot::get_class_type())) {
          ModelRoot *model_root = DCAST(ModelRoot, result.p());
          model_root->set_fullpath(pathname);
          model_root->set_timestamp(record->get_source_timestamp());

          if (allow_ram_cache) {
            // Store the loaded model in the RAM cache, and make sure
            // we return a copy so that this node can be modified
            // independently from the RAM cached version.
            ModelPool::add_model(pathname, model_root);
            if ((options.get_flags() & LoaderOptions::LF_allow_instance) == 0) {
              return model_root->copy_subgraph();
            }
          }
        }
        return result;
      }
    }

    if (loader_cat.is_debug()) {
      loader_cat.debug()
        << "Model " << pathname << " not found in cache.\n";
    }
  }

  bool cache_only = (options.get_flags() & LoaderOptions::LF_cache_only) != 0;
  if (!cache_only) {
    // Load the model from disk.
    PT(PandaNode) result = requested_type->load_file(pathname, options, record);
    if (result != (PandaNode *)NULL) {
      if (record != (BamCacheRecord *)NULL) {
        // Store the loaded model in the model cache.
        record->set_data(result, result);
        cache->store(record);
      }

      if (premunge_data) {
        SceneGraphReducer sgr;
        sgr.premunge(result, RenderState::make_empty());
      }

      if (allow_ram_cache && result->is_of_type(ModelRoot::get_class_type())) {
        // Store the loaded model in the RAM cache, and make sure
        // we return a copy so that this node can be modified
        // independently from the RAM cached version.
        ModelPool::add_model(pathname, DCAST(ModelRoot, result.p()));
        if ((options.get_flags() & LoaderOptions::LF_allow_instance) == 0) {
          result = result->copy_subgraph();
        }
      }
      return result;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::save_file
//       Access: Private
//  Description: Saves a scene graph to a single file, if possible.
//               The file type written is implicit in the filename
//               extension.
////////////////////////////////////////////////////////////////////
bool Loader::
save_file(const Filename &filename, const LoaderOptions &options,
          PandaNode *node) const {
  Filename this_filename(filename);
  LoaderOptions this_options(options);

  bool report_errors = (this_options.get_flags() & LoaderOptions::LF_report_errors) != 0;

  string extension = this_filename.get_extension();
  if (extension.empty()) {
    // If the filename has no filename extension, append the default
    // extension specified in the Config file.
    this_filename = this_filename.get_fullpath() + default_model_extension.get_value();
    extension = this_filename.get_extension();
  }

  bool pz_file = false;
#ifdef HAVE_ZLIB
  if (extension == "pz") {
    pz_file = true;
    extension = Filename(this_filename.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB

  if (extension.empty()) {
    if (report_errors) {
      loader_cat.error()
        << "Cannot save " << this_filename
        << " without filename extension.\n";
    }
    return false;
  }

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  LoaderFileType *requested_type =
    reg->get_type_from_extension(extension);

  if (requested_type == (LoaderFileType *)NULL) {
    if (report_errors) {
      loader_cat.error()
        << "Extension of file " << this_filename
        << " is unrecognized; cannot save.\n";
      loader_cat.error(false)
        << "Currently known scene file types are:\n";
      reg->write(loader_cat.error(false), 2);
    }
    return false;

  } else if (!requested_type->supports_save()) {
    if (report_errors) {
      loader_cat.error()
        << requested_type->get_name() << " file type (."
        << extension << ") does not support saving.\n";
    }
    return false;

  } else if (pz_file && !requested_type->supports_compressed()) {
    if (report_errors) {
      loader_cat.error()
        << requested_type->get_name() << " file type (."
        << extension << ") does not support in-line compression.\n";
    }
    return false;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  bool result = try_save_file(this_filename, this_options, node, requested_type);
  if (!result) {
    if (report_errors) {
      loader_cat.error()
        << "Couldn't save file " << this_filename << ".\n";
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Loader::try_save_file
//       Access: Private
//  Description: The implementation of save_file(), this tries to
//               write a specific file type.
////////////////////////////////////////////////////////////////////
bool Loader::
try_save_file(const Filename &pathname, const LoaderOptions &options,
              PandaNode *node, LoaderFileType *requested_type) const {
  bool report_errors = ((options.get_flags() & LoaderOptions::LF_report_errors) != 0 || loader_cat.is_debug());

  bool result = requested_type->save_file(pathname, options, node);
  return result;
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
        void *tmp = load_dso(get_plugin_path().get_value(), dlname);
        if (tmp == (void *)NULL) {
          loader_cat.warning()
            << "Unable to load " << dlname.to_os_specific()
            << ": " << load_dso_error() << endl;
        } else if (loader_cat.is_debug()) {
          loader_cat.debug()
            << "done loading file type module: " << name << endl;
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
//     Function: Loader::make_global_ptr
//       Access: Private, Static
//  Description: Called once per application to create the global
//               loader object.
////////////////////////////////////////////////////////////////////
void Loader::
make_global_ptr() {
  nassertv(_global_ptr == (Loader *)NULL);

  _global_ptr = new Loader("loader");
}

