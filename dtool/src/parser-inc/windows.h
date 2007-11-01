// Filename: windows.h
// Created by:  drose (17Aug00)
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef WINDOWS_H
#define WINDOWS_H

typedef bool BOOL;
typedef long DWORD;
typedef unsigned long ULONG;
typedef long HRESULT;
typedef int CRITICAL_SECTION;
typedef int HANDLE;
typedef void *LPVOID;

#define WINAPI

union LARGE_INTEGER {
  __int64 QuadPart;
};

class IGraphBuilder;
class ICaptureGraphBuilder2;
class IBaseFilter;
class IMediaControl;
class IVMRWindowlessControl;
class CSampleGrabberCB;
template<class N> class CComPtr;

#endif
