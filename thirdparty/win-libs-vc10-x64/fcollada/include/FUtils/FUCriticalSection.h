/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUCriticalSection.h
	This file contains the FUCriticalSection class.
*/

#ifndef _FU_CRITICAL_SECTION_H_
#define _FU_CRITICAL_SECTION_H_

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

/**
	An OS dependent critical section.
	
	Currently only supported for WIN32.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUCriticalSection
{
private:
#ifdef WIN32
	CRITICAL_SECTION criticalSection; // WIN32
#elif defined (__APPLE__)
	//Do something here.
	MPCriticalRegionID criticalSection;
#elif defined (LINUX)
#else
#warning "FUCriticalSection: Critical section not implemented for other platforms."
#endif

public:
	/** Constructor. */
	FUCriticalSection();

	/** Destructor. */
	~FUCriticalSection();

	/** Enters the critical section, blocking if another thread is already in.
		Note: a thread may call Enter multiple times and it will still enter to prevent it from deadlocking itself. It
				must then call Leave the same number of times. */
	void Enter();

	/** Leaves the critical section, allowing other threads to enter. */
	void Leave();
};

#endif // _FU_CRITICAL_SECTION_H_

