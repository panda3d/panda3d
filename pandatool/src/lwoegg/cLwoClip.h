// Filename: cLwoClip.h
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOCLIP_H
#define CLWOCLIP_H

#include <pandatoolbase.h>

#include <lwoClip.h>
#include <eggGroup.h>
#include <pointerTo.h>

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoClip
// Description : This class is a wrapper around LwoClip and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoClip {
public:
  INLINE CLwoClip(LwoToEggConverter *converter, const LwoClip *clip);
  INLINE int get_number() const;

  LwoToEggConverter *_converter;
  CPT(LwoClip) _clip;
};

#include "cLwoClip.I"

#endif


