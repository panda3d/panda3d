/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVPointer.h
 * @author rdb
 * @date 2026-07-15
 */

#ifndef SPIRVPOINTER_H
#define SPIRVPOINTER_H

#include "spirVValue.h"

/**
 * A reference to (part of) a variable in the SPIR-V function currently being
 * built: a base pointer id plus optional access chain.
 *
 * The most ergonomic way to obtain a SpirVPointer is to use the helper
 * functions on SpirVBuilder (define_uniform et al.) but it can also be
 * directly constructed from a builder and a known-valid Id.
 *
 * To access the pointer value, use explicit memory operations such as store()
 * and load() (or the atomic counterparts), which immediately emit an
 * instruction at the builder's cursor.  The value-consuming operations accept
 * a literal number, which materializes against the pointed-to type.
 *
 * Note that access chains are not eagerly emitted (unlike most SpirVValue
 * operations); a new access chain is emitted for each memory operation.  This
 * is safe because access chains are side-effect-free, but may cause some
 * (harmless) duplication.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVPointer {
public:
  using Id = SpirVId;

  SpirVPointer() = default;
  INLINE SpirVPointer(SpirVBuilder &builder, Id var_id);

  SpirVPointer(const SpirVPointer &) = default;
  SpirVPointer(SpirVPointer &&) = default;
  SpirVPointer &operator = (const SpirVPointer &) = default;
  SpirVPointer &operator = (SpirVPointer &&) = default;

  // Explicitly deleted to make it clear that this is just a rebind;
  // use store() to store a value.
  SpirVPointer &operator = (glsl::Value) = delete;

  INLINE explicit operator bool() const;

  INLINE SpirVBuilder *get_builder() const;

  SpirVPointer operator [](glsl::Value index) const;
  SpirVPointer operator [](uint32_t index) const;
  SpirVPointer operator [](Id) const = delete;

  glsl::Value load(spv::MemoryAccessMask access = spv::MemoryAccessMaskNone) const;
  void store(glsl::Argument value, spv::MemoryAccessMask access = spv::MemoryAccessMaskNone) const;

  INLINE glsl::Value atomic_add(glsl::Argument value) const;
  INLINE glsl::Value atomic_increment() const;

  void output(std::ostream &out) const;

private:
  Id emit_pointer() const;
  const ShaderType *get_pointed_type(Id pointer) const;
  glsl::Value emit_atomic(spv::Op opcode, const glsl::Argument *value) const;

  SpirVBuilder *_builder = nullptr;
  Id _base;
  pvector<Id> _chain;
};

INLINE std::ostream &operator << (std::ostream &out, const SpirVPointer &pointer);

#include "spirVPointer.I"

#endif
