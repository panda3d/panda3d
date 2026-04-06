/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cyclerHolder.h
 * @author drose
 * @date 2006-02-09
 */

#ifndef CYCLERHOLDER_H
#define CYCLERHOLDER_H

#include "pandabase.h"
#include "pipelineCyclerBase.h"

/**
 * A lightweight C++ object whose constructor calls acquire() and whose
 * destructor calls release() on a PipelineCyclerBase object.  This is similar
 * to a MutexHolder.
 */
class EXPCL_PANDA_PIPELINE CyclerHolder {
public:
  INLINE CyclerHolder(PipelineCyclerBase &cycler);
  CyclerHolder(const CyclerHolder &copy) = delete;
  INLINE ~CyclerHolder();

  CyclerHolder &operator = (const CyclerHolder &copy) = delete;

private:
#ifdef DO_PIPELINING
  PipelineCyclerBase *_cycler;
#endif
};

#include "cyclerHolder.I"

#endif
