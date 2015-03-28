// Filename: windows.h
// Created by:  drose (17Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef WINDOWS_H
#define WINDOWS_H

// http://msdn.microsoft.com/en-us/library/cc230309.aspx
typedef bool BOOL;
typedef long DWORD;
typedef long LONG;
typedef long UINT;
typedef unsigned long ULONG;
typedef signed long long LONGLONG;
typedef long HRESULT;
typedef int CRITICAL_SECTION;
typedef int HANDLE;
typedef int HGLOBAL;
typedef int HWAVEIN;
typedef void *LPSTR;
typedef void *LPWAVEHDR;
typedef void *LPVOID;
typedef void *DWORD_PTR;
typedef unsigned short WCHAR;
typedef WCHAR *BSTR;
typedef struct _MediaType AM_MEDIA_TYPE;
typedef struct _VIDEO_STREAM_CONFIG_CAPS VIDEO_STREAM_CONFIG_CAPS;
typedef struct _GUID GUID;
typedef struct _STICKYKEYS STICKYKEYS;
typedef struct _TOGGLEKEYS TOGGLEKEYS;
typedef struct _FILTERKEYS FILTERKEYS;

#define CALLBACK

#define WINAPI

union LARGE_INTEGER {
  long long QuadPart;
};

class IGraphBuilder;
class ICaptureGraphBuilder2;
class IBaseFilter;
class IMediaControl;
class IVMRWindowlessControl;
class CSampleGrabberCB;
template<class N> class CComPtr;

#endif
