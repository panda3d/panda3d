// Filename: computedVerticesMakerEntity.I
// Created by:  drose (02Mar99)
// 
////////////////////////////////////////////////////////////////////

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMakerEntity::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
INLINE ComputedVerticesMakerEntity<ValueType, MorphType>::
ComputedVerticesMakerEntity(const ValueType &value, const MorphType &morphs)
  : _value(value), _morphs(morphs) {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMakerEntity::Ordering operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
bool ComputedVerticesMakerEntity<ValueType, MorphType>::
operator < (const ComputedVerticesMakerEntity<ValueType, MorphType> &other) const {
  // First, check the value.  This is some vector type, which we
  // compare componentwise.
  int compare = _value.compare_to(other._value);
  if (compare != 0) {
    return compare < 0;
  }

  // The values are identical; compare the morphs.
  return _morphs < other._morphs;
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMakerEntityMap::add_value
//       Access: Public
//  Description: Creates a new entry for the value type and morph
//               combination, if it is unique, and returns its new
//               index number, or returns the index number of a
//               previously-created, identical value type and morph.
//
//               The PTA table is updated as each new index number is
//               allocated so that table[index] == value.  It is also
//               used to determine the next available index number.
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
int ComputedVerticesMakerEntityMap<ValueType, MorphType>::
add_value(const ValueType &value, const MorphType &morphs,
          PTA(ValueType) &table) {
  // First, see if we have such an entity already.
  ComputedVerticesMakerEntity<ValueType, MorphType> entity(value, morphs);
  MapType::const_iterator mi = _map.find(entity);
  if (mi != _map.end()) {
    // We do!  Return its index number.
    return (*mi).second;
  }

  // No, this is the first time we've encountered this combination.
  // Define it.
  int index = table.size();
  table.push_back(value);
  _map[entity] = index;

  return index;
};

