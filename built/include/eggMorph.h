/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMorph.h
 * @author drose
 * @date 1999-01-29
 */

#ifndef EGGMORPH_H
#define EGGMORPH_H

#include "pandabase.h"

#include "namable.h"
#include "luse.h"
#include "pset.h"

/**
 * A single <Dxyz> or <Duv> or some such entry.  This simply contains the
 * morph name and the offset value.  The class EggMorph is actually a template
 * class on the type of value that is being offset; the specific kinds of
 * morphs are instantiated from this below.
 */
template<class Parameter>
class EggMorph : public Namable {
public:
  INLINE EggMorph(const std::string &name, const Parameter &offset);
  INLINE void set_offset(const Parameter &offset);
  INLINE const Parameter &get_offset() const;

  INLINE bool operator < (const EggMorph<Parameter> &other) const;
  INLINE bool operator == (const EggMorph<Parameter> &other) const;
  INLINE bool operator != (const EggMorph<Parameter> &other) const;

  INLINE int compare_to(const EggMorph<Parameter> &other, double threshold) const;

  INLINE void output(std::ostream &out, const std::string &tag,
                     int num_dimensions) const;

private:
  Parameter _offset;
};

// I'd love to export these, but it produces a strange linker issue with Mac
// OS X's version of GCC.  We'll do it only on Windows, then.
#ifdef _MSC_VER
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, EggMorph<LVector3d>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, EggMorph<LVector4>);
#endif

typedef EggMorph<LVector3d> EggMorphVertex;
typedef EggMorph<LVector3d> EggMorphNormal;
typedef EggMorph<LVector3d> EggMorphTexCoord;
typedef EggMorph<LVector4> EggMorphColor;

#include "eggMorph.I"

#endif
