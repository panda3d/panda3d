// Filename: cLwoLayer.h
// Created by:  drose (25Apr01)
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

#ifndef CLWOLAYER_H
#define CLWOLAYER_H

#include "pandatoolbase.h"

#include "lwoLayer.h"
#include "eggGroup.h"
#include "pointerTo.h"

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
//       Class : CLwoLayer
// Description : This class is a wrapper around LwoLayer and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoLayer {
public:
  INLINE CLwoLayer(LwoToEggConverter *converter, const LwoLayer *layer);
  INLINE int get_number() const;

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoLayer) _layer;
  PT(EggGroup) _egg_group;
};

#include "cLwoLayer.I"

#endif


