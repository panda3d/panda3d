// Filename: builderVertexTempl.h
// Created by:  drose (09Sep97)
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

#ifndef BUILDERVERTEXTEMPL_H
#define BUILDERVERTEXTEMPL_H

#include "pandabase.h"

#include "builderTypes.h"
#include "builderAttribTempl.h"
#include "builder_compare.h"

#include "notify.h"
#include "pvector.h"

/////////////////////////////////////////////////////////////////////
//       Class : BuilderVertexTempl
// Description : The main body of BuilderVertex and BuilderVertexI.
//               This is a template class on each of the four
//               attribute types: vertex coordinates, normal, texture
//               coordinates, and color.  See builderVertex.h.
////////////////////////////////////////////////////////////////////
template <class VT, class NT, class TT, class CT>
class BuilderVertexTempl : public BuilderAttribTempl<VT, NT, TT, CT> {
public:
  typedef VT VType;
  typedef NT NType;
  typedef TT TType;
  typedef CT CType;

  INLINE BuilderVertexTempl();
  INLINE BuilderVertexTempl(const VType &c);
  INLINE BuilderVertexTempl(const BuilderVertexTempl &copy);
  INLINE BuilderVertexTempl &operator = (const BuilderVertexTempl &copy);

  INLINE bool is_valid() const;
  INLINE BuilderVertexTempl &clear();

  INLINE bool has_coord() const;
  INLINE VType get_coord() const;
  INLINE BuilderVertexTempl &set_coord(const VType &c);

  INLINE BuilderVertexTempl &set_normal(const NType &c);
  INLINE BuilderVertexTempl &clear_normal();

  INLINE bool has_texcoord() const;
  INLINE TType get_texcoord() const;
  INLINE BuilderVertexTempl &set_texcoord(const TType &t);
  INLINE BuilderVertexTempl &clear_texcoord();

  INLINE BuilderVertexTempl &set_color(const CType &c);
  INLINE BuilderVertexTempl &clear_color();

  INLINE BuilderVertexTempl &set_pixel_size(float s);
  INLINE BuilderVertexTempl &clear_pixel_size();

  INLINE bool operator == (const BuilderVertexTempl &other) const;
  INLINE bool operator != (const BuilderVertexTempl &other) const;
  INLINE bool operator < (const BuilderVertexTempl &other) const;
  int compare_to(const BuilderVertexTempl &other) const;

  ostream &output(ostream &out) const;

protected:
  VType _coord;
  TType _texcoord;
};

template <class VT, class NT, class TT, class CT>
INLINE ostream &operator << (ostream &out,
                             const BuilderVertexTempl<VT, NT, TT, CT> &vertex) {
  return vertex.output(out);
}


#include "builderVertexTempl.I"

#endif
