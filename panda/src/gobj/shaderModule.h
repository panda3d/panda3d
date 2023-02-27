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

#include "bamCacheRecord.h"
#include "copyOnWriteObject.h"
#include "internalName.h"
#include "shaderEnums.h"
#include "shaderType.h"

/**
 * Represents a single shader module in some intermediate representation for
 * passing to the driver.  This could contain compiled bytecode, or in some
 * cases, preprocessed source code to be given directly to the driver.
 *
 * This class inherits from CopyOnWriteObject so that modules can safely be
 * shared between multiple Shader objects, with a unique copy automatically
 * being created if the Shader needs to manipulate the module.
 */
class EXPCL_PANDA_GOBJ ShaderModule : public CopyOnWriteObject, public ShaderEnums {
PUBLISHED:
  /**
   * Defines an interface variable.
   */
  struct Variable {
  public:
    int has_location() const { return _location >= 0; }
    int get_location() const { return _location; }

  PUBLISHED:
    const ShaderType *type;
    CPT(InternalName) name;

    MAKE_PROPERTY2(location, has_location, get_location);

  public:
    int _location;
  };

  /**
   * Defines a specialization constant.
   */
  struct SpecializationConstant {
  PUBLISHED:
    const ShaderType *type;
    CPT(InternalName) name;
    uint32_t id;
  };

public:
  ShaderModule(Stage stage);
  virtual ~ShaderModule();

  INLINE Stage get_stage() const;
  INLINE uint64_t get_used_capabilities() const;

  INLINE const Filename &get_source_filename() const;
  INLINE void set_source_filename(const Filename &);

  INLINE const SpecializationConstant &get_spec_constant(size_t i) const;
  INLINE size_t get_num_spec_constants() const;

  size_t get_num_inputs() const;
  const Variable &get_input(size_t i) const;
  int find_input(CPT_InternalName name) const;

  size_t get_num_outputs() const;
  const Variable &get_output(size_t i) const;
  int find_output(CPT_InternalName name) const;

  size_t get_num_parameters() const;
  const Variable &get_parameter(size_t i) const;
  int find_parameter(CPT_InternalName name) const;

  typedef pmap<CPT_InternalName, Variable *> VariablesByName;

  virtual bool link_inputs(const ShaderModule *previous, pmap<int, int> &remap) const;
  virtual void remap_input_locations(const pmap<int, int> &remap);
  virtual void remap_parameter_locations(const pmap<int, int> &remap);

PUBLISHED:
  MAKE_PROPERTY(stage, get_stage);
  MAKE_PROPERTY(used_capabilities, get_used_capabilities);
  MAKE_SEQ_PROPERTY(inputs, get_num_inputs, get_input);
  MAKE_SEQ_PROPERTY(outputs, get_num_outputs, get_output);
  MAKE_SEQ_PROPERTY(parameters, get_num_parameters, get_parameter);
  MAKE_SEQ_PROPERTY(spec_constants, get_num_spec_constants, get_spec_constant);

  virtual std::string get_ir() const=0;

public:
  virtual void output(std::ostream &out) const;

  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  void fillin(DatagramIterator &scan, BamReader *manager) override;

protected:
  Stage _stage;
  PT(BamCacheRecord) _record;
  //std::pvector<Filename> _source_files;
  Filename _source_filename;
  //time_t _source_modified = 0;
  uint64_t _used_caps = 0;

  typedef pvector<Variable> Variables;
  Variables _inputs;
  Variables _outputs;
  Variables _parameters;

  typedef pvector<SpecializationConstant> SpecializationConstants;
  SpecializationConstants _spec_constants;

  friend class Shader;

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

INLINE std::ostream &operator << (std::ostream &out, const ShaderModule &module) {
  module.output(out);
  return out;
}

#include "shaderModule.I"

#endif
