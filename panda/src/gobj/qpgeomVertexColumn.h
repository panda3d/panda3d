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
#include "internalName.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexColumn
// Description : This defines how a single column is interleaved
//               within a vertex array stored within a Geom.  The
//               GeomVertexArrayFormat class maintains a list of these
//               to completely define a particular array structure.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexColumn {
PUBLISHED:
  enum NumericType {
    NT_uint8,        // An integer 0..255
    NT_uint16,       // An integer 0..65535
    NT_packed_dcba,  // DirectX style, four byte values packed in a uint32
    NT_packed_dabc,  // DirectX packed color order (ARGB)
    NT_float32,      // A floating-point number
  };

  enum Contents {
    C_other,        // Arbitrary meaning, leave it alone
    C_point,        // A point in 3-space or 4-space
    C_vector,       // A surface normal, tangent, or binormal
    C_texcoord,     // A texture coordinate
    C_color,        // 3- or 4-component color, ordered R, G, B, [A]
    C_index,        // An index value into some other table
    C_morph_delta,  // A delta from some base value, defining a blend shape
  };

  INLINE qpGeomVertexColumn(const InternalName *name, int num_components,
                              NumericType numeric_type, Contents contents,
                              int start);
  INLINE qpGeomVertexColumn(const qpGeomVertexColumn &copy);
  INLINE void operator = (const qpGeomVertexColumn &copy);

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
  CPT(InternalName) _name;
  int _num_components;
  int _num_values;
  NumericType _numeric_type;
  Contents _contents;
  int _start;
  int _component_bytes;
  int _total_bytes;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexColumn &obj);

#include "qpgeomVertexColumn.I"

#endif
