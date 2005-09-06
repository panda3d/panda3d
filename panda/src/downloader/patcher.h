// Filename: patcher.h
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

#ifndef PATCHER_H
#define PATCHER_H

#include "pandabase.h"

#ifdef HAVE_SSL

#include "filename.h"
#include "buffer.h"
#include "patchfile.h"

////////////////////////////////////////////////////////////////////
//       Class : Patcher
// Description : Applies a patch synchronously
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patcher {
PUBLISHED:
  Patcher();
  Patcher(PT(Buffer) buffer);
  virtual ~Patcher();

  int initiate(Filename &patch, Filename &infile);
  int run();

  INLINE float get_progress() const;

private:
  void init(PT(Buffer) buffer);
  PT(Buffer) _buffer;
  Patchfile *_patchfile;
};

#include "patcher.I"

#endif  // HAVE_SSL

#endif
