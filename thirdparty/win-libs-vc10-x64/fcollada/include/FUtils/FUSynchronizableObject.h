/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUSynchronizableObject.h
	This file contains the FUSynchronizableObject class.
*/

#ifndef _FU_SYNCHRONIZABLE_OBJECT_H_
#define _FU_SYNCHRONIZABLE_OBJECT_H_

#ifndef _FU_CRITICAL_SECTION_H_
#include "FUCriticalSection.h"
#endif

/**
	A synchronizable object.

	A synchronizable object is an object that has the ability to lock itself so only one thread may be using it. 
	This can be done by calling the Lock and Unlock methods of the object. Be sure to Lock and Unlock around
	all the critical sections of the object to ensure mutual exclusion.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUSynchronizableObject
{
private:
	FUCriticalSection criticalSection;

public:
	/** Destructor. */
	virtual ~FUSynchronizableObject();

	/** Locks the object, blocking if another thread has locked it. */
	virtual void Lock();

	/** Unlocks the object, allowing other threads to lock it. */
	virtual void Unlock();
};

#endif // _FU_SYNCHRONIZABLE_OBJECT_H_
