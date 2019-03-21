/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.I
 * @author rdb
 * @date 2019-03-21
 */

/**
 * Returns the stage of the shading pipeline that this shader module is
 * intended to fill.
 */
INLINE ShaderModule::Stage ShaderModule::
get_stage() const {
  return _stage;
}

/**
 * Returns the original filename that the module was compiled from.  This may
 * be empty.
 */
INLINE const Filename &ShaderModule::
get_source_filename() const {
  return _source_filename;
}

/**
 * Sets the source filename.
 */
INLINE void ShaderModule::
set_source_filename(const Filename &filename) {
  _source_filename = filename;
}
