// Filename: cLwoPoints.h
// Created by:  drose (25Apr01)
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

#ifndef CLWOPOINTS_H
#define CLWOPOINTS_H

#include "pandatoolbase.h"

#include <lwoPoints.h>
#include <eggVertexPool.h>
#include "pointerTo.h"

#include "pmap.h"

class LwoToEggConverter;
class LwoVertexMap;
class CLwoLayer;

////////////////////////////////////////////////////////////////////
//       Class : CLwoPoints
// Description : This class is a wrapper around LwoPoints and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoPoints {
public:
  INLINE CLwoPoints(LwoToEggConverter *converter, const LwoPoints *points,
                    CLwoLayer *layer);

  void add_vmap(const LwoVertexMap *lwo_vmap);
  bool get_uv(const string &uv_name, int n, LPoint2f &uv) const;

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoPoints) _points;
  CLwoLayer *_layer;
  PT(EggVertexPool) _egg_vpool;

  // A number of vertex maps of different types may be associated, but
  // we only care about some of the types here.
  typedef pmap<string, const LwoVertexMap *> VMap;
  VMap _txuv;
  VMap _pick;
};

#include "cLwoPoints.I"

#endif


