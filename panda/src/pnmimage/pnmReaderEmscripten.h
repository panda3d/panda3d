/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmReaderEmscripten.h
 * @author rdb
 * @date 2021-02-05
 */

#ifndef PNMREADEREMSCRIPTEN_H
#define PNMREADEREMSCRIPTEN_H

#include "pandabase.h"

#ifdef __EMSCRIPTEN__

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

/**
 * For reading files using the emscripten pre-load system.
 */
class PNMReaderEmscripten final : public PNMReader {
public:
  PNMReaderEmscripten(PNMFileType *type, Filename fullpath, int width, int height);

  virtual int read_data(xel *array, xelval *alpha);

private:
  Filename _fullpath;
};

#endif  // __EMSCRIPTEN__

#endif
