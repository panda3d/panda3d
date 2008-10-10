// Filename: threadPriority.h
// Created by:  drose (08Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef THREADPRIORITY_H
#define THREADPRIORITY_H

#include "pandabase.h"

BEGIN_PUBLISH
////////////////////////////////////////////////////////////////////
// An enumerated type used by Thread to specify a suggested relative
// priority for a particular thread.
////////////////////////////////////////////////////////////////////
enum ThreadPriority {
  TP_low,
  TP_normal,
  TP_high,
  TP_urgent
};
END_PUBLISH

EXPCL_PANDA_PIPELINE ostream &
operator << (ostream &out, ThreadPriority pri);
EXPCL_PANDA_PIPELINE istream &
operator >> (istream &in, ThreadPriority &pri);


#endif
