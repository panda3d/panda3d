/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUThread.h
	This file contains the FUThread class.
*/

#ifndef _FU_THREAD_H_
#define _FU_THREAD_H_

/**
	An OS independent thread. 
	
	Currently only supported for WIN32.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUThread
{
private:
#ifdef WIN32
	HANDLE thread;
#else
#warning "Threads not yet implemented for non Windows."
#endif

private:
	/** Constructor. */
	FUThread();

public:
	/** Destructor. */
	virtual ~FUThread();

	/** Creates a thread.
		The thread must be passed to ExitFUThread for everything to be destroyed properly.
		@param lpStartAddress The procedure to start the new thread running.
		@param lpParameter The parameter to pass to the new thread.
		@return The new OS independent thread. */
#ifdef WIN32
	static FUThread* CreateFUThread(LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter);
#else
	static FUThread* CreateFUThread(); // TODO: implement and merge with WIN32, removing the LPTHREAD_START_ROUTINE
#endif

	/** Waits for the thread to exit and clean up after it.
		@param thread The thread to exit. */
	static void ExitFUThread(FUThread* thread);

	/** Yields the current thread to other threads, giving them a chance to run. */
	static void YieldCurrentThread();

	/** Sleeps the current thread for a minimum specified duration.
		@param milliseconds The duration to sleep. */
	static void SleepCurrentThread(unsigned long milliseconds);
};

#endif // _FU_THREAD_H_
