// Filename: computedVerticesMorph.I
// Created by:  drose (03Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "characterSlider.h"

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue2::
ComputedVerticesMorphValue2(int index, const VecType &mvector)
  : _index(index), _vector(mvector) {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue2::
ComputedVerticesMorphValue2(void){
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue3::
ComputedVerticesMorphValue3(int index, const VecType &mvector)
  : _index(index), _vector(mvector) {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue3::
ComputedVerticesMorphValue3(void){
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue4::
ComputedVerticesMorphValue4(int index, const VecType &mvector)
  : _index(index), _vector(mvector) {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorphValue4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVerticesMorphValue4::
ComputedVerticesMorphValue4(void){
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorph::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
ComputedVerticesMorph<ValueType, MorphValueType>::
ComputedVerticesMorph() {
  _slider_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorph::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
ComputedVerticesMorph<ValueType, MorphValueType>::
ComputedVerticesMorph(const ComputedVerticesMorph<ValueType, MorphValueType> &copy) :
  _slider_index(copy._slider_index),
  _morphs(copy._morphs)
{
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorph::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
void ComputedVerticesMorph<ValueType, MorphValueType>::
output(ostream &out) const {
  out << "morph index " << _slider_index << ": "
      << _morphs.size() << " points.";
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorph::write_datagram
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
void ComputedVerticesMorph<ValueType, MorphValueType>::
write_datagram(Datagram &dest)
{
  dest.add_int16(_slider_index);
  dest.add_uint16(_morphs.size());
  for(int i = 0; i < (int)_morphs.size(); i++)
  {
    _morphs[i].write_datagram(dest);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMorph::read_datagram
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
void ComputedVerticesMorph<ValueType, MorphValueType>::
read_datagram(DatagramIterator& source)
{
  _slider_index = source.get_int16();
  int size = source.get_uint16();
  for(int i = 0; i < size; i++)
  {
    MorphValue mv;
    mv.read_datagram(source);
    _morphs.push_back(mv);
  }
}
