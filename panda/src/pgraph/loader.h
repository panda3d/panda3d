// Filename: loader.h
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

////////////////////////////////////////////////////////////////////
//       Class : Loader
// Description : A convenient class for loading models from disk, in
//               bam or egg format (or any of a number of other
//               formats implemented by a LoaderFileType, such as
//               ptloader).
//
//               This class supports synchronous as well as
//               asynchronous loading.  In asynchronous loading, the
//               model is loaded in the background by a thread, and an
//               event will be generated when the model is available.
//               If threading is not available, the asynchronous
//               loading interface may be used, but it loads
//               synchronously.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Loader : public AsyncTaskManager {
private:
  class ConsiderFile {
  public:
    Filename _path;
    LoaderFileType *_type;
  };

PUBLISHED:
  class EXPCL_PANDA Results {
  PUBLISHED:
    INLINE Results();
    INLINE Results(const Results &copy);
    INLINE void operator = (const Results &copy);
    INLINE ~Results();

    INLINE void clear();
    INLINE int get_num_files() const;
    INLINE const Filename &get_file(int n) const;
    INLINE LoaderFileType *get_file_type(int n) const;

  public:
    INLINE void add_file(const Filename &file, LoaderFileType *type);

  private:
    typedef pvector<ConsiderFile> Files;
    Files _files;
  };

  Loader(const string &name = "loader", int num_threads = -1);

  BLOCKING INLINE PT(PandaNode) load_sync(const Filename &filename, 
                                          const LoaderOptions &options = LoaderOptions()) const;

  INLINE void load_async(AsyncTask *request);

  virtual void output(ostream &out) const;

private:
  PT(PandaNode) load_file(const Filename &filename, const LoaderOptions &options) const;

  static void load_file_types();
  static bool _file_types_loaded;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTaskManager::init_type();
    register_type(_type_handle, "Loader",
                  AsyncTaskManager::get_class_type());
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
