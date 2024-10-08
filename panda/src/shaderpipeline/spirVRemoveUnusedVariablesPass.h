/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVRemoveUnusedVariablesPass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVREMOVEUNUSEDVARIABLESPASS_H
#define SPIRVREMOVEUNUSEDVARIABLESPASS_H

#include "spirVTransformPass.h"

/**
 * Removes unused variables.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVRemoveUnusedVariablesPass final : public SpirVTransformPass {
public:
  virtual void preprocess();
};

#endif
