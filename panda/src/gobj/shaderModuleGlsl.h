/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleGlsl.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERMODULEGLSL_H
#define SHADERMODULEGLSL_H

#include "shader.h"

/**
 * ShaderModule that contains raw, preprocessed GLSL code
 */
class EXPCL_PANDA_GOBJ ShaderModuleGlsl final : public ShaderModule {
PUBLISHED:
  ShaderModuleGlsl(Stage stage, std::string code, int version = 0);
  virtual ~ShaderModuleGlsl();

public:
  virtual PT(CopyOnWriteObject) make_cow_copy() override;

  virtual std::string get_ir() const override;

  int add_included_file(Filename fn);
  Filename get_filename_from_index(int index) const;

protected:
  std::string _code;
  int _version = 0;

  static const int _fileno_offset = 2048;
  typedef pvector<Filename> Filenames;
  Filenames _included_files;

  friend class ShaderCompilerGlslPreProc;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderModule::init_type();
    register_type(_type_handle, "ShaderModuleGlsl",
                  ShaderModule::get_class_type());
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

#endif
