// Filename: cLwoLayer.h
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOLAYER_H
#define CLWOLAYER_H

#include <pandatoolbase.h>

#include <lwoLayer.h>
#include <eggGroup.h>
#include <pointerTo.h>

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoLayer
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


