/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVUsageAnalysis.h
 * @author rdb
 * @date 2026-07-07
 */

#ifndef SPIRVUSAGEANALYSIS_H
#define SPIRVUSAGEANALYSIS_H

#include "pandabase.h"
#include "vector_uchar.h"
#include "spirVId.h"

/**
 * Contains the result of a usage analysis of a module's variables, computed as
 * a one-time snapshot by a single sweep over the function bodies.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVUsageAnalysis {
public:
  using Id = SpirVId;

  SpirVUsageAnalysis() = default;
  INLINE SpirVUsageAnalysis(uint32_t id_bound);

  INLINE Id get_origin(Id id) const;
  INLINE bool is_used(Id id) const;
  INLINE bool is_dref_sampled(Id var_id) const;
  INLINE bool is_non_dref_sampled(Id var_id) const;
  INLINE bool is_dynamically_indexed(Id var_id) const;
  INLINE bool was_size_or_levels_queried(Id var_id) const;
  INLINE bool is_sampled_image_value(Id id) const;
  INLINE bool is_constant_expression(Id id) const;

private:
  enum UsageFlags : uint8_t {
    UF_used = 1 << 0,
    UF_constant_expression = 1 << 1,
    UF_sampled_image = 1 << 2,
    UF_dref_sampled = 1 << 3,
    UF_non_dref_sampled = 1 << 4,
    UF_dynamically_indexed = 1 << 5,
    UF_queried_size_levels = 1 << 6,
  };

  INLINE bool has_flag(Id id, uint8_t flag) const;
  INLINE void set_flag(Id id, uint8_t flag);
  INLINE void set_origin(Id id, Id origin);
  INLINE void mark_used(Id id);
  INLINE void set_origin_flag(Id id, uint8_t flag);

  pvector<uint8_t> _flags;
  pvector<Id> _origins;

  friend class SpirVModule;
};

#include "spirVUsageAnalysis.I"

#endif
