/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pthread.h
 * @author drose
 * @date 2006-02-10
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef PTHREAD_H
#define PTHREAD_H

typedef int pthread_t;
typedef int pthread_key_t;
typedef int pthread_mutex_t;
typedef int pthread_cond_t;

#endif
