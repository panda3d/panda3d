/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patcher.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef PATCHER_H
#define PATCHER_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "filename.h"
#include "buffer.h"
#include "patchfile.h"

/**
 * Applies a patch synchronously
 */
class EXPCL_PANDA_DOWNLOADER Patcher {
PUBLISHED:
  Patcher();
  explicit Patcher(PT(Buffer) buffer);
  virtual ~Patcher();

  int initiate(Filename &patch, Filename &infile);
  int run();

  INLINE PN_stdfloat get_progress() const;

private:
  void init(PT(Buffer) buffer);
  PT(Buffer) _buffer;
  Patchfile *_patchfile;
};

#include "patcher.I"

#endif  // HAVE_OPENSSL

#endif
