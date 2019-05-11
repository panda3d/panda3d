/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarImpl.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef CONDITIONVARIMPL_H
#define CONDITIONVARIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(THREAD_DUMMY_IMPL)

#include "conditionVarDummyImpl.h"
typedef ConditionVarDummyImpl ConditionVarImpl;

#elif defined(THREAD_SIMPLE_IMPL)

#include "conditionVarSimpleImpl.h"
typedef ConditionVarSimpleImpl ConditionVarImpl;

#elif defined(MUTEX_SPINLOCK)

#include "conditionVarSpinlockImpl.h"
typedef ConditionVarSpinlockImpl ConditionVarImpl;

#elif defined(THREAD_WIN32_IMPL)

#include "conditionVarWin32Impl.h"
typedef ConditionVarWin32Impl ConditionVarImpl;

#elif defined(THREAD_POSIX_IMPL)

#include "conditionVarPosixImpl.h"
typedef ConditionVarPosixImpl ConditionVarImpl;

#endif

typedef ConditionVarImpl ConditionVarFullImpl;

#if defined(WIN32_VC)
#include "conditionVarWin32Impl.h"
typedef ConditionVarWin32Impl TrueConditionVarImpl;

#elif defined(HAVE_POSIX_THREADS)
#include "conditionVarPosixImpl.h"
typedef ConditionVarPosixImpl TrueConditionVarImpl;

#else
// No true threads, sorry.  Better not try to use 'em.

#endif

#endif
