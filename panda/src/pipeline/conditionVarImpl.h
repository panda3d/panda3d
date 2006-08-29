// Filename: conditionVarImpl.h
// Created by:  drose (09Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

#elif defined(THREAD_NSPR_IMPL)

#include "conditionVarNsprImpl.h"
typedef ConditionVarNsprImpl ConditionVarImpl;
typedef ConditionVarNsprImpl ConditionVarFullImpl;

#endif

#endif



