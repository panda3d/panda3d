/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVDebugOutputPass.I
 * @author rdb
 * @date 2026-02-28
 */

/**
 *
 */
INLINE SpirVDebugOutputPass::ExtInstImport SpirVDebugOutputPass::
get_ext_inst_import(uint32_t id) const {
  auto it = _ext_inst_imports.find(id);
  if (it != _ext_inst_imports.end()) {
    return it->second;
  } else {
    return EI_other;
  }
}

/**
 *
 */
INLINE SpirVId SpirVDebugOutputPass::
get_uint_constant(SpirVModule &module, uint32_t v) {
  if (v >= _numbers.size()) {
    _numbers.resize(v + 1, Id());
  }
  if (_numbers[v] == 0u) {
    _numbers[v] = module.define_constant(ShaderType::UINT, v);
  }
  return _numbers[v];
}
