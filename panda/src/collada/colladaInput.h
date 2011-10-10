// Filename: colladaInput.h
// Created by:  rdb (23May11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef COLLADAINPUT_H
#define COLLADAINPUT_H

#include "config_collada.h"
#include "referenceCount.h"
#include "pvector.h"
#include "pta_LVecBase4.h"
#include "internalName.h"
#include "geomEnums.h"

class GeomPrimitive;
class GeomVertexArrayFormat;
class GeomVertexData;

#if PANDA_COLLADA_VERSION < 15
#define domInput_local domInputLocal
#define domInput_localRef domInputLocalRef
#define domInput_local_offset domInputLocalOffset
#define domInput_local_offsetRef domInputLocalOffsetRef
#endif

class domInput_local;
class domInput_local_offset;
class domP;
class domSource;

////////////////////////////////////////////////////////////////////
//       Class : ColladaInput
// Description : Class that deals with COLLADA data sources.
////////////////////////////////////////////////////////////////////
class ColladaInput : public ReferenceCount {
public:
  static ColladaInput *from_dom(domInput_local_offset &input);
  static ColladaInput *from_dom(domInput_local &input);

  int make_vertex_columns(GeomVertexArrayFormat *fmt) const;
  void write_data(GeomVertexData *vdata, int start_row, domP &p, unsigned int stride) const;

  INLINE bool is_vertex_source() const;
  INLINE unsigned int get_offset() const;

private:
  ColladaInput(const string &semantic);
  ColladaInput(const string &semantic, unsigned int set);
  bool read_data(domSource &source);
  void write_data(GeomVertexData *vdata, int start_row, domP &p, unsigned int stride, unsigned int offset) const;

  typedef pvector<PT(ColladaInput)> Inputs;
  Inputs _vertex_inputs;
  PTA_LVecBase4f _data;

  // Only filled in when appropriate.
  PT(InternalName) _column_name;
  GeomEnums::Contents _column_contents;

  unsigned int _num_bound_params;
  unsigned int _offset;
  string _semantic;
  bool _have_set;
  unsigned int _set;
};

#include "colladaInput.I"

#endif
