// Filename: eggMorphList.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMORPHLIST_H
#define EGGMORPHLIST_H

#include <pandabase.h>

#include "eggMorph.h"

#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : EggMorphList
// Description : A collection of <Dxyz>'s or <Duv>'s or some such.
////////////////////////////////////////////////////////////////////
template<class MorphType>
class EggMorphList : public set<MorphType> {
public:
  void write(ostream &out, int indent_level) const;
};

typedef EggMorphList<EggMorphVertex> EggMorphVertexList;
typedef EggMorphList<EggMorphNormal> EggMorphNormalList;
typedef EggMorphList<EggMorphTexCoord> EggMorphTexCoordList;
typedef EggMorphList<EggMorphColor> EggMorphColorList;

#include "eggMorphList.I"

#endif
