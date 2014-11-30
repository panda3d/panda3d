/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUAssert.h
	This file contains a simple debugging assertion mechanism.
*/

#ifndef _FU_ASSERT_H_
#define _FU_ASSERT_H_

#ifndef _FU_FUNCTOR_H_
#include "FUtils/FUFunctor.h"
#endif // _FU_FUNCTOR_H_

namespace FUAssertion
{
    /** A functor that deals with assertions that fail. */
	typedef FUStaticFunctor1<const char*, bool> FUAssertCallback;

	/** Set a functor to be called back on assertion fail.  If no callback
		is set, the default value is to break with a message box 
		@param assertionCallback Ptr to the callback to send messages to. */
	FCOLLADA_EXPORT void SetAssertionFailedCallback(FUAssertCallback* assertionCallback);
	
	/** [INTERNAL] Event-handler for failed assertions.
        Do not use directly. We recommend you use FUBreak, FUFail or FUAssert. */
	FCOLLADA_EXPORT bool OnAssertionFailed(const char* filename, uint32 line);
};

#ifdef _DEBUG

#ifndef __FILE__
// This is unfortunately what we get on non-WIN
#define __FILE__	"NOT_SET"
#define __LINE__	0
#endif

/** Breaks into the debugger.
	In debug builds, this intentionally crashes the application. */
#define FUBreak \
	static bool ignoreAssert = false; \
	if (!ignoreAssert) { ignoreAssert = FUAssertion::OnAssertionFailed(__FILE__, __LINE__); }
#else

/** Breaks into the debugger.
	In release builds, this is an empty function. */
#define FUBreak  { FUAssertion::OnAssertionFailed(__FILE__, __LINE__); }

#endif // _DEBUG

/** Forces the debugger to break, or take the fall-back.
	@param command The fall_back command to execute. */
#define FUFail(command) { FUBreak; command; }

/** Asserts that a condition is met.
	Use this macro, instead of 'if' statements
	when you are asserting for a programmer's error.
	@param condition The condition to assert.
	@param fall_back The command to execute if the condition is not met. */
#define FUAssert(condition, fall_back) { if (!(condition)) { FUBreak; fall_back; } }

#endif // _FU_ASSERT_H_
