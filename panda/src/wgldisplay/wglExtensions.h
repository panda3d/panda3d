// Filename: wglExtensions.h
// Created by:  drose (09Feb04)
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

#ifndef WGLEXTENSIONS_H
#define WGLEXTENSIONS_H

#include "pandabase.h"

#include <windows.h>

// This file defines some headers that are necessary to make use of
// the various wgl extensions we might test for in this module.


// The following headers are from the WGL_ARB_pbuffer extension for
// offscreen rendering into Pbuffers.

DECLARE_HANDLE(HPBUFFERARB);

typedef HPBUFFERARB (*wglCreatePbufferARB_proc)(HDC hDC,
                                                int iPixelFormat,
                                                int iWidth,
                                                int iHeight,
                                                const int *piAttribList);

typedef HDC (*wglGetPbufferDCARB_proc)(HPBUFFERARB hPbuffer);

typedef int (*wglReleasePbufferDCARB_proc)(HPBUFFERARB hPbuffer,
                                           HDC hDC);

typedef BOOL (*wglDestroyPbufferARB_proc)(HPBUFFERARB hPbuffer);

typedef BOOL (*wglQueryPbufferARB_proc)(HPBUFFERARB hPbuffer,
                                        int iAttribute,
                                        int *piValue);

#define WGL_DRAW_TO_PBUFFER_ARB              0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB           0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB            0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB           0x2030
#define WGL_PBUFFER_LARGEST_ARB              0x2033
#define WGL_PBUFFER_WIDTH_ARB                0x2034
#define WGL_PBUFFER_HEIGHT_ARB               0x2035
#define WGL_PBUFFER_LOST_ARB                 0x2036


// The following headers are from the WGL_ARB_pixel_format extension
// for getting extended pixel format information.

typedef BOOL (*wglGetPixelFormatAttribivARB_proc)(HDC hdc,
                                                  int iPixelFormat,
                                                  int iLayerPlane,
                                                  UINT nAttributes,
                                                  const int *piAttributes,
                                                  int *piValues);

typedef BOOL (*wglGetPixelFormatAttribfvARB_proc)(HDC hdc,
                                                  int iPixelFormat,
                                                  int iLayerPlane,
                                                  UINT nAttributes,
                                                  const int *piAttributes,
                                                  FLOAT *pfValues);

typedef BOOL (*wglChoosePixelFormatARB_proc)(HDC hdc,
                                             const int *piAttribIList,
                                             const FLOAT *pfAttribFList,
                                             UINT nMaxFormats,
                                             int *piFormats,
                                             UINT *nNumFormats);

#define WGL_NUMBER_PIXEL_FORMATS_ARB		0x2000
#define WGL_DRAW_TO_WINDOW_ARB			0x2001
#define WGL_DRAW_TO_BITMAP_ARB			0x2002
#define WGL_ACCELERATION_ARB			0x2003
#define WGL_NEED_PALETTE_ARB			0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB		0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB		0x2006
#define WGL_SWAP_METHOD_ARB			0x2007
#define WGL_NUMBER_OVERLAYS_ARB			0x2008
#define WGL_NUMBER_UNDERLAYS_ARB		0x2009
#define WGL_TRANSPARENT_ARB			0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB		0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB		0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB		0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB		0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB		0x203B
#define WGL_SHARE_DEPTH_ARB			0x200C
#define WGL_SHARE_STENCIL_ARB			0x200D
#define WGL_SHARE_ACCUM_ARB			0x200E
#define WGL_SUPPORT_GDI_ARB			0x200F
#define WGL_SUPPORT_OPENGL_ARB			0x2010
#define WGL_DOUBLE_BUFFER_ARB			0x2011
#define WGL_STEREO_ARB				0x2012
#define WGL_PIXEL_TYPE_ARB			0x2013
#define WGL_COLOR_BITS_ARB			0x2014
#define WGL_RED_BITS_ARB			0x2015
#define WGL_RED_SHIFT_ARB			0x2016
#define WGL_GREEN_BITS_ARB			0x2017
#define WGL_GREEN_SHIFT_ARB			0x2018
#define WGL_BLUE_BITS_ARB			0x2019
#define WGL_BLUE_SHIFT_ARB			0x201A
#define WGL_ALPHA_BITS_ARB			0x201B
#define WGL_ALPHA_SHIFT_ARB			0x201C
#define WGL_ACCUM_BITS_ARB			0x201D
#define WGL_ACCUM_RED_BITS_ARB			0x201E
#define WGL_ACCUM_GREEN_BITS_ARB		0x201F
#define WGL_ACCUM_BLUE_BITS_ARB			0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB		0x2021
#define WGL_DEPTH_BITS_ARB			0x2022
#define WGL_STENCIL_BITS_ARB			0x2023
#define WGL_AUX_BUFFERS_ARB			0x2024

#define WGL_NO_ACCELERATION_ARB			0x2025
#define WGL_GENERIC_ACCELERATION_ARB		0x2026
#define WGL_FULL_ACCELERATION_ARB		0x2027

#define WGL_SWAP_EXCHANGE_ARB			0x2028
#define WGL_SWAP_COPY_ARB			0x2029
#define WGL_SWAP_UNDEFINED_ARB			0x202A

#define WGL_TYPE_RGBA_ARB			0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

#endif
