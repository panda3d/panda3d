// Filename: eggMorphList.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMORPHLIST_H
#define EGGMORPHLIST_H

#include <pandabase.h>

#include "eggMorph.h"

#include <indent.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : EggMorphList
// Description : A collection of <Dxyz>'s or <Duv>'s or some such.
////////////////////////////////////////////////////////////////////
template<class MorphType>
class EggMorphList {
private:
  typedef vector<MorphType> Morphs;

public:
  typedef Morphs::iterator iterator;
  typedef Morphs::const_iterator const_iterator;
  typedef Morphs::size_type size_type;

  INLINE EggMorphList();
  INLINE EggMorphList(const EggMorphList<MorphType> &copy);
  INLINE void operator = (const EggMorphList<MorphType> &copy);
  INLINE ~EggMorphList();

  INLINE bool operator == (const EggMorphList<MorphType> &other) const;
  INLINE bool operator != (const EggMorphList<MorphType> &other) const;
  INLINE bool operator < (const EggMorphList<MorphType> &other) const;

  INLINE iterator begin();
  INLINE const_iterator begin() const;
  INLINE iterator end();
  INLINE const_iterator end() const;

  INLINE size_type size() const;
  INLINE bool empty() const;

  pair<iterator, bool> insert(const MorphType &value);

  void write(ostream &out, int indent_level) const;

private:
  Morphs _morphs;
};

// Export all of the vectors.

#define EXPCL EXPCL_PANDAEGG 
#define EXPTP EXPTP_PANDAEGG 
#define TYPE LVector3d
#define NAME vector_LVector3d
#include <vector_src.h>

#define EXPCL EXPCL_PANDAEGG 
#define EXPTP EXPTP_PANDAEGG 
#define TYPE LVector2d
#define NAME vector_LVector2d
#include <vector_src.h>

#define EXPCL EXPCL_PANDAEGG 
#define EXPTP EXPTP_PANDAEGG 
#define TYPE LVector4f
#define NAME vector_LVector4f
#include <vector_src.h>

// Now export each EggMorphList.

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<LVector3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<LVector2d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorphList<LVector4f>)

typedef EggMorphList<EggMorphVertex> EggMorphVertexList;
typedef EggMorphList<EggMorphNormal> EggMorphNormalList;
typedef EggMorphList<EggMorphTexCoord> EggMorphTexCoordList;
typedef EggMorphList<EggMorphColor> EggMorphColorList;

#include "eggMorphList.I"

#endif
