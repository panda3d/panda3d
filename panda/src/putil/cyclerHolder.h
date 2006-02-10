// Filename: cyclerHolder.h
// Created by:  drose (09Feb06)
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

#ifndef CYCLERHOLDER_H
#define CYCLERHOLDER_H

#include "pandabase.h"
#include "pipelineCyclerBase.h"

////////////////////////////////////////////////////////////////////
//       Class : CyclerHolder
// Description : A lightweight C++ object whose constructor calls
//               lock() and whose destructor calls release() on a
//               PipelineCyclerBase object.  This is similar to a
//               MutexHolder.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CyclerHolder {
public:
  INLINE CyclerHolder(PipelineCyclerBase &cycler);
  INLINE ~CyclerHolder();
private:
  INLINE CyclerHolder(const CyclerHolder &copy);
  INLINE void operator = (const CyclerHolder &copy);

private:
#ifdef DO_PIPELINING
  PipelineCyclerBase *_cycler;
#endif
};

#include "cyclerHolder.I"

#endif
