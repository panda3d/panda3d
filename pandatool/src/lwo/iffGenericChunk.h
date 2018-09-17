/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffGenericChunk.h
 * @author drose
 * @date 2001-04-23
 */

#ifndef IFFGENERICCHUNK_H
#define IFFGENERICCHUNK_H

#include "pandatoolbase.h"

#include "iffChunk.h"

#include "datagram.h"


/**
 * A class for a generic kind of IffChunk that is not understood by a
 * particular IffReader.  It remembers its entire contents.
 */
class IffGenericChunk : public IffChunk {
public:
  INLINE IffGenericChunk();

  INLINE const Datagram &get_data() const;
  INLINE void set_data(const Datagram &data);

  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  Datagram _data;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IffChunk::init_type();
    register_type(_type_handle, "IffGenericChunk",
                  IffChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "iffGenericChunk.I"

#endif
