// Filename: builderAttribTempl.h
// Created by:  drose (17Sep97)
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
#ifndef BUILDERATTRIBTEMPL_H
#define BUILDERATTRIBTEMPL_H

#include "pandabase.h"

#include "builderTypes.h"

#include "pvector.h"


////////////////////////////////////////////////////////////////////
//       Class : BuilderAttribTempl
// Description : The main body of BuilderAttrib and BuilderAttribI,
//               and the base class for BuilderVertexTempl and
//               BuilderPrimTempl, this class defines the attributes
//               that may be specified for either vertices or
//               primitives.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
class BuilderAttribTempl {
public:
  typedef VT VType;
  typedef NT NType;
  typedef TT TType;
  typedef CT CType;

  INLINE BuilderAttribTempl();
  INLINE BuilderAttribTempl(const BuilderAttribTempl &copy);
  INLINE BuilderAttribTempl &operator = (const BuilderAttribTempl &copy);

  INLINE BuilderAttribTempl &clear();

  INLINE bool has_normal() const;
  INLINE NType get_normal() const;
  INLINE BuilderAttribTempl &set_normal(const NType &n);
  INLINE BuilderAttribTempl &clear_normal();

  INLINE bool has_color() const;
  INLINE CType get_color() const;
  INLINE BuilderAttribTempl &set_color(const CType &c);
  INLINE BuilderAttribTempl &clear_color();

  INLINE bool has_pixel_size() const;
  INLINE float get_pixel_size() const;
  INLINE BuilderAttribTempl &set_pixel_size(float s);
  INLINE BuilderAttribTempl &clear_pixel_size();

  INLINE bool operator == (const BuilderAttribTempl &other) const;
  INLINE bool operator != (const BuilderAttribTempl &other) const;
  INLINE bool operator < (const BuilderAttribTempl &other) const;
  int compare_to(const BuilderAttribTempl &other) const;

  ostream &output(ostream &out) const;

protected:
  NType _normal;
  CType _color;
  float _pixel_size;
  int _flags;
};

template <class VT, class NT, class TT, class CT>
INLINE ostream &operator << (ostream &out,
                             const BuilderAttribTempl<VT, NT, TT, CT> &attrib) {
  return attrib.output(out);
}

#include "builderAttribTempl.I"

#endif

