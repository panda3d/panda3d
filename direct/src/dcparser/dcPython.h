// Filename: dcPython.h
// Created by:  drose (22Jun04)
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

#ifndef DCPYTHON_H
#define DCPYTHON_H

// The only purpose of this file is to serve as a common place to put
// the nonsense associated with #including <Python.h>.

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#include <Python.h>

// Several interfaces in this module that use Python also require
// these header files, so we might as well pick them up too.
#include "datagram.h"
#include "datagramIterator.h"

#endif  // HAVE_PYTHON

#endif
