/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoVertexMap.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOVERTEXMAP_H
#define LWOVERTEXMAP_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "pta_stdfloat.h"

/**
 * A mapping of floating-point values per integer index.  The meaning of these
 * values is determined by the mapping type code and/or its name.
 */
class LwoVertexMap : public LwoChunk {
public:
  bool has_value(int index) const;
  PTA_stdfloat get_value(int index) const;

  IffId _map_type;
  int _dimension;
  std::string _name;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pmap<int, PTA_stdfloat> VMap;
  VMap _vmap;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoChunk::init_type();
    register_type(_type_handle, "LwoVertexMap",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
