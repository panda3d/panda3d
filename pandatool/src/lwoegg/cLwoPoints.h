// Filename: cLwoPoints.h
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOPOINTS_H
#define CLWOPOINTS_H

#include <pandatoolbase.h>

#include <lwoPoints.h>
#include <eggVertexPool.h>
#include <pointerTo.h>

#include <map>

class LwoToEggConverter;
class LwoVertexMap;
class CLwoLayer;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoPoints
// Description : This class is a wrapper around LwoPoints and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoPoints {
public:
  INLINE CLwoPoints(LwoToEggConverter *converter, const LwoPoints *points,
		    CLwoLayer *layer);

  void add_vmap(const LwoVertexMap *lwo_vmap);

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoPoints) _points;
  CLwoLayer *_layer;
  PT(EggVertexPool) _egg_vpool;

  // A number of vertex maps may be associated, by type and then by
  // name.
  typedef map<string, const LwoVertexMap *> VMapNames;
  typedef map<IffId, VMapNames> VmapTypes;
  VmapTypes _vmap;
};

#include "cLwoPoints.I"

#endif


