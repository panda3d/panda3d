// Filename: eggMorph.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMORPH_H
#define EGGMORPH_H

#include <pandabase.h>

#include <namable.h>
#include <luse.h>
#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : EggMorph
// Description : A single <Dxyz> or <Duv> or some such entry.  This
//               simply contains the morph name and the offset value.
//               The class EggMorph is actually a template class on
//               the type of value that is being offset; the specific
//               kinds of morphs are instantiated from this below.
////////////////////////////////////////////////////////////////////
template<class Parameter>
class EggMorph : public Namable {
public:
  INLINE EggMorph(const string &name, const Parameter &offset);
  INLINE void set_offset(const Parameter &offset);
  INLINE const Parameter &get_offset() const;

  INLINE bool operator < (const EggMorph<Parameter> &other) const;
  INLINE bool operator == (const EggMorph<Parameter> &other) const;
  INLINE bool operator != (const EggMorph<Parameter> &other) const;

private:
  Parameter _offset;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorph<LVector3d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorph<LVector2d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, EggMorph<LVector4f>);

typedef EggMorph<LVector3d> EggMorphVertex;
typedef EggMorph<LVector3d> EggMorphNormal;
typedef EggMorph<LVector2d> EggMorphTexCoord;
typedef EggMorph<LVector4f> EggMorphColor;

INLINE ostream &operator << (ostream &out, const EggMorphVertex &m);
INLINE ostream &operator << (ostream &out, const EggMorphTexCoord &m);
INLINE ostream &operator << (ostream &out, const EggMorphColor &m);

// EggMorphNormal is, by virtue of equivalent typedefs, another name
// for EggMorphVertex.  Therefore we shouldn't define its output
// operator again.
//INLINE ostream &operator << (ostream &out, const EggMorphNormal &m);

#include "eggMorph.I"

#endif
