// Filename: cLwoPolygons.h
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOPOLYGONS_H
#define CLWOPOLYGONS_H

#include <pandatoolbase.h>

#include <lwoPolygons.h>
#include <eggGroup.h>
#include <pointerTo.h>

class LwoToEggConverter;
class CLwoPoints;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoPolygons
// Description : This class is a wrapper around LwoPolygons and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoPolygons {
public:
  INLINE CLwoPolygons(LwoToEggConverter *converter, 
		      const LwoPolygons *polygons,
		      CLwoPoints *points);

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoPolygons) _polygons;
  CLwoPoints *_points;
  PT(EggGroup) _egg_group;

private:
  void make_faces();
};

#include "cLwoPolygons.I"

#endif


