// Filename: cLwoSurfaceBlockTMap.h
// Created by:  drose (30Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOSURFACEBLOCKTMAP_H
#define CLWOSURFACEBLOCKTMAP_H

#include <pandatoolbase.h>

#include <lwoSurfaceBlockTMap.h>
#include <lwoSurfaceBlockCoordSys.h>

#include <luse.h>

class LwoToEggConverter;

////////////////////////////////////////////////////////////////////
// 	 Class : CLwoSurfaceBlockTMap
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


