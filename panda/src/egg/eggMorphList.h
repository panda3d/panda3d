/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMorphList.h
 * @author drose
 * @date 1999-01-29
 */

#ifndef EGGMORPHLIST_H
#define EGGMORPHLIST_H

#include "pandabase.h"

#include "eggMorph.h"

#include "indent.h"

#include "epvector.h"

/**
 * A collection of <Dxyz>'s or <Duv>'s or some such.
 */
template<class MorphType>
class EggMorphList {
private:
  typedef epvector<MorphType> Morphs;

public:
  typedef typename Morphs::iterator iterator;
  typedef typename Morphs::const_iterator const_iterator;
  typedef typename Morphs::size_type size_type;

  INLINE EggMorphList();
  INLINE EggMorphList(const EggMorphList<MorphType> &copy);
  INLINE void operator = (const EggMorphList<MorphType> &copy);
  INLINE ~EggMorphList();

  INLINE bool operator == (const EggMorphList<MorphType> &other) const;
  INLINE bool operator != (const EggMorphList<MorphType> &other) const;
  INLINE bool operator < (const EggMorphList<MorphType> &other) const;
  int compare_to(const EggMorphList<MorphType> &other, double threshold) const;

  INLINE iterator begin();
  INLINE const_iterator begin() const;
  INLINE iterator end();
  INLINE const_iterator end() const;

  INLINE size_type size() const;
  INLINE bool empty() const;

  std::pair<iterator, bool> insert(const MorphType &value);
  INLINE void clear();

  void write(std::ostream &out, int indent_level,
             const std::string &tag, int num_dimensions) const;

private:
  Morphs _morphs;
};

typedef EggMorphList<EggMorphVertex> EggMorphVertexList;
typedef EggMorphList<EggMorphNormal> EggMorphNormalList;
typedef EggMorphList<EggMorphTexCoord> EggMorphTexCoordList;
typedef EggMorphList<EggMorphColor> EggMorphColorList;

#include "eggMorphList.I"

#endif
