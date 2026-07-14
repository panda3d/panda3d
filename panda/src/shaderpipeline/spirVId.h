/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVId.h
 * @author rdb
 * @date 2026-07-07
 */

#ifndef SPIRVID_H
#define SPIRVID_H

#include <stdint.h>
#include <iostream>

/**
 * A SPIR-V result id.  This is just a wrapper around a uint32_t but exists for
 * type safety.
 */
class SpirVId {
public:
  SpirVId() = default;
  explicit SpirVId(uint32_t word) : _word(word) {}

  operator uint32_t() const { return _word; }
  explicit operator bool() const { return _word != 0; }

private:
  uint32_t _word = 0;
};

inline std::ostream &
operator << (std::ostream &out, SpirVId id) {
  return out << (uint32_t)id;
}

#endif
