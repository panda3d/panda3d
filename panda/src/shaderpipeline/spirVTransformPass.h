/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformPass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVTRANSFORMPASS_H
#define SPIRVTRANSFORMPASS_H

#include "spirVModule.h"
#include "spirVBuilder.h"

/**
 * Base class for transformations applied to a SpirVModule.  SpirVTransformer
 * deduplicates unique type declarations after each pass.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVTransformPass {
public:
  using Instruction = SpirVModule::Instruction;
  using Function = SpirVModule::Function;
  using EntryPoint = SpirVModule::EntryPoint;
  using Id = SpirVId;

  virtual ~SpirVTransformPass() = default;

  virtual void run(SpirVModule &module) = 0;

  /**
   * Helper class for storing a chain of member or array accesses.  The
   * chain elements are literal member indices, not ids.
   */
  class AccessChain {
  public:
    explicit AccessChain(Id var_id) : _var_id(var_id) {}
    AccessChain(Id var_id, std::initializer_list<uint32_t> chain) : _var_id(var_id), _chain(std::move(chain)) {}

    INLINE void prepend(uint32_t id);
    INLINE void append(uint32_t id);
    INLINE void extend(const AccessChain &other);

    INLINE bool startswith(const AccessChain &other) const;
    INLINE bool operator < (const AccessChain &other) const;

    uint32_t operator [] (size_t i) const { return _chain[i]; }
    size_t size() const { return _chain.size(); }

    INLINE void output(std::ostream &out) const;

  public:
    Id _var_id;
    pvector<uint32_t> _chain;
  };

private:
  // Set by SpirVTransformer::run; a pass object may only be run once.
  bool _ran = false;

  friend class SpirVTransformer;
};

INLINE std::ostream &operator << (std::ostream &out, const SpirVTransformPass::AccessChain &obj);

#include "spirVTransformPass.I"

#endif
