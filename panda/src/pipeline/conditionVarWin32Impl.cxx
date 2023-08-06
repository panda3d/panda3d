/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarWin32Impl.cxx
 * @author drose
 * @date 2006-02-07
 */

#include "selectThreadImpl.h"

#ifdef _WIN32

#include "conditionVarWin32Impl.h"

// This function gets replaced by PStats to measure the time spent waiting.
BOOL (__stdcall *ConditionVarWin32Impl::_wait_func)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG) = &SleepConditionVariableSRW;

#endif  // _WIN32
