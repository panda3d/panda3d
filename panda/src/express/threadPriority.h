// Filename: threadPriority.h
// Created by:  drose (08Aug02)
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

#ifndef THREADPRIORITY_H
#define THREADPRIORITY_H

#include "pandabase.h"

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


#endif
