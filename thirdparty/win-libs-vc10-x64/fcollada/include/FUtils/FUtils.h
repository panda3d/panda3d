/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUtils.h
	Includes the common utilities classes and macros.
*/

/** @defgroup FUtils Utility Classes. */

// [January 2007] PLUG_CRT and the CRT memory debugger is not used directly, anymore, at Feeling Software.
// Instead, we rely on the Visual Leak Detector: http://www.codeproject.com/tools/visualleakdetector.asp

#ifndef _F_UTILS_H_
#define _F_UTILS_H_

// CRT
#ifdef _WIN32
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1 // MSVS 2005 support.
#endif // _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4702)
#endif // _WIN32

#define NOMINMAX /**< Rid us of the default versions of MINMAX. */
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

#ifndef FCOLLADA_NOMINMAX
/** Retrieves the largest of two values. */
template<class T>
inline T& max(T& x, T& y) { return (x > y) ? x : y; }
template<class T>
inline const T& max(const T& x, const T& y) { return (x > y) ? x : y; } /**< See above. */

/** Retrieves the smallest of two values. */
template <class T>
inline T& min(T& x, T& y) { return (x < y) ? x : y; }
template <class T>
inline const T& min(const T& x,const T& y) { return (x < y) ? x : y; } /**< See above. */
#endif // FCOLLADA_NOMINMAX

#ifdef _WIN32
#pragma warning(default:4702)
#endif

// Conversion macros
#define UNUSED(a) /**< Removes a piece of code during the pre-process. This macro is useful for these pesky unused variable warnings. */
#ifdef _DEBUG
#define UNUSED_NDEBUG(a) a
#else
#define UNUSED_NDEBUG(a) /**< Removes a piece of debug code during the pre-process. This macro is useful for these pesky unused variable warnings. */
#endif // _DEBUG

// Pre-include the platform-specific macros and definitions
#ifndef _FU_PLATFORMS_H_
#include "FUtils/Platforms.h"
#endif // _FU_PLATFORMS_H_
#ifndef _FU_ASSERT_H_
#include "FUtils/FUAssert.h"
#endif // _FU_ASSERT_H_

// FMath
#define HAS_VECTORTYPES /**< Used by FCollada, implies that we are including all the common dynamically-sized arrays. */
#ifndef _F_MATH_H_
#include "FMath/FMath.h"
#endif // _F_MATH_H_

// LibXML
#ifndef NO_LIBXML
#define HAS_LIBXML /**< Used by FCollada, implies that we are including LibXML functions in the library interface. */
#if !defined(FCOLLADA_DLL) || defined(FCOLLADA_INTERNAL)
#define LIBXML_STATIC /**< Used by LibXML, implies that we are statically-linking the LibXML. */
#endif // !FCOLLADA_DLL || !FCOLLADA_INTERNAL
#ifndef __XML_TREE_H__
#include <libxml/tree.h>
#endif // __XML_TREE_H__
#else // NO_LIBXML
typedef struct _xmlNode xmlNode;
#endif // NO_LIBXML

// SAFE_DELETE Macro set.
#define SAFE_DELETE(ptr) if ((ptr) != NULL) { delete (ptr); (ptr) = NULL; } /**< This macro safely deletes a pointer and sets the given pointer to NULL. */
#define SAFE_DELETE_ARRAY(ptr) if (ptr != NULL) { delete [] ptr; ptr = NULL; } /**< This macro safely deletes an heap array and sets the given pointer to NULL. */
#define SAFE_FREE(ptr) if (ptr != NULL) { free(ptr); ptr = NULL; } /**< This macro safely frees a memory block and sets the given pointer to NULL. */
#define SAFE_RELEASE(ptr) { if ((ptr) != NULL) { (ptr)->Release(); (ptr) = NULL; } } /**< This macro safely releases an interface and sets the given pointer to NULL. */
#define CLEAR_POINTER_VECTOR(a) { size_t l = (a).size(); for (size_t i = 0; i < l; ++i) SAFE_DELETE((a)[i]); (a).clear(); } /**< This macro deletes all the object pointers contained within a vector and clears it. */
#define CLEAR_ARRAY_VECTOR(a) { size_t l = (a).size(); for (size_t i = 0; i < l; ++i) SAFE_DELETE_ARRAY((a)[i]); (a).clear(); } /**< This macro deletes all the object array pointers contained within a vector and clears it. */
#define CLEAR_POINTER_STD_PAIR_CONT(cont, a){ for (cont::iterator it = (a).begin(); it != (a).end(); ++it) SAFE_DELETE((*it).second); (a).clear(); } /**< This macro deletes all the object pointers contained within any std container and clears it. */
#define CLEAR_POINTER_STD_CONT(cont, a){ for (cont::iterator it = (a).begin(); it != (a).end(); ++it) SAFE_DELETE((*it)); (a).clear(); } /**< This macro deletes all the object pointers contained within any std container and clears it. */

// Base RTTI object and object container class
#ifndef _FU_OBJECT_TYPE_H_
#include "FUtils/FUObjectType.h"
#endif //_FU_OBJECT_TYPE_H_
#ifndef _FU_OBJECT_H_
#include "FUtils/FUObject.h"
#endif // _FU_OBJECT_H_
#ifndef _FU_TRACKER_H_
#include "FUtils/FUTracker.h"
#endif // _FU_TRACKER_H_

// More complex utility classes
#ifndef _FU_STRING_H_
#include "FUtils/FUString.h"
#endif // _FU_STRING_H_
#ifndef _FU_CRC32_H_
#include "FUtils/FUCrc32.h"
#endif // _FU_CRC32_H_
#ifndef _FU_DEBUG_H_
#include "FUtils/FUDebug.h"
#endif // _FU_DEBUG_H_
#ifndef _FU_ERROR_H_
#include "FUtils/FUError.h"
#endif // _FU_ERROR_H_

#ifndef RETAIL
#define ENABLE_TEST /**< Used by FCollada, enables the compilation and gives access to the unit tests. Disabled in the retail configurations. */
#endif

#endif // _F_UTILS_H_
