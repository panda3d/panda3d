// Filename: temporaryFile.h
// Created by:  drose (23Jun11)
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

#ifndef TEMPORARYFILE_H
#define TEMPORARYFILE_H

#include "pandabase.h"

#include "fileReference.h"

////////////////////////////////////////////////////////////////////
//       Class : TemporaryFile
// Description : This is a special kind of FileReference class that
//               automatically deletes the file in question when it is
//               deleted.  It is not responsible for creating,
//               opening, or closing the file, however.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TemporaryFile : public FileReference {
PUBLISHED:
  INLINE TemporaryFile(const Filename &filename);
  virtual ~TemporaryFile();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FileReference::init_type();
    register_type(_type_handle, "TemporaryFile",
                  FileReference::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "temporaryFile.I"

#endif
