// Filename: cLwoSurfaceBlockTMap.h
// Created by:  drose (30Apr01)
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

#ifndef CLWOSURFACEBLOCKTMAP_H
#define CLWOSURFACEBLOCKTMAP_H

#include "pandatoolbase.h"

#include <lwoSurfaceBlockTMap.h>
#include <lwoSurfaceBlockCoordSys.h>

#include "luse.h"

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
//       Class : CLwoSurfaceBlockTMap
// Description : This class is a wrapper around LwoSurfaceBlockTMap
//               and stores additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoSurfaceBlockTMap {
public:
  CLwoSurfaceBlockTMap(LwoToEggConverter *converter, const LwoSurfaceBlockTMap *tmap);

  void get_transform(LMatrix4d &mat) const;

  LPoint3f _center;
  LVecBase3f _size;
  LVecBase3f _rotation;

  string _reference_object;

  LwoSurfaceBlockCoordSys::Type _csys;

  LwoToEggConverter *_converter;
  CPT(LwoSurfaceBlockTMap) _tmap;
};

#include "cLwoSurfaceBlockTMap.I"

#endif


