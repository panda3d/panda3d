/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patcher.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "config_downloader.h"
#include "patcher.h"
#include "filename.h"

/**
 *
 */
Patcher::
Patcher() {
  PT(Buffer) buffer = new Buffer(patcher_buffer_size);
  init(buffer);
}

/**
 *
 */
Patcher::
Patcher(PT(Buffer) buffer) {
  init(buffer);
}

/**
 *
 */
void Patcher::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _buffer = buffer;

  _patchfile = nullptr;
  _patchfile = new Patchfile(_buffer);
}

/**
 *
 */
Patcher::
~Patcher() {
  delete _patchfile;
}

/**
 *
 */
int Patcher::
initiate(Filename &patch, Filename &infile) {
  return _patchfile->initiate(patch, infile);
}

/**
 *
 */
int Patcher::
run() {
  return _patchfile->run();
}

#endif  // HAVE_OPENSSL
