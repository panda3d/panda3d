/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoLayer.h
 * @author drose
 * @date 2001-04-25
 */

#ifndef CLWOLAYER_H
#define CLWOLAYER_H

#include "pandatoolbase.h"

#include "lwoLayer.h"
#include "eggGroup.h"
#include "pointerTo.h"

class LwoToEggConverter;

/**
 * This class is a wrapper around LwoLayer and stores additional information
 * useful during the conversion-to-egg process.
 */
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
