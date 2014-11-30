/*************************************************************************
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
* All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of EITHER:                                  *
*   (1) The GNU Lesser General Public License as published by the Free  *
*       Software Foundation; either version 2.1 of the License, or (at  *
*       your option) any later version. The text of the GNU Lesser      *
*       General Public License is included with this library in the     *
*       file LICENSE.TXT.                                               *
*   (2) The BSD-style license that is included with this library in     *
*       the file LICENSE-BSD.TXT.                                       *
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
*                                                                       *
*************************************************************************/

/* Library initialization/finalization functions. */

#ifndef _ODE_ODEINIT_H_
#define _ODE_ODEINIT_H_

#include <ode/common.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************ */
/* Library initialization */

/**
 * @defgroup init Library Initialization
 *
 * Library initialization functions prepare ODE internal data structures for use
 * and release allocated resources after ODE is not needed any more.
 */


/**
 * @brief Library initialization flags.
 *
 * These flags define ODE library initialization options.
 *
 * @c dInitFlagManualThreadCleanup indicates that resources allocated in TLS for threads
 * using ODE are to be cleared by library client with explicit call to @c dCleanupODEAllDataForThread.
 * If this flag is not specified the automatic resource tracking algorithm is used.
 *
 * With automatic resource tracking, On Windows, memory allocated for a thread may 
 * remain not freed for some time after the thread exits. The resources may be 
 * released when one of other threads calls @c dAllocateODEDataForThread. Ultimately,
 * the resources are released when library is closed with @c dCloseODE. On other 
 * operating systems resources are always released by the thread itself on its exit
 * or on library closure with @c dCloseODE.
 *
 * With manual thread data cleanup mode every collision space object must be 
 * explicitly switched to manual cleanup mode with @c dSpaceSetManualCleanup
 * after creation. See description of the function for more details.
 *
 * If @c dInitFlagManualThreadCleanup was not specified during initialization,
 * calls to @c dCleanupODEAllDataForThread are not allowed.
 *
 * @see dInitODE2
 * @see dAllocateODEDataForThread
 * @see dSpaceSetManualCleanup
 * @see dCloseODE
 * @ingroup init
 */
enum dInitODEFlags {
	dInitFlagManualThreadCleanup = 0x00000001 //@< Thread local data is to be cleared explicitly on @c dCleanupODEAllDataForThread function call
};

/**
 * @brief Initializes ODE library.
 *
 * @c dInitODE is obsolete. @c dInitODE2 is to be used for library initialization.
 *
 * A call to @c dInitODE is equal to the following initialization sequence
 * @code
 *     dInitODE2(0);
 *     dAllocateODEDataForThread(dAllocateMaskAll);
 * @endcode
 *
 * @see dInitODE2
 * @see dAllocateODEDataForThread
 * @ingroup init
 */
ODE_API void dInitODE(void);

/**
 * @brief Initializes ODE library.
 * @param uiInitFlags Initialization options bitmask
 * @return A nonzero if initialization succeeded and zero otherwise.
 *
 * This function must be called to initialize ODE library before first use. If 
 * initialization succeeds the function may not be called again until library is 
 * closed with a call to @c dCloseODE.
 *
 * The @a uiInitFlags parameter specifies initialization options to be used. These
 * can be combination of zero or more @c dInitODEFlags flags.
 *
 * @note
 * If @c dInitFlagManualThreadCleanup flag is used for initialization, 
 * @c dSpaceSetManualCleanup must be called to set manual cleanup mode for every
 * space object right after creation. Failure to do so may lead to resource leaks.
 *
 * @see dInitODEFlags
 * @see dCloseODE
 * @see dSpaceSetManualCleanup
 * @ingroup init
 */
ODE_API int dInitODE2(unsigned int uiInitFlags/*=0*/);


/**
 * @brief ODE data allocation flags.
 *
 * These flags are used to indicate which data is to be pre-allocated in call to
 * @c dAllocateODEDataForThread.
 *
 * @c dAllocateFlagBasicData tells to allocate the basic data set required for
 * normal library operation. This flag is equal to zero and is always implicitly 
 * included.
 *
 * @c dAllocateFlagCollisionData tells that collision detection data is to be allocated.
 * Collision detection functions may not be called if the data has not be allocated 
 * in advance. If collision detection is not going to be used, it is not necessary
 * to specify this flag.
 *
 * @c dAllocateMaskAll is a mask that can be used for for allocating all possible 
 * data in cases when it is not known what exactly features of ODE will be used.
 * The mask may not be used in combination with other flags. It is guaranteed to
 * include all the current and future legal allocation flags. However, mature 
 * applications should use explicit flags they need rather than allocating everything.
 *
 * @see dAllocateODEDataForThread
 * @ingroup init
 */
enum dAllocateODEDataFlags {
	dAllocateFlagBasicData = 0, //@< Allocate basic data required for library to operate

	dAllocateFlagCollisionData = 0x00000001, //@< Allocate data for collision detection

	dAllocateMaskAll = ~0U //@< Allocate all the possible data that is currently defined or will be defined in the future.
};

/**
 * @brief Allocate thread local data to allow the thread calling ODE.
 * @param uiAllocateFlags Allocation options bitmask.
 * @return A nonzero if allocation succeeded and zero otherwise.
 * 
 * The function is required to be called for every thread that is going to use
 * ODE. This function allocates the data that is required for accessing ODE from 
 * current thread along with optional data required for particular ODE subsystems.
 *
 * @a uiAllocateFlags parameter can contain zero or more flags from @c dAllocateODEDataFlags
 * enumerated type. Multiple calls with different allocation flags are allowed.
 * The flags that are already allocated are ignored in subsequent calls. If zero
 * is passed as the parameter, it means to only allocate the set of most important
 * data the library can not operate without.
 *
 * If the function returns failure status it means that none of the requested 
 * data has been allocated. The client may retry allocation attempt with the same 
 * flags when more system resources are available.
 *
 * @see dAllocateODEDataFlags
 * @see dCleanupODEAllDataForThread
 * @ingroup init
 */
ODE_API int dAllocateODEDataForThread(unsigned int uiAllocateFlags);

/**
 * @brief Free thread local data that was allocated for current thread.
 *
 * If library was initialized with @c dInitFlagManualThreadCleanup flag the function 
 * is required to be called on exit of every thread that was calling @c dAllocateODEDataForThread.
 * Failure to call @c dCleanupODEAllDataForThread may result in some resources remaining 
 * not freed until program exit. The function may also be called when ODE is still 
 * being used to release resources allocated for all the current subsystems and 
 * possibly proceed with data pre-allocation for other subsystems.
 *
 * The function can safely be called several times in a row. The function can be 
 * called without prior invocation of @c dAllocateODEDataForThread. The function 
 * may not be called before ODE is initialized with @c dInitODE2 or after library 
 * has been closed with @c dCloseODE. A call to @c dCloseODE implicitly releases 
 * all the thread local resources that might be allocated for all the threads that 
 * were using ODE.
 *
 * If library was initialized without @c dInitFlagManualThreadCleanup flag 
 * @c dCleanupODEAllDataForThread must not be called.
 *
 * @see dAllocateODEDataForThread
 * @see dInitODE2
 * @see dCloseODE
 * @ingroup init
 */
ODE_API void dCleanupODEAllDataForThread();


/**
 * @brief Close ODE after it is not needed any more.
 *
 * The function is required to be called when program does not need ODE features any more.
 * The call to @c dCloseODE releases all the resources allocated for library
 * including all the thread local data that might be allocated for all the threads
 * that were using ODE.
 *
 * @c dCloseODE is a paired function for @c dInitODE2 and must only be called
 * after successful library initialization.
 *
 * @note Important!
 * Make sure that all the threads that were using ODE have already terminated 
 * before calling @c dCloseODE. In particular it is not allowed to call
 * @c dCleanupODEAllDataForThread after @c dCloseODE.
 *
 * @see dInitODE2
 * @see dCleanupODEAllDataForThread
 * @ingroup init
 */
ODE_API void dCloseODE(void);



#ifdef __cplusplus
} // extern "C"
#endif


#endif // _ODE_ODEINIT_H_
