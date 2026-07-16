/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlDataModel.h
 * @author tkfoss
 * @date 2026-06-08
 */

#ifndef RML_DATA_MODEL_H
#define RML_DATA_MODEL_H

#include "config_rmlui.h"
#include "referenceCount.h"

#ifndef CPPPARSER
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/DataVariable.h>
#include <RmlUi/Core/Variant.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#endif

/**
 * Handle to an existing RmlUi data model.  Obtained from
 * RmlContext::create_data_model() or RmlContext::get_data_model().
 * Use dirty_variable() / dirty_all() to notify RmlUi that Python-side
 * data has changed so the DOM re-evaluates data-value / data-if / data-for.
 */
class EXPCL_PANDARMLUI RmlDataModel : public ReferenceCount {
PUBLISHED:
  bool is_valid() const;

  void dirty_variable(const std::string &name);
  void dirty_all();

#ifdef HAVE_PYTHON
  bool bind_func(const std::string &name,
                 PyObject *getter,
                 PyObject *setter = nullptr);

  // Bind a list-valued variable for use with data-for.
  // getter() must return a Python list of scalars (bool/int/float/str).
  // Call dirty_variable(name) after mutating the list to trigger DOM refresh.
  bool bind_list(const std::string &name, PyObject *getter);

  // Bind a list-of-records variable for data-for with per-field access.
  // getter() must return a Python list of dicts (field name -> scalar).  In RML,
  // data-for="row : name" then {{ row.field }} / data-if="row.field" etc.
  bool bind_dict_list(const std::string &name, PyObject *getter);

  bool bind_event_callback(const std::string &name, PyObject *callback);
#endif  // HAVE_PYTHON

public:
  RmlDataModel() = default;
  RmlDataModel(const RmlDataModel &) = delete;
  RmlDataModel &operator=(const RmlDataModel &) = delete;
#ifndef CPPPARSER
  explicit RmlDataModel(Rml::DataModelHandle handle,
                        Rml::DataModelConstructor constructor)
    : _valid(true), _handle(handle), _constructor(constructor) {}

  // Called by RmlContext when the underlying model is removed, so a retained
  // wrapper stops touching the now-destroyed RmlUi model.  Drops the custom
  // definitions (RmlUi no longer references them) and the Python callables.
  void _invalidate() {
    _valid = false;
    _custom_definitions.clear();
  }

private:
  friend class RmlContext;
  bool _valid = false;
  Rml::DataModelHandle _handle;
  Rml::DataModelConstructor _constructor;
  // Owns custom VariableDefinition objects whose pointers are stored inside
  // DataVariables registered with the model.  Must outlive the model.
  std::vector<std::unique_ptr<Rml::VariableDefinition>> _custom_definitions;
#endif
};

#endif
