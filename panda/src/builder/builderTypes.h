// Filename: builderTypes.h
// Created by:  drose (11Sep97)
//
////////////////////////////////////////////////////////////////////
#ifndef BUILDERTYPES_H
#define BUILDERTYPES_H

#include <pandabase.h>

#include <luse.h>
#include <typedef.h>

#ifndef WIN32_VC
#include <stl_config.h>
#endif

static const float nearly_zero = 0.0001;

// The BuilderVec classes are a series of proxies around Vertexf,
// Normalf, TexCoordf, and Colorf.  They're useful for building
// collections of these vertex values, and provide handy things like
// (almost) equivalence operators and sorting operators.

// The BuilderVec's each have a special constructor with a single int.
// These constructors create an instance of the vector with all values
// initialized to zero.  This is a cheat to create a uniform way to create
// a zero-valued VType, CType, or TType without knowing whether the type
// is indexed (a ushort) or nonindexed (a BuilderVec).

class EXPCL_PANDAEGG BuilderTC {
public:
  BuilderTC() {}
  BuilderTC(int) : _v(0.0, 0.0) {}
  BuilderTC(const TexCoordf &v) : _v(v) {}
  BuilderTC(const TexCoordd &v) : _v(v[0], v[1]) {}
  BuilderTC(const BuilderTC &copy) : _v(copy._v) {}

  operator TexCoordf & () {
    return _v;
  }

  operator const TexCoordf & () const {
    return _v;
  }

  float operator [] (int n) const { return _v[n]; }
  float &operator [] (int n) { return _v[n]; }

  BuilderTC &operator = (const BuilderTC &copy) {
    _v = copy._v;
    return *this;
  }
  bool operator == (const BuilderTC &other) const {
    return _v.almost_equal(other._v, nearly_zero);
  }
  bool operator != (const BuilderTC &other) const {
    return !operator == (other);
  }

  // The < operator is simply for ordering vectors in a sorted
  // container; it has no useful mathematical meaning.
  bool operator < (const BuilderTC &other) const {
    return (_v.compare_to(other._v) < 0);
  }
  TexCoordf _v;
};

class EXPCL_PANDAEGG BuilderV {
public:
  BuilderV() {}
  BuilderV(int) : _v(0.0, 0.0, 0.0) {}
  BuilderV(const Vertexf &v) : _v(v) {}
  BuilderV(const Vertexd &v) : _v(v[0], v[1], v[2]) {}
  BuilderV(const BuilderV &copy) : _v(copy._v) {}

  operator Vertexf & () {
    return _v;
  }

  operator const Vertexf & () const {
    return _v;
  }

  float operator [] (int n) const { return _v[n]; }
  float &operator [] (int n) { return _v[n]; }

  BuilderV &operator = (const BuilderV &copy) {
    _v = copy._v;
    return *this;
  }
  bool operator == (const BuilderV &other) const {
    return _v.almost_equal(other._v, nearly_zero);
  }
  bool operator != (const BuilderV &other) const {
    return !operator == (other);
  }
  bool operator < (const BuilderV &other) const {
    return (_v.compare_to(other._v) < 0);
  }
  Vertexf _v;
};

class EXPCL_PANDAEGG BuilderN {
public:
  BuilderN() {}
  BuilderN(int) : _v(0.0, 0.0, 0.0) {}
  BuilderN(const Normalf &v) : _v(v) {}
  BuilderN(const Normald &v) : _v(v[0], v[1], v[2]) {}
  BuilderN(const BuilderN &copy) : _v(copy._v) {}

  operator Normalf & () {
    return _v;
  }

  operator const Normalf & () const {
    return _v;
  }

  float operator [] (int n) const { return _v[n]; }
  float &operator [] (int n) { return _v[n]; }

  BuilderN &operator = (const BuilderN &copy) {
    _v = copy._v;
    return *this;
  }
  bool operator == (const BuilderN &other) const {
    return _v.almost_equal(other._v, nearly_zero);
  }
  bool operator != (const BuilderN &other) const {
    return !operator == (other);
  }
  bool operator < (const BuilderN &other) const {
    return (_v.compare_to(other._v) < 0);
  }
  Normalf _v;
};

class EXPCL_PANDAEGG BuilderC {
public:
  BuilderC() {}
  BuilderC(int) : _v(0.0, 0.0, 0.0, 0.0) {}
  BuilderC(const Colorf &v) : _v(v) {}
  BuilderC(const Colord &v) : _v(v[0], v[1], v[2], v[3]) {}
  BuilderC(const BuilderC &copy) : _v(copy._v) {}

  operator Colorf & () {
    return _v;
  }

  operator const Colorf & () const {
    return _v;
  }

  float operator [] (int n) const { return _v[n]; }
  float &operator [] (int n) { return _v[n]; }

  BuilderC &operator = (const BuilderC &copy) {
    _v = copy._v;
    return *this;
  }
  bool operator == (const BuilderC &other) const {
    return _v.almost_equal(other._v, nearly_zero);
  }
  bool operator != (const BuilderC &other) const {
    return !(*this == other);
  }
  bool operator < (const BuilderC &other) const {
    return (_v.compare_to(other._v) < 0);
  }
  Colorf _v;
};

INLINE ostream &operator << (ostream &out, const BuilderTC &v) {
  return out << "(" << v[0] << " " << v[1] << ")";
}

INLINE ostream &operator << (ostream &out, const BuilderV &v) {
  return out << "(" << v[0] << " " << v[1] << " " << v[2] << ")";
}

INLINE ostream &operator << (ostream &out, const BuilderN &v) {
  return out << "(" << v[0] << " " << v[1] << " " << v[2] << ")";
}

INLINE ostream &operator << (ostream &out, const BuilderC &v) {
  return out << "(" << v[0] << " " << v[1] << " " << v[2] << " "
	     << v[3] << ")";
}

enum BuilderAttribFlags {
  BAF_coord                  = 0x00001,
  BAF_normal                 = 0x00002,
  BAF_texcoord               = 0x00004,
  BAF_color                  = 0x00008,
  BAF_pixel_size             = 0x00010,

  BAF_overall_updated        = 0x00100,
  BAF_overall_normal         = 0x00200,
  BAF_overall_color          = 0x00400,
  BAF_overall_pixel_size     = 0x00800,

  BAF_vertex_normal          = 0x01000,
  BAF_vertex_texcoord        = 0x02000,
  BAF_vertex_color           = 0x04000,
  BAF_vertex_pixel_size      = 0x08000,

  BAF_component_normal       = 0x10000,
  BAF_component_color        = 0x20000,
  BAF_component_pixel_size   = 0x04000,
};

ostream &operator << (ostream &out, BuilderAttribFlags baf);

enum BuilderPrimType {
  BPT_poly,
  BPT_point,
  BPT_line,

  // The following types are generated internally by the builder and
  // mesher.  Normally they will not be seen by the user.
  BPT_tri,
  BPT_tristrip,
  BPT_trifan,
  BPT_quad,
  BPT_quadstrip,
};

ostream &operator << (ostream &out, BuilderPrimType bpt);

#endif
