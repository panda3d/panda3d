/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeSrt.h
 * @author drose
 * @date 2010-10-06
 */

#ifndef LOADERFILETYPESRT_H
#define LOADERFILETYPESRT_H

#include "pandabase.h"

#include "loaderFileType.h"

/**
 * This defines the Loader interface to read SpeedTree SRT files, which
 * describe a single tree.  It actually returns a SpeedTreeNode with just a
 * single tree within it.
 */
class EXPCL_PANDASPEEDTREE LoaderFileTypeSrt : public LoaderFileType {
public:
  LoaderFileTypeSrt();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypeSrt",
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
