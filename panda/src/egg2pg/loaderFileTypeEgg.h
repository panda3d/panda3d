// Filename: loaderFileTypeEgg.h
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

#ifndef LOADERFILETYPEEGG_H
#define LOADERFILETYPEEGG_H

#include "pandabase.h"

#include "loaderFileType.h"

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeEgg
// Description : This defines the Loader interface to read Egg files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG LoaderFileTypeEgg : public LoaderFileType {
public:
  LoaderFileTypeEgg();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool supports_load() const;
  virtual bool supports_save() const;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &optoins,
                                  BamCacheRecord *record) const;
  virtual bool save_file(const Filename &path, const LoaderOptions &options,
                         PandaNode *node) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeEgg",
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

