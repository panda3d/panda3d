/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUSemaphore.h
	This file contains the FUSemaphore and FUBinarySemaphore classes.
*/

#ifndef _FU_SEMAPHORE_H_
#define _FU_SEMAPHORE_H_

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

/**
	An OS independent semaphore. 
	
	Currently only supported for WIN32.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUSemaphore
{
private:
#ifdef WIN32
	HANDLE semaphoreHandle; // WIN32
#elif defined (__APPLE__)
	MPTaskID semaphoreHandle;
#else
#warning "FUSemaphore: Semaphore not implemented for non Windows"
#endif

public:
	/** Constructor. 
		@param initialValue The initial value for the semaphore.
		@param maximumValue The maximum value for the semaphore. Must be higher than initialValue. */
	FUSemaphore(uint32 initialValue, uint32 maximumValue);

	/** Destructor. */
	~FUSemaphore();

	/** Increments the value of the semaphore. 
		Do not increment it above the maximum value set in the constructor. No check is made. */
	void Up();

	/** Decrements the value of the semaphore.

		The value of the semaphore never drops below 0. If the current value is 0, and this method is called,
		then this method blocks until it is possible to decrement it and then does so. This is made possible when
		another thread calls Up. 

		If more than one thread is blocked, and Up is called, any one of the waiting threads may be unblocked. */
	void Down();
};

/**
	An OS dependent binary semaphore.

	It only has the values 0, and 1.
*/
class FCOLLADA_EXPORT FUBinarySemaphore : public FUSemaphore
{
public:
	FUBinarySemaphore() : FUSemaphore(0, 1) {}
};

#endif // _FU_SEMAPHORE_H_

