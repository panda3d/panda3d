// Filename: computedVerticesMakerEntity.h
// Created by:  drose (02Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef COMPUTEDVERTICESMAKERENTITY_H
#define COMPUTEDVERTICESMAKERENTITY_H

#include <pandabase.h>

#include <luse.h>
#include <eggMorphList.h>
#include <typedef.h>
#include <pointerToArray.h>

#include <map>

///////////////////////////////////////////////////////////////////
// 	 Class : ComputedVerticesMakerEntity
// Description : This represents a single vertex value, or color
//               value, or normal value, or some such thing, added to
//               the ComputedVerticesMaker.  This supports
//               ComputedVerticesMakerEntityMap, below, which is used
//               by ComputedVerticesMaker to collect together vertex
//               values with identical values.
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
class EXPCL_PANDAEGG ComputedVerticesMakerEntity {
public:
  INLINE ComputedVerticesMakerEntity(const ValueType &value,
				     const MorphType &morphs);
  bool
  operator < (const ComputedVerticesMakerEntity<ValueType, MorphType> &other) const;

  ValueType _value;
  const MorphType &_morphs;
};


///////////////////////////////////////////////////////////////////
// 	 Class : ComputedVerticesMakerEntityMap
// Description : A map of some kind of entity, above, to an integer
//               index number.  This collects together identical
//               vertices into a common index number.
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
class EXPCL_PANDAEGG ComputedVerticesMakerEntityMap {
public:
  int add_value(const ValueType &value, const MorphType &morphs,
		PTA(ValueType) &table);
 
  typedef map<ComputedVerticesMakerEntity<ValueType, MorphType>, int> MapType;
  MapType _map;
};

typedef ComputedVerticesMakerEntityMap<Vertexf, EggMorphVertexList> ComputedVerticesMakerVertexMap;
typedef ComputedVerticesMakerEntityMap<Normalf, EggMorphNormalList> ComputedVerticesMakerNormalMap;
typedef ComputedVerticesMakerEntityMap<TexCoordf, EggMorphTexCoordList> ComputedVerticesMakerTexCoordMap;
typedef ComputedVerticesMakerEntityMap<Colorf, EggMorphColorList> ComputedVerticesMakerColorMap;

#include "computedVerticesMakerEntity.I"

#endif
