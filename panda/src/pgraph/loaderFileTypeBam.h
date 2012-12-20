// Filename: loaderFileTypeBam.h
// Created by:  drose (20Jun00)
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

#ifndef LOADERFILETYPEBAM_H
#define LOADERFILETYPEBAM_H

#include "pandabase.h"

#include "loaderFileType.h"

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeBam
// Description : This defines the Loader interface to read Bam files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH LoaderFileTypeBam : public LoaderFileType {
public:
  LoaderFileTypeBam();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool supports_load() const;
  virtual bool supports_save() const;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const;
  virtual bool save_file(const Filename &path, const LoaderOptions &options,
                         PandaNode *node) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeBam",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

