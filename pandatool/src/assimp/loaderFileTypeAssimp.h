// Filename: loaderFileTypeAssimp.h
// Created by:  rdb (29Mar11)
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

#ifndef LOADERFILETYPEASSIMP_H
#define LOADERFILETYPEASSIMP_H

#include "config_assimp.h"
#include "loaderFileType.h"

class AssimpLoader;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeAssimp
// Description : This defines the Loader interface that uses the
//               Assimp library to load various model formats.
////////////////////////////////////////////////////////////////////
class EXPCL_ASSIMP LoaderFileTypeAssimp : public LoaderFileType {
public:
  LoaderFileTypeAssimp();
  virtual ~LoaderFileTypeAssimp();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual string get_additional_extensions() const;
  virtual bool supports_compressed() const;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const;

public:
  AssimpLoader *_loader;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeAssimp",
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

