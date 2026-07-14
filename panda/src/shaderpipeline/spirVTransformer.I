/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformer.I
 * @author rdb
 * @date 2024-10-08
 */

/**
 * Runs the given one-shot transformation pass object.
 */
INLINE void SpirVTransformer::
run(SpirVTransformPass &&pass) {
  run(pass);
}

/**
 * Returns the id bound of the module.
 */
INLINE uint32_t SpirVTransformer::
get_id_bound() const {
  return _module.get_id_bound();
}

/**
 * Returns the module being transformed.
 */
INLINE SpirVModule &SpirVTransformer::
get_module() {
  return _module;
}

/**
 * Returns the module being transformed.
 */
INLINE const SpirVModule &SpirVTransformer::
get_module() const {
  return _module;
}
