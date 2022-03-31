/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loader.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef LOADER_H
#define LOADER_H

#include "pandabase.h"

#include "namable.h"
#include "loaderOptions.h"
#include "pnotify.h"
#include "pandaNode.h"
#include "filename.h"
#include "dSearchPath.h"
#include "pvector.h"
#include "asyncTaskManager.h"
#include "asyncTask.h"

class LoaderFileType;

/**
 * A convenient class for loading models from disk, in bam or egg format (or
 * any of a number of other formats implemented by a LoaderFileType, such as
 * ptloader).
 *
 * This class supports synchronous as well as asynchronous loading.  In
 * asynchronous loading, the model is loaded in the background by a thread,
 * and an event will be generated when the model is available.  If threading
 * is not available, the asynchronous loading interface may be used, but it
 * loads synchronously.
 */
class EXPCL_PANDA_PGRAPH Loader : public TypedReferenceCount, public Namable {
private:
  class ConsiderFile {
  public:
    Filename _path;
    LoaderFileType *_type;
  };

PUBLISHED:
  class EXPCL_PANDA_PGRAPH Results {
  PUBLISHED:
    INLINE Results();
    INLINE Results(const Results &copy);
    INLINE void operator = (const Results &copy);
    INLINE ~Results();

    INLINE void clear();
    INLINE int get_num_files() const;
    INLINE const Filename &get_file(int n) const;
    MAKE_SEQ(get_files, get_num_files, get_file);
    INLINE LoaderFileType *get_file_type(int n) const;
    MAKE_SEQ(get_file_types, get_num_files, get_file_type);

  public:
    INLINE void add_file(const Filename &file, LoaderFileType *type);

  private:
    typedef pvector<ConsiderFile> Files;
    Files _files;
  };

  explicit Loader(const std::string &name = "loader");

  INLINE void set_task_manager(AsyncTaskManager *task_manager);
  INLINE AsyncTaskManager *get_task_manager() const;
  INLINE void set_task_chain(const std::string &task_chain);
  INLINE const std::string &get_task_chain() const;

  BLOCKING INLINE void stop_threads();
  INLINE bool remove(AsyncTask *task);

  BLOCKING INLINE PT(PandaNode) load_sync(const Filename &filename,
                                          const LoaderOptions &options = LoaderOptions()) const;

  PT(AsyncTask) make_async_request(const Filename &filename,
                                   const LoaderOptions &options = LoaderOptions());
  INLINE void load_async(AsyncTask *request);

  INLINE bool save_sync(const Filename &filename, const LoaderOptions &options,
                        PandaNode *node) const;
  PT(AsyncTask) make_async_save_request(const Filename &filename,
                                        const LoaderOptions &options,
                                        PandaNode *node);
  INLINE void save_async(AsyncTask *request);

  BLOCKING PT(PandaNode) load_bam_stream(std::istream &in);

  virtual void output(std::ostream &out) const;

  INLINE static Loader *get_global_ptr();

private:
  PT(PandaNode) load_file(const Filename &filename, const LoaderOptions &options) const;
  PT(PandaNode) try_load_file(const Filename &pathname, const LoaderOptions &options,
                              LoaderFileType *requested_type) const;

  bool save_file(const Filename &filename, const LoaderOptions &options,
                 PandaNode *node) const;
  bool try_save_file(const Filename &filename, const LoaderOptions &options,
                     PandaNode *node, LoaderFileType *requested_type) const;

  static void make_global_ptr();

  PT(AsyncTaskManager) _task_manager;
  std::string _task_chain;

  static void load_file_types();
  static bool _file_types_loaded;

  static PT(Loader) _global_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "Loader",
                  TypedReferenceCount::get_class_type(),
                  Namable::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ModelLoadRequest;
};

#include "loader.I"

#endif
