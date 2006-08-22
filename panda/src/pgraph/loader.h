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
#include "thread.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "pvector.h"
#include "pdeque.h"

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
class EXPCL_PANDA Loader : public Namable {
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

  Loader(const string &name = "loader",
         int num_threads = -1);
  virtual ~Loader();

  int find_all_files(const Filename &filename, const DSearchPath &search_path,
                     Results &results) const;

  INLINE PT(PandaNode) load_sync(const Filename &filename, 
                                 const LoaderOptions &options = LoaderOptions()) const;

  int begin_request(const string &event_name);
  void request_load(int id, const Filename &filename,
                    const LoaderOptions &options = LoaderOptions());
  bool check_load(int id);
  PT(PandaNode) fetch_load(int id);

  virtual void output(ostream &out) const;

private:
  void poll_loader();
  PT(PandaNode) load_file(const Filename &filename, const LoaderOptions &options) const;

  static void load_file_types();
  static bool _file_types_loaded;

private:
  class LoaderThread : public Thread {
  public:
    LoaderThread(Loader *loader);
    virtual void thread_main();
    Loader *_loader;
  };

  typedef pvector< PT(LoaderThread) > Threads;

  class LoaderRequest {
  public:
    int _id;
    string _event_name;
    Filename _filename;
    LoaderOptions _options;
    PT(PandaNode) _model;
  };

  // We declare this a deque rather than a vector, on the assumption
  // that we will usually be popping requests from the front (although
  // the interface does not require this).
  typedef pdeque<LoaderRequest *> Requests;

  static int find_id(const Requests &requests, int id);

  int _num_threads;

  Mutex _lock;  // Protects all the following members.
  ConditionVar _cvar;  // condition: _pending.empty()

  enum State {
    S_initial,
    S_started,
    S_shutdown
  };

  Requests _initial, _pending, _finished;
  int _next_id;
  Threads _threads;
  State _state;

  friend class LoaderThread;
};

#include "loader.I"

#endif
