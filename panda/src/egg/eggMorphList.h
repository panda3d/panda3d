// Filename: eggMorphList.h
// Created by:  drose (29Jan99)
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

#ifndef EGGMORPHLIST_H
#define EGGMORPHLIST_H

#include "pandabase.h"

#include "eggMorph.h"

#include "indent.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : EggMorphList
// Description : A collection of <Dxyz>'s or <Duv>'s or some such.
////////////////////////////////////////////////////////////////////
template<class MorphType>
class EggMorphList {
private:
  typedef pvector<MorphType> Morphs;

public:
  typedef TYPENAME Morphs::iterator iterator;
  typedef TYPENAME Morphs::const_iterator const_iterator;
  typedef TYPENAME Morphs::size_type size_type;

  INLINE EggMorphList();
  INLINE EggMorphList(const EggMorphList<MorphType> &copy);
  INLINE void operator = (const EggMorphList<MorphType> &copy);
  INLINE ~EggMorphList();

  INLINE bool operator == (const EggMorphList<MorphType> &other) const;
  INLINE bool operator != (const EggMorphList<MorphType> &other) const;
  INLINE bool operator < (const EggMorphList<MorphType> &other) const;
  int compare_to(const EggMorphList<MorphType> &other) const;

  INLINE iterator begin();
  INLINE const_iterator begin() const;
  INLINE iterator end();
  INLINE const_iterator end() const;

  INLINE size_type size() const;
  INLINE bool empty() const;

  pair<iterator, bool> insert(const MorphType &value);
  INLINE void clear();

  void write(ostream &out, int indent_level,
             const string &tag, int num_dimensions) const;

private:
  Morphs _morphs;
};

// Export all of the vectors.

#define EXPCL EXPCL_PANDAEGG
#define EXPTP EXPTP_PANDAEGG
#define TYPE LVector3d
#define NAME vector_LVector3d
#include "vector_src.h"

#define EXPCL EXPCL_PANDAEGG
#define EXPTP EXPTP_PANDAEGG
#define TYPE LVector2d
#define NAME vector_LVector2d
#include "vector_src.h"

#define EXPCL EXPCL_PANDAEGG
#define EXPTP EXPTP_PANDAEGG
#define TYPE LVector4f
#define NAME vector_LVector4f
#include "vector_src.h"

typedef EggMorphList<EggMorphVertex> EggMorphVertexList;
typedef EggMorphList<EggMorphNormal> EggMorphNormalList;
typedef EggMorphList<EggMorphTexCoord> EggMorphTexCoordList;
typedef EggMorphList<EggMorphColor> EggMorphColorList;

#include "eggMorphList.I"

#endif
