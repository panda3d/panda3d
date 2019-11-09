/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonLoaderFileType.h
 * @author rdb
 * @date 2019-07-29
 */

#ifndef PYTHONLOADERFILETYPE_H
#define PYTHONLOADERFILETYPE_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "loaderFileType.h"
#include "extension.h"

class LoaderFileTypeRegistry;

/**
 * This defines a Python-based loader plug-in.  An instance of this can be
 * constructed by inheritance and explicitly registered, or it can be created
 * by passing in a pkg_resources.EntryPoint instance.
 *
 * @since 1.10.4
 */
class PythonLoaderFileType : public LoaderFileType {
public:
  PythonLoaderFileType();
  PythonLoaderFileType(std::string extension, PyObject *entry_point);
  ~PythonLoaderFileType();

  bool init(PyObject *loader);
  bool ensure_loaded() const;

  virtual std::string get_name() const override;
  virtual std::string get_extension() const override;
  virtual std::string get_additional_extensions() const override;
  virtual bool supports_compressed() const override;

  virtual bool supports_load() const override;
  virtual bool supports_save() const override;

  virtual PT(PandaNode) load_file(const Filename &path, const LoaderOptions &options,
                                  BamCacheRecord *record) const override;
  virtual bool save_file(const Filename &path, const LoaderOptions &options,
                         PandaNode *node) const override;

private:
  std::string _extension;
  std::string _additional_extensions;
  PyObject *_entry_point = nullptr;
  PyObject *_load_func = nullptr;
  PyObject *_save_func = nullptr;
  bool _supports_compressed = false;

  friend class Extension<LoaderFileTypeRegistry>;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "PythonLoaderFileType",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_PYTHON

#endif
