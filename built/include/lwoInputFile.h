/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoInputFile.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOINPUTFILE_H
#define LWOINPUTFILE_H

#include "pandatoolbase.h"

#include "iffInputFile.h"

#include "luse.h"

/**
 * A specialization of IffInputFile to handle reading a Lightwave Object file.
 */
class LwoInputFile : public IffInputFile {
public:
  LwoInputFile();
  ~LwoInputFile();

  INLINE double get_lwo_version() const;
  INLINE void set_lwo_version(double version);

  int get_vx();
  LVecBase3 get_vec3();
  Filename get_filename();

protected:
  virtual IffChunk *make_new_chunk(IffId id);

private:
  double _lwo_version;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IffInputFile::init_type();
    register_type(_type_handle, "LwoInputFile",
                  IffInputFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lwoInputFile.I"

#endif
