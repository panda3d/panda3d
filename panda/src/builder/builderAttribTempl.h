// Filename: builderAttribTempl.h
// Created by:  drose (17Sep97)
//
////////////////////////////////////////////////////////////////////
#ifndef BUILDERATTRIBTEMPL_H
#define BUILDERATTRIBTEMPL_H

#include <pandabase.h>

#include "builderTypes.h"

#include <vector>


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

  INLINE bool has_color() const;
  INLINE CType get_color() const;
  INLINE BuilderAttribTempl &set_color(const CType &c);

  INLINE bool has_pixel_size() const;
  INLINE float get_pixel_size() const;
  INLINE BuilderAttribTempl &set_pixel_size(float s);

  INLINE bool operator == (const BuilderAttribTempl &other) const;
  INLINE bool operator < (const BuilderAttribTempl &other) const;

  INLINE ostream &output(ostream &out) const;

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

