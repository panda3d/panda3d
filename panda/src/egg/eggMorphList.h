// Filename: eggMorphList.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMORPHLIST_H
#define EGGMORPHLIST_H

#include <pandabase.h>

#include "eggMorph.h"

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : EggMorphList
// Description : A collection of <Dxyz>'s or <Duv>'s or some such.
////////////////////////////////////////////////////////////////////
template<class MorphType>
class EggMorphList : public vector<MorphType> {
public:
  pair<iterator, bool> insert(const MorphType &value);
  void write(ostream &out, int indent_level) const;
};


EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<EggMorphVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<EggMorphNormal>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<EggMorphTexCoord>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<EggMorphColor>)

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<EggMorphVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<EggMorphNormal>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<EggMorphTexCoord>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<EggMorphColor>)

typedef EggMorphList<EggMorphVertex> EggMorphVertexList;
typedef EggMorphList<EggMorphNormal> EggMorphNormalList;
typedef EggMorphList<EggMorphTexCoord> EggMorphTexCoordList;
typedef EggMorphList<EggMorphColor> EggMorphColorList;

#include "eggMorphList.I"

#endif
