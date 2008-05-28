// Filename: cLwoClip.h
// Created by:  drose (26Apr01)
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

#ifndef CLWOCLIP_H
#define CLWOCLIP_H

#include "pandatoolbase.h"

#include "lwoClip.h"
#include "eggGroup.h"
#include "pointerTo.h"

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
//       Class : CLwoClip
// Description : This class is a wrapper around LwoClip and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoClip {
public:
  CLwoClip(LwoToEggConverter *converter, const LwoClip *clip);

  INLINE int get_index() const;
  INLINE bool is_still_image() const;

  LwoToEggConverter *_converter;
  CPT(LwoClip) _clip;

  Filename _filename;
  bool _still_image;
};

#include "cLwoClip.I"

#endif


