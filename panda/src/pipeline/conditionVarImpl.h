// Filename: conditionVarImpl.h
// Created by:  drose (09Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CONDITIONVARIMPL_H
#define CONDITIONVARIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(THREAD_DUMMY_IMPL)

#include "conditionVarDummyImpl.h"
typedef ConditionVarDummyImpl ConditionVarImpl;
typedef ConditionVarDummyImpl ConditionVarFullImpl;

#elif defined(THREAD_SIMPLE_IMPL)

#include "conditionVarSimpleImpl.h"
typedef ConditionVarSimpleImpl ConditionVarImpl;
typedef ConditionVarSimpleImpl ConditionVarFullImpl;

#elif defined(MUTEX_SPINLOCK)

#include "conditionVarSpinlockImpl.h"
typedef ConditionVarSpinlockImpl ConditionVarImpl;
typedef ConditionVarSpinlockImpl ConditionVarFullImpl;

#elif defined(THREAD_WIN32_IMPL)

#include "conditionVarWin32Impl.h"
#include "conditionVarFullWin32Impl.h"
typedef ConditionVarWin32Impl ConditionVarImpl;
typedef ConditionVarFullWin32Impl ConditionVarFullImpl;

#elif defined(THREAD_POSIX_IMPL)

#include "conditionVarPosixImpl.h"
typedef ConditionVarPosixImpl ConditionVarImpl;
typedef ConditionVarPosixImpl ConditionVarFullImpl;

#endif

#endif



