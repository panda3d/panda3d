// Filename: computedVerticesMorph.h
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

#ifndef COMPUTEDVERTICESMORPH_H
#define COMPUTEDVERTICESMORPH_H

#include "pandabase.h"
#include "vector_typedWritable.h"

#include "luse.h"

class CharacterSlider;
class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;
class TypedWritable;

////////////////////////////////////////////////////////////////////
//       Class : ComputedVerticesMorphValue2
// Description : This is a single vertex morph offset value.  This is
//               the amount by which the given vertex will be offset
//               when the morph is full on.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ComputedVerticesMorphValue2 {
public:
  typedef LVector2f VecType;

  INLINE ComputedVerticesMorphValue2(int index, const VecType &mvector);
  INLINE ComputedVerticesMorphValue2();

  ushort _index;
  VecType _vector;

public:
  void write_datagram(Datagram &dest);
  void read_datagram(DatagramIterator &source);
};

////////////////////////////////////////////////////////////////////
//       Class : ComputedVerticesMorphValue3
// Description : This is a single vertex morph offset value.  This is
//               the amount by which the given vertex will be offset
//               when the morph is full on.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ComputedVerticesMorphValue3 {
public:
  typedef LVector3f VecType;

  INLINE ComputedVerticesMorphValue3(int index, const VecType &mvector);
  INLINE ComputedVerticesMorphValue3();

  ushort _index;
  VecType _vector;

public:
  void write_datagram(Datagram &dest);
  void read_datagram(DatagramIterator &source);
};

////////////////////////////////////////////////////////////////////
//       Class : ComputedVerticesMorphValue4
// Description : This is a single vertex morph offset value.  This is
//               the amount by which the given vertex will be offset
//               when the morph is full on.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ComputedVerticesMorphValue4 {
public:
  typedef LVector4f VecType;

  INLINE ComputedVerticesMorphValue4(int index, const VecType &mvector);
  INLINE ComputedVerticesMorphValue4();

  ushort _index;
  VecType _vector;

public:
  void write_datagram(Datagram &dest);
  void read_datagram(DatagramIterator &source);
};

////////////////////////////////////////////////////////////////////
//       Class : ComputedVerticesMorph
// Description : A list of MorphValues, this represents the complete
//               effect of a given morph target on a particular class
//               of vertex values.
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphValueType>
class ComputedVerticesMorph {
public:
  INLINE ComputedVerticesMorph();
  INLINE ComputedVerticesMorph(const ComputedVerticesMorph &copy);

  typedef MorphValueType MorphValue;
  typedef pvector<MorphValue> Morphs;

  void output(ostream &out) const;

  short _slider_index;
  Morphs _morphs;

public:
  void write_datagram(Datagram &dest);
  void read_datagram(DatagramIterator &source);
};

template<class ValueType, class MorphValueType>
inline ostream &operator << (ostream &out, const ComputedVerticesMorph<ValueType, MorphValueType> &morph) {
  morph.output(out);
  return out;
}

typedef ComputedVerticesMorph<Vertexf, ComputedVerticesMorphValue3> ComputedVerticesMorphVertex;
typedef ComputedVerticesMorph<Normalf, ComputedVerticesMorphValue3> ComputedVerticesMorphNormal;
typedef ComputedVerticesMorph<TexCoordf, ComputedVerticesMorphValue2> ComputedVerticesMorphTexCoord;
typedef ComputedVerticesMorph<Colorf, ComputedVerticesMorphValue4> ComputedVerticesMorphColor;

#include "computedVerticesMorph.I"

#endif

