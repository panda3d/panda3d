// Filename: loaderFileTypePandatool.h
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

#ifndef LOADERFILETYPEPANDATOOL_H
#define LOADERFILETYPEPANDATOOL_H

#include "pandatoolbase.h"

#include "loaderFileType.h"

class SomethingToEggConverter;
class EggToSomethingConverter;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypePandatool
// Description : This defines the Loader interface to files whose
//               converters are defined within the Pandatool package
//               and inherit from SomethingToEggConverter, like
//               FltToEggConverter and LwoToEggConverter.
////////////////////////////////////////////////////////////////////
class EXPCL_PTLOADER LoaderFileTypePandatool : public LoaderFileType {
public:
  LoaderFileTypePandatool(SomethingToEggConverter *loader,
                          EggToSomethingConverter *saver = NULL);
  virtual ~LoaderFileTypePandatool();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual string get_additional_extensions() const;
  virtual bool supports_compressed() const;

  virtual bool supports_load() const;
  virtual bool supports_save() const;

  virtual void resolve_filename(Filename &path) const;
  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const;
  virtual bool save_file(const Filename &path, const LoaderOptions &options,
                         PandaNode *node) const;

private:
  SomethingToEggConverter *_loader;
  EggToSomethingConverter *_saver;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypePandatool",
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

