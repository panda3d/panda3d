/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexColumn.h
 * @author drose
 * @date 2005-03-06
 */

#ifndef GEOMVERTEXCOLUMN_H
#define GEOMVERTEXCOLUMN_H

#include "pandabase.h"
#include "geomEnums.h"
#include "internalName.h"
#include "pointerTo.h"
#include "luse.h"

class TypedWritable;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

class GeomVertexReader;
class GeomVertexWriter;

/**
 * This defines how a single column is interleaved within a vertex array
 * stored within a Geom.  The GeomVertexArrayFormat class maintains a list of
 * these to completely define a particular array structure.
 */
class EXPCL_PANDA_GOBJ GeomVertexColumn : public GeomEnums {
private:
  INLINE GeomVertexColumn();
PUBLISHED:
  INLINE explicit GeomVertexColumn(CPT_InternalName name, int num_components,
                                   NumericType numeric_type, Contents contents,
                                   int start, int column_alignment = 0,
                                   int num_elements = 0, int element_stride = 0);
  INLINE GeomVertexColumn(const GeomVertexColumn &copy);
  void operator = (const GeomVertexColumn &copy);
  INLINE ~GeomVertexColumn();

  INLINE const InternalName *get_name() const;
  INLINE int get_num_components() const;
  INLINE int get_num_values() const;
  INLINE int get_num_elements() const;
  INLINE NumericType get_numeric_type() const;
  INLINE Contents get_contents() const;
  INLINE int get_start() const;
  INLINE int get_column_alignment() const;
  INLINE int get_element_stride() const;
  INLINE int get_component_bytes() const;
  INLINE int get_total_bytes() const;
  INLINE bool has_homogeneous_coord() const;

  INLINE bool overlaps_with(int start_byte, int num_bytes) const;
  INLINE bool is_bytewise_equivalent(const GeomVertexColumn &other) const;

  void set_name(InternalName *name);
  void set_num_components(int num_components);
  void set_numeric_type(NumericType numeric_type);
  void set_contents(Contents contents);
  void set_start(int start);
  void set_column_alignment(int column_alignment);

  void output(std::ostream &out) const;

public:
  INLINE bool is_packed_argb() const;
  INLINE bool is_uint8_rgba() const;

  INLINE int compare_to(const GeomVertexColumn &other) const;
  INLINE bool operator == (const GeomVertexColumn &other) const;
  INLINE bool operator != (const GeomVertexColumn &other) const;
  INLINE bool operator < (const GeomVertexColumn &other) const;

private:
  class Packer;

  void setup();
  Packer *make_packer() const;

public:
  void write_datagram(BamWriter *manager, Datagram &dg);
  int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  CPT_InternalName _name;
  int _num_components;
  int _num_values;
  int _num_elements;
  NumericType _numeric_type;
  Contents _contents;
  int _start;
  int _column_alignment;
  int _element_stride;
  int _component_bytes;
  int _total_bytes;
  Packer *_packer;

  // This nested class provides the implementation for packing and unpacking
  // data in a very general way, but also provides the hooks for implementing
  // the common, very direct code paths (for instance, 3-component float32 to
  // LVecBase3f) as quickly as possible.
  class Packer : public MemoryBase {
  public:
    virtual ~Packer();

    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);

    virtual double get_data1d(const unsigned char *pointer);
    virtual const LVecBase2d &get_data2d(const unsigned char *pointer);
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);
    virtual const LVecBase4d &get_data4d(const unsigned char *pointer);

    virtual int get_data1i(const unsigned char *pointer);
    virtual const LVecBase2i &get_data2i(const unsigned char *pointer);
    virtual const LVecBase3i &get_data3i(const unsigned char *pointer);
    virtual const LVecBase4i &get_data4i(const unsigned char *pointer);

    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);

    virtual void set_data1d(unsigned char *pointer, double data);
    virtual void set_data2d(unsigned char *pointer, const LVecBase2d &data);
    virtual void set_data3d(unsigned char *pointer, const LVecBase3d &data);
    virtual void set_data4d(unsigned char *pointer, const LVecBase4d &data);

    virtual void set_data1i(unsigned char *pointer, int data);
    virtual void set_data2i(unsigned char *pointer, const LVecBase2i &data);
    virtual void set_data3i(unsigned char *pointer, const LVecBase3i &data);
    virtual void set_data4i(unsigned char *pointer, const LVecBase4i &data);

    virtual const char *get_name() const {
      return "Packer";
    }

    const GeomVertexColumn *_column;
    LVecBase2f _v2;
    LVecBase3f _v3;
    LVecBase4f _v4;
    LVecBase2d _v2d;
    LVecBase3d _v3d;
    LVecBase4d _v4d;
    LVecBase2i _v2i;
    LVecBase3i _v3i;
    LVecBase4i _v4i;
    unsigned int _a, _b, _c, _d;
  };


  // This is a specialization on the generic Packer that handles points, which
  // are special because the fourth component, if not present in the data, is
  // implicitly 1.0; and if it is present, then any three-component or smaller
  // return is implicitly divided by the fourth component.
  class Packer_point : public Packer {
  public:
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);

    virtual double get_data1d(const unsigned char *pointer);
    virtual const LVecBase2d &get_data2d(const unsigned char *pointer);
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);
    virtual const LVecBase4d &get_data4d(const unsigned char *pointer);

    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);

    virtual void set_data1d(unsigned char *pointer, double data);
    virtual void set_data2d(unsigned char *pointer, const LVecBase2d &data);
    virtual void set_data3d(unsigned char *pointer, const LVecBase3d &data);
    virtual void set_data4d(unsigned char *pointer, const LVecBase4d &data);

    virtual const char *get_name() const {
      return "Packer_point";
    }
  };

  // This is similar to Packer_point, in that the fourth component is
  // implicitly 1.0 if it is not present in the data, but we never divide by
  // alpha.  It also transforms integer colors to the 0-1 range.
  class Packer_color : public Packer {
  public:
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);

    virtual double get_data1d(const unsigned char *pointer);
    virtual const LVecBase2d &get_data2d(const unsigned char *pointer);
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);
    virtual const LVecBase4d &get_data4d(const unsigned char *pointer);

    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);

    virtual void set_data1d(unsigned char *pointer, double data);
    virtual void set_data2d(unsigned char *pointer, const LVecBase2d &data);
    virtual void set_data3d(unsigned char *pointer, const LVecBase3d &data);
    virtual void set_data4d(unsigned char *pointer, const LVecBase4d &data);

    virtual const char *get_name() const {
      return "Packer_color";
    }
  };


  // These are the specializations on the generic Packer that handle the
  // direct code paths.

  class Packer_float32_3 : public Packer {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);

    virtual const char *get_name() const {
      return "Packer_float32_3";
    }
  };

  class Packer_point_float32_2 : public Packer_point {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &value);

    virtual const char *get_name() const {
      return "Packer_point_float32_2";
    }
  };

  class Packer_point_float32_3 : public Packer_point {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);

    virtual const char *get_name() const {
      return "Packer_point_float32_3";
    }
  };

  class Packer_point_float32_4 : public Packer_point {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);

    virtual const char *get_name() const {
      return "Packer_point_float32_4";
    }
  };

  class Packer_nativefloat_3 : public Packer_float32_3 {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_nativefloat_3";
    }
  };

  class Packer_point_nativefloat_2 : public Packer_point_float32_2 {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_nativefloat_2";
    }
  };

  class Packer_point_nativefloat_3 : public Packer_point_float32_3 {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_point_nativefloat_3";
    }
  };

  class Packer_point_nativefloat_4 : public Packer_point_float32_4 {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_point_nativefloat_4";
    }
  };

  class Packer_float64_3 : public Packer {
  public:
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);
    virtual void set_data3d(unsigned char *pointer, const LVecBase3d &value);

    virtual const char *get_name() const {
      return "Packer_float64_3";
    }
  };

  class Packer_point_float64_2 : public Packer_point {
  public:
    virtual const LVecBase2d &get_data2d(const unsigned char *pointer);
    virtual void set_data2d(unsigned char *pointer, const LVecBase2d &value);

    virtual const char *get_name() const {
      return "Packer_point_float64_2";
    }
  };

  class Packer_point_float64_3 : public Packer_point {
  public:
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);
    virtual void set_data3d(unsigned char *pointer, const LVecBase3d &value);

    virtual const char *get_name() const {
      return "Packer_point_float64_3";
    }
  };

  class Packer_point_float64_4 : public Packer_point {
  public:
    virtual const LVecBase4d &get_data4d(const unsigned char *pointer);
    virtual void set_data4d(unsigned char *pointer, const LVecBase4d &value);

    virtual const char *get_name() const {
      return "Packer_point_float64_4";
    }
  };

  class Packer_nativedouble_3 final : public Packer_float64_3 {
  public:
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_nativedouble_3";
    }
  };

  class Packer_point_nativedouble_2 final : public Packer_point_float64_2 {
  public:
    virtual const LVecBase2d &get_data2d(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_nativedouble_2";
    }
  };

  class Packer_point_nativedouble_3 final : public Packer_point_float64_3 {
  public:
    virtual const LVecBase3d &get_data3d(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_point_nativedouble_3";
    }
  };

  class Packer_point_nativedouble_4 : public Packer_point_float64_4 {
  public:
    virtual const LVecBase4d &get_data4d(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_point_nativedouble_4";
    }
  };

  class Packer_argb_packed final : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);

    virtual const char *get_name() const {
      return "Packer_argb_packed";
    }
  };

  class Packer_rgba_uint8_4 final : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);

    virtual const char *get_name() const {
      return "Packer_rgba_uint8_4";
    }
  };

  class Packer_rgba_float32_4 : public Packer_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);

    virtual const char *get_name() const {
      return "Packer_rgba_float32_4";
    }
  };

  class Packer_rgba_nativefloat_4 final : public Packer_rgba_float32_4 {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);

    virtual const char *get_name() const {
      return "Packer_rgba_nativefloat_4";
    }
  };

  class Packer_uint16_1 final : public Packer {
  public:
    virtual int get_data1i(const unsigned char *pointer);
    virtual void set_data1i(unsigned char *pointer, int value);

    virtual const char *get_name() const {
      return "Packer_uint16_1";
    }
  };

  friend class GeomVertexArrayFormat;
  friend class GeomVertexData;
  friend class GeomVertexReader;
  friend class GeomVertexWriter;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomVertexColumn &obj);

#include "geomVertexColumn.I"

#endif
