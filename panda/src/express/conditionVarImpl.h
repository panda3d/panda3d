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

#elif defined(THREAD_NSPR_IMPL)

#include "conditionVarNsprImpl.h"
typedef ConditionVarNsprImpl ConditionVarImpl;

#endif

#endif



