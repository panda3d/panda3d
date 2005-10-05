// Filename: loaderFileTypePandatool.h
// Created by:  drose (20Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LOADERFILETYPEPANDATOOL_H
#define LOADERFILETYPEPANDATOOL_H

#include "pandatoolbase.h"

#include "loaderFileType.h"

class SomethingToEggConverter;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypePandatool
// Description : This defines the Loader interface to files whose
//               converters are defined within the Pandatool package
//               and inherit from SomethingToEggConverter, like
//               FltToEggConverter and LwoToEggConverter.
////////////////////////////////////////////////////////////////////
class EXPCL_PTLOADER LoaderFileTypePandatool : public LoaderFileType {
public:
  LoaderFileTypePandatool(SomethingToEggConverter *converter);
  virtual ~LoaderFileTypePandatool();

  virtual string get_name() const;
  virtual string get_extension() const;
  virtual string get_additional_extensions() const;

  virtual void resolve_filename(Filename &path) const;
  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options) const;

private:
  SomethingToEggConverter *_converter;

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

