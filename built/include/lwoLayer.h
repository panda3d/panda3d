/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoLayer.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOLAYER_H
#define LWOLAYER_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"

/**
 * Signals the start of a new layer.  All the data chunks which follow will be
 * included in this layer until another layer chunk is encountered.  If data
 * is encountered before a layer chunk, it goes into an arbitrary layer.
 */
class LwoLayer : public LwoChunk {
public:
  void make_generic();

  enum Flags {
    F_hidden   = 0x0001
  };

  int _number;
  int _flags;
  LPoint3 _pivot;
  std::string _name;
  int _parent;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

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
    register_type(_type_handle, "LwoLayer",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
