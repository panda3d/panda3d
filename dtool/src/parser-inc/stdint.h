/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stdint.h
 * @author rdb
 * @date 2010-03-29
 */

#ifndef _STDINT_H
#define _STDINT_H

#if defined(_LP64) || defined(_WIN64)
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif

using intmax_t = long long int;
using uintmax_t = unsigned long long int;

using int8_t = signed char;
using int16_t = short int;
using int32_t = int;
using int64_t = long long int;

using uint8_t = unsigned char;
using uint16_t = unsigned short int;
using uint32_t = unsigned int;
using uint64_t = unsigned long long int;

using int_least8_t = int8_t;
using int_least16_t = int16_t;
using int_least32_t = int32_t;
using int_least64_t = int64_t;

using uint_least8_t = uint8_t;
using uint_least16_t = uint16_t;
using uint_least32_t = uint32_t;
using uint_least64_t = uint64_t;

using int_fast8_t = int8_t;
using int_fast16_t = int16_t;
using int_fast32_t = int32_t;
using int_fast64_t = int64_t;

using uint_fast8_t = uint8_t;
using uint_fast16_t = uint16_t;
using uint_fast32_t = uint32_t;
using uint_fast64_t = uint64_t;

#if __WORDSIZE == 64
using intptr_t = int64_t;
using uintptr_t = uint64_t;
#else
using intptr_t = int32_t;
using uintptr_t = uint32_t;
#endif

#endif
