// Filename: loader.h
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
#ifndef LOADER_H
#define LOADER_H

#include "pandabase.h"

#include "notify.h"
#include "node.h"
#include "pt_Node.h"
#include "pandaNode.h"
#include "filename.h"
#include "tokenBoard.h"
#include "asyncUtility.h"

class LoaderToken;

////////////////////////////////////////////////////////////////////
//       Class : Loader
// Description : Handles database loading through asynchronous
//               threading
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Loader : public AsyncUtility {
PUBLISHED:
  Loader();
  ~Loader();

  void resolve_filename(Filename &filename) const;

  INLINE PT_Node load_sync(const Filename &filename) const;
  INLINE PT(PandaNode) qpload_sync(const Filename &filename) const;

  uint request_load(const Filename &filename, const string &event_name);
  bool check_load(uint id);
  PT_Node fetch_load(uint id);

private:
  static void load_file_types();
  static bool _file_types_loaded;

  virtual bool process_request(void);
  PT_Node load_file(const Filename &filename) const;
  PT_Node load_unknown_file_type(const Filename &filename) const;
  PT(PandaNode) qpload_file(const Filename &filename) const;
  PT(PandaNode) qpload_unknown_file_type(const Filename &filename) const;
  void resolve_unknown_file_type(Filename &filename) const;

  typedef TokenBoard<LoaderToken> LoaderTokenBoard;
  LoaderTokenBoard *_token_board;
};

#include "loader.I"

#endif
