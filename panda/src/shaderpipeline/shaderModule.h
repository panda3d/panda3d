/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include "typedWritableReferenceCount.h"

/**
 * This is the base class for the outputs of shaderCompilers
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderModule : public TypedWritableReferenceCount {
PUBLISHED:
  enum class Stage {
    unspecified,
    vertex,
    tess_control,
    tess_evaluation,
    geometry,
    fragment,
    compute,
  };

public:
  ShaderModule(Stage stage);
  virtual ~ShaderModule();

  INLINE Stage get_stage() const;

  INLINE const Filename &get_source_filename() const;
  INLINE void set_source_filename(const Filename &);

  static std::string format_stage(Stage stage);

PUBLISHED:
  MAKE_PROPERTY(stage, get_stage);

  virtual std::string get_ir() const=0;

private:
  Stage _stage = Stage::unspecified;
  Filename _source_filename;
  time_t _source_modified = 0;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderModule",
                  TypedWritableReferenceCount::get_class_type());
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

INLINE std::ostream &operator << (std::ostream &out, ShaderModule::Stage stage) {
  return out << ShaderModule::format_stage(stage);
}

#include "shaderModule.I"

#endif
