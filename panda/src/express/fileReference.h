/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fileReference.h
 * @author drose
 * @date 2011-06-23
 */

#ifndef FILEREFERENCE_H
#define FILEREFERENCE_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "filename.h"

/**
 * Keeps a reference-counted pointer to a file on disk.  As long as the
 * FileReference is held, someone presumably has a use for this file.
 */
class EXPCL_PANDA_EXPRESS FileReference : public TypedReferenceCount {
PUBLISHED:
  INLINE FileReference(const Filename &filename);
  INLINE const Filename &get_filename() const;

protected:
  Filename _filename;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "FileReference",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "fileReference.I"

#endif
