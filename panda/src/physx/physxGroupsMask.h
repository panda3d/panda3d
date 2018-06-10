/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxGroupsMask.h
 * @author enn0x
 * @date 2009-10-21
 */

#ifndef PHYSXGROUPSMASK_H
#define PHYSXGROUPSMASK_H

#include "pandabase.h"

#include "physx_includes.h"

/**
 * 128-bit bitmask class.
 */
class EXPCL_PANDAPHYSX PhysxGroupsMask {

PUBLISHED:
  INLINE PhysxGroupsMask();
  INLINE PhysxGroupsMask(NxGroupsMask mask);
  INLINE ~PhysxGroupsMask();

  void set_bit(unsigned int idx);
  void clear_bit(unsigned int idx);
  bool get_bit(unsigned int idx) const;

  void output(std::ostream &out) const;

  static PhysxGroupsMask all_on();
  static PhysxGroupsMask all_off();

  INLINE unsigned int get_bits0() const;
  INLINE unsigned int get_bits1() const;
  INLINE unsigned int get_bits2() const;
  INLINE unsigned int get_bits3() const;

  INLINE void set_bits0( unsigned int bits );
  INLINE void set_bits1( unsigned int bits );
  INLINE void set_bits2( unsigned int bits );
  INLINE void set_bits3( unsigned int bits );

public:
  INLINE NxGroupsMask get_mask() const;
  INLINE void set_mask(NxGroupsMask mask);

  NxGroupsMask _mask;
};

INLINE std::ostream &operator << (std::ostream &out, const PhysxGroupsMask &mask) {
  mask.output(out);
  return out;
}

#include "physxGroupsMask.I"

#endif // PHYSXGROUPSMASK_H
