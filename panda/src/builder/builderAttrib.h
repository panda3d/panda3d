// Filename: builderAttrib.h
// Created by:  drose (22Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef BUILDERATTRIB_H
#define BUILDERATTRIB_H

///////////////////////////////////////////////////////////////////
//
// BuilderAttrib, BuilderAttribI
//
// This is the parent class of both BuilderVertex and BuilderPrim, and
// contains the attribute values which may be set on either of them:
// specifically, normal, color, and pixel size. (Pixel size is the
// thickness of the lines, for a polygon or a line, or the size of the
// point, in pixels.)
//
// Like BuilderPrim and BuilderVertex, the two classes BuilderAttrib
// and BuilderAttribI are actually both instantiations of the same
// template class, BuilderAttribTempl.
//
///////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "builderAttribTempl.h"

#define BUILDERATTRIBTEMPL_BUILDERV BuilderAttribTempl<BuilderV, BuilderN, BuilderTC, BuilderC>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, BUILDERATTRIBTEMPL_BUILDERV);
#define BUILDERATTRIBTEMPL_USHORT BuilderAttribTempl<ushort, ushort, ushort, ushort>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, BUILDERATTRIBTEMPL_USHORT);

class EXPCL_PANDAEGG BuilderAttrib
  : public BuilderAttribTempl<BuilderV, BuilderN, BuilderTC, BuilderC> {
public:
  BuilderAttrib() {}

  INLINE void set_normal_value(const BuilderN *array, ushort index);
  INLINE void set_color_value(const BuilderC *array, ushort index);
};

class EXPCL_PANDAEGG BuilderAttribI
  : public BuilderAttribTempl<ushort, ushort, ushort, ushort> {
public:
  BuilderAttribI() {}

  INLINE void set_normal_value(const BuilderN *array, ushort index);
  INLINE void set_color_value(const BuilderC *array, ushort index);
};

#include "builderAttrib.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
