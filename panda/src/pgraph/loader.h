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

#include "notify.h"
#include "pandaNode.h"
#include "filename.h"
#include "tokenBoard.h"
#include "asyncUtility.h"
#include "dSearchPath.h"

class LoaderToken;
class LoaderFileType;

////////////////////////////////////////////////////////////////////
//       Class : Loader
// Description : Handles database loading through asynchronous
//               threading
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Loader : public AsyncUtility {
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

  Loader();
  ~Loader();

  int find_all_files(const Filename &filename, const DSearchPath &search_path,
                     Results &results) const;

  INLINE PT(PandaNode) load_sync(const Filename &filename, bool search = true) const;

  uint request_load(const string &event_name, const Filename &filename, bool search = true);
  bool check_load(uint id);
  PT(PandaNode) fetch_load(uint id);

private:
  static void load_file_types();
  static bool _file_types_loaded;

  virtual bool process_request(void);
  PT(PandaNode) load_file(const Filename &filename, bool search) const;

  typedef TokenBoard<LoaderToken> LoaderTokenBoard;
  LoaderTokenBoard *_token_board;
};

#include "loader.I"

#endif
