/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pbitops.I
 * @author drose
 * @date 2008-05-10
 */

/**
 * Returns the number of 1 bits in the indicated word.
 */
INLINE int
count_bits_in_word(uint16_t x) {
  return (int)num_bits_on[x];
}

/**
 * Returns the number of 1 bits in the indicated word.
 */
INLINE int
count_bits_in_word(uint32_t x) {
#if defined(__GNUC__) && defined(__POPCNT__)
  return __builtin_popcount((unsigned int) x);
#else
  return (int)num_bits_on[x & 0xffff] + (int)num_bits_on[(x >> 16) & 0xffff];
#endif
}

/**
 * Returns the number of 1 bits in the indicated word.
 */
INLINE int
count_bits_in_word(uint64_t x) {
#if defined(__GNUC__) && defined(__POPCNT__)
  return __builtin_popcountll((unsigned long long) x);
#else
  return count_bits_in_word((uint32_t)x) + count_bits_in_word((uint32_t)(x >> 32));
#endif
}

/**
 * Returns a value such that every bit at or below the highest bit in x is 1.
 */
INLINE uint16_t
flood_bits_down(uint16_t x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  return x;
}

/**
 * Returns a value such that every bit at or below the highest bit in x is 1.
 */
INLINE uint32_t
flood_bits_down(uint32_t x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  return x;
}

/**
 * Returns a value such that every bit at or below the highest bit in x is 1.
 */
INLINE uint64_t
flood_bits_down(uint64_t x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);
  return x;
}

/**
 * Returns a value such that every bit at or above the highest bit in x is 1.
 */
INLINE uint16_t
flood_bits_up(uint16_t x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
  return x;
}

/**
 * Returns a value such that every bit at or above the highest bit in x is 1.
 */
INLINE uint32_t
flood_bits_up(uint32_t x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
  x |= (x << 16);
  return x;
}

/**
 * Returns a value such that every bit at or above the highest bit in x is 1.
 */
INLINE uint64_t
flood_bits_up(uint64_t x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
  x |= (x << 16);
  x |= (x << 32);
  return x;
}

/**
 * Returns the index of the lowest 1 bit in the word.  Returns -1 if there are
 * no 1 bits.
 */
INLINE int
get_lowest_on_bit(uint16_t x) {
#if defined(_MSC_VER)
  unsigned long result;
  return (_BitScanForward(&result, (unsigned long) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return __builtin_ffs((int) x) - 1;
#else
  if (x == 0) {
    return -1;
  }

  uint16_t w = (x & (~x + 1));
  return (int)num_bits_on[w - 1];
#endif
}

/**
 * Returns the index of the lowest 1 bit in the word.  Returns -1 if there are
 * no 1 bits.
 */
INLINE int
get_lowest_on_bit(uint32_t x) {
#if defined(_MSC_VER)
  unsigned long result;
  return (_BitScanForward(&result, (unsigned long) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return __builtin_ffs((int) x) - 1;
#else
  if (x == 0) {
    return -1;
  }

  uint32_t w = (x & (~x + 1));
  return count_bits_in_word(w - 1);
#endif
}

/**
 * Returns the index of the lowest 1 bit in the word.  Returns -1 if there are
 * no 1 bits.
 */
INLINE int
get_lowest_on_bit(uint64_t x) {
#if defined(_MSC_VER) && defined(_M_X64)
  unsigned long result;
  return (_BitScanForward64(&result, (unsigned __int64) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return __builtin_ffsll((long long) x) - 1;
#else
  if (x == 0) {
    return -1;
  }

  uint64_t w = (x & (~x + 1));
  return count_bits_in_word(w - 1);
#endif
}

/**
 * Returns the index of the highest 1 bit in the word.  Returns -1 if there
 * are no 1 bits.
 */
INLINE int
get_highest_on_bit(uint16_t x) {
#if defined(_MSC_VER)
  unsigned long result;
  return (_BitScanReverse(&result, (unsigned long) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return (x == 0) ? -1 : 31 - __builtin_clz((unsigned int) x);
#else
  uint16_t w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
#endif
}

/**
 * Returns the index of the highest 1 bit in the word.  Returns -1 if there
 * are no 1 bits.
 */
INLINE int
get_highest_on_bit(uint32_t x) {
#if defined(_MSC_VER)
  unsigned long result;
  return (_BitScanReverse(&result, (unsigned long) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return (x == 0) ? -1 : 31 - __builtin_clz((unsigned int) x);
#else
  uint32_t w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
#endif
}

/**
 * Returns the index of the highest 1 bit in the word.  Returns -1 if there
 * are no 1 bits.
 */
INLINE int
get_highest_on_bit(uint64_t x) {
#if defined(_MSC_VER) && defined(_M_X64)
  unsigned long result;
  return (_BitScanReverse64(&result, (unsigned __int64) x) == 0) ? -1 : result;
#elif defined(__GNUC__)
  return (x == 0) ? -1 : 63 - __builtin_clzll((unsigned long long) x);
#else
  uint64_t w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
#endif
}

/**
 * Returns the smallest power of 2 greater than x.
 *
 * Returns the smallest number n such that (1 << n) is larger than x.
 */
INLINE int
get_next_higher_bit(uint16_t x) {
  return get_highest_on_bit(x) + 1;
}

/**
 * Returns the smallest power of 2 greater than x.
 *
 * Returns the smallest number n such that (1 << n) is larger than x.
 */
INLINE int
get_next_higher_bit(uint32_t x) {
  return get_highest_on_bit(x) + 1;
}

/**
 * Returns the smallest power of 2 greater than x.
 *
 * Returns the smallest number n such that (1 << n) is larger than x.
 */
INLINE int
get_next_higher_bit(uint64_t x) {
  return get_highest_on_bit(x) + 1;
}
