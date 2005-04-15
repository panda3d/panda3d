// Filename: qpgeomVertexColumn.h
// Created by:  drose (06Mar05)
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

#ifndef qpGEOMVERTEXCOLUMN_H
#define qpGEOMVERTEXCOLUMN_H

#include "pandabase.h"
#include "qpgeomEnums.h"
#include "internalName.h"
#include "pointerTo.h"
#include "luse.h"

class TypedWritable;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

class qpGeomVertexReader;
class qpGeomVertexWriter;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexColumn
// Description : This defines how a single column is interleaved
//               within a vertex array stored within a Geom.  The
//               GeomVertexArrayFormat class maintains a list of these
//               to completely define a particular array structure.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexColumn : public qpGeomEnums {
PUBLISHED:
private:
  INLINE qpGeomVertexColumn();
PUBLISHED:
  INLINE qpGeomVertexColumn(const InternalName *name, int num_components,
                            NumericType numeric_type, Contents contents,
                            int start);
  INLINE qpGeomVertexColumn(const qpGeomVertexColumn &copy);
  void operator = (const qpGeomVertexColumn &copy);
  INLINE ~qpGeomVertexColumn();

  INLINE const InternalName *get_name() const;
  INLINE int get_num_components() const;
  INLINE int get_num_values() const;
  INLINE NumericType get_numeric_type() const;
  INLINE Contents get_contents() const;
  INLINE int get_start() const;
  INLINE int get_component_bytes() const;
  INLINE int get_total_bytes() const;
  INLINE bool has_homogeneous_coord() const;

  INLINE bool overlaps_with(int start_byte, int num_bytes) const;
  INLINE bool is_bytewise_equivalent(const qpGeomVertexColumn &other) const;

  void output(ostream &out) const;

public:
  INLINE bool is_packed_argb() const;
  INLINE bool is_uint8_rgba() const;

  INLINE int compare_to(const qpGeomVertexColumn &other) const;
  INLINE bool operator == (const qpGeomVertexColumn &other) const;
  INLINE bool operator != (const qpGeomVertexColumn &other) const;
  INLINE bool operator < (const qpGeomVertexColumn &other) const;

private:
  class Packer;

  void setup();
  Packer *make_packer() const;

public:
  void write_datagram(BamWriter *manager, Datagram &dg);
  int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  CPT(InternalName) _name;
  int _num_components;
  int _num_values;
  NumericType _numeric_type;
  Contents _contents;
  int _start;
  int _component_bytes;
  int _total_bytes;
  Packer *_packer;

  // This nested class provides the implementation for packing and
  // unpacking data in a very general way, but also provides the hooks
  // for implementing the common, very direct code paths (for
  // instance, 3-component float32 to LVecBase3f) as quickly as
  // possible.
  class Packer {
  public:
    virtual ~Packer();
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual int get_data1i(const unsigned char *pointer);
    virtual const int *get_data2i(const unsigned char *pointer);
    virtual const int *get_data3i(const unsigned char *pointer);
    virtual const int *get_data4i(const unsigned char *pointer);

    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);
    
    virtual void set_data1i(unsigned char *pointer, int a);
    virtual void set_data2i(unsigned char *pointer, int a, int b);
    virtual void set_data3i(unsigned char *pointer, int a, int b, int c);
    virtual void set_data4i(unsigned char *pointer, int a, int b, int c, int d);

    INLINE float maybe_scale_color(unsigned int value);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b,
                                  unsigned int c);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b,
                                  unsigned int c, unsigned int d);

    INLINE unsigned int maybe_unscale_color(float data);
    INLINE void maybe_unscale_color(const LVecBase2f &data);
    INLINE void maybe_unscale_color(const LVecBase3f &data);
    INLINE void maybe_unscale_color(const LVecBase4f &data);

    const qpGeomVertexColumn *_column;
    LVecBase2f _v2;
    LVecBase3f _v3;
    LVecBase4f _v4;
    int _i[4];
    unsigned int _a, _b, _c, _d;
  };


  // This is a specialization on the generic Packer that handles
  // points, which are special because the fourth component, if not
  // present in the data, is implicitly 1.0; and if it is present,
  // than any three-component or smaller return is implicitly divided
  // by the fourth component.
  class Packer_point : public Packer {
  public:
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);
  };

  // This is similar to Packer_point, in that the fourth component is
  // implicitly 1.0 if it is not present in the data, but we never
  // divide by alpha.
  class Packer_color : public Packer {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
  };


  // These are the specializations on the generic Packer that handle
  // the direct code paths.

  class Packer_float32_3 : public Packer {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);
  };

  class Packer_point_float32_2 : public Packer_point {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &value);
  };

  class Packer_point_float32_3 : public Packer_point {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);
  };

  class Packer_point_float32_4 : public Packer_point {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Packer_nativefloat_3 : public Packer_float32_3 {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Packer_point_nativefloat_2 : public Packer_point_float32_2 {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
  };

  class Packer_point_nativefloat_3 : public Packer_point_float32_3 {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Packer_point_nativefloat_4 : public Packer_point_float32_4 {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Packer_argb_packed : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Packer_rgba_uint8_4 : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Packer_rgba_float32_4 : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Packer_rgba_nativefloat_4 : public Packer_rgba_float32_4 {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Packer_uint16_1 : public Packer {
  public:
    virtual int get_data1i(const unsigned char *pointer);
    virtual void set_data1i(unsigned char *pointer, int value);
  };

  friend class qpGeomVertexArrayFormat;
  friend class qpGeomVertexReader;
  friend class qpGeomVertexWriter;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexColumn &obj);

#include "qpgeomVertexColumn.I"

#endif
