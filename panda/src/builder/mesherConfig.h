// Filename: mesherConfig.h
// Created by:  drose (27Oct00)
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
#ifndef MESHERCONFIG_H
#define MESHERCONFIG_H

#include "pandabase.h"

// This is just a file to declare a definition or two global to the
// mesher compilation.

// Define this to support making triangle fans in addition to triangle
// strips.  Fans may improve the grouping in certain models, although
// in most real cases the don't seem to help very much (and can
// actually hurt, by bitching the heuristic).
#define SUPPORT_FANS

// Define this to enable code to visualize the normals when generating
// primitives.  This creates line segment geometry to show each normal
// as the primitives are created.
#define SUPPORT_SHOW_NORMALS

#endif
