// Filename: cLwoSurfaceBlockTMap.h
// Created by:  drose (30Apr01)
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

#ifndef CLWOSURFACEBLOCKTMAP_H
#define CLWOSURFACEBLOCKTMAP_H

#include "pandatoolbase.h"

#include "lwoSurfaceBlockTMap.h"
#include "lwoSurfaceBlockCoordSys.h"

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

  LPoint3 _center;
  LVecBase3 _size;
  LVecBase3 _rotation;

  string _reference_object;

  LwoSurfaceBlockCoordSys::Type _csys;

  LwoToEggConverter *_converter;
  CPT(LwoSurfaceBlockTMap) _tmap;
};

#include "cLwoSurfaceBlockTMap.I"

#endif


