// Filename: nurbsCurve.h
// Created by:  drose (01Mar01)
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

#ifndef NURBSCURVE_H
#define NURBSCURVE_H

// This header file includes either ClassicNurbsCurve or NurbsPPCurve;
// whichever one is actually typedeffed as "NurbsCurve".

#include "pandabase.h"

#ifdef HAVE_NURBSPP
#include "nurbsPPCurve.h"
#else
#include "classicNurbsCurve.h"
#endif

#endif

