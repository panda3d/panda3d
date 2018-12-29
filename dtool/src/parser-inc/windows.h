/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windows.h
 * @author drose
 * @date 2000-08-17
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef WINDOWS_H
#define WINDOWS_H

#include <wtypes.h>

#ifdef _WIN64
typedef int HALF_PTR;
typedef long long INT_PTR;
typedef long long LONG_PTR;
typedef unsigned long long UINT_PTR;
typedef unsigned long long ULONG_PTR;
#else
typedef short HALF_PTR;
typedef int INT_PTR;
typedef long LONG_PTR;
typedef unsigned int UINT_PTR;
typedef unsigned long ULONG_PTR;
#endif

// http://msdn.microsoft.com/en-us/library/cc230309.aspx
typedef bool BOOL;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef long LONG;
typedef long UINT;
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef long long LONGLONG;
typedef long HRESULT;
typedef int CRITICAL_SECTION;
typedef void *LPSTR;
typedef void *LPWAVEHDR;
typedef void *PVOID;
typedef void *LPVOID;
typedef PVOID HANDLE;
typedef HANDLE HGLOBAL;
typedef HANDLE HWAVEIN;
typedef HANDLE HWND;
typedef ULONG_PTR DWORD_PTR;
typedef DWORD_PTR *PDWORD_PTR;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef wchar_t WCHAR;
typedef WCHAR *BSTR;
typedef struct _MediaType AM_MEDIA_TYPE;
typedef struct _VIDEO_STREAM_CONFIG_CAPS VIDEO_STREAM_CONFIG_CAPS;
typedef struct _GUID GUID;
typedef struct _STICKYKEYS STICKYKEYS;
typedef struct _TOGGLEKEYS TOGGLEKEYS;
typedef struct _FILTERKEYS FILTERKEYS;

#define CALLBACK

#define WINAPI

class IGraphBuilder;
class ICaptureGraphBuilder2;
class IBaseFilter;
class IMediaControl;
class IVMRWindowlessControl;
class CSampleGrabberCB;
template<class N> class CComPtr;

#endif
