// Filename: cycleData.h
// Created by:  drose (21Feb02)
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

#ifndef CYCLEDATA_H
#define CYCLEDATA_H

#include "pandabase.h"

#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : CycleData
// Description : A single page of data maintained by a PipelineCycler.
//               Normally you should inherit from this class to define
//               the data structures that are important to protect
//               between stages of a pipeline.  See PipelineCycler.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CycleData : public ReferenceCount {
public:
  INLINE CycleData();
  virtual ~CycleData();

  virtual CycleData *make_copy() const=0;
};

#include "cycleData.I"

#endif

