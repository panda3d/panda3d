#ifndef __wcrext_h_
#define __wcrext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

/*************************************************************/

/* Header file version number */
#define WCR_WCREXT_VERSION 1

#ifndef WCR_ARB_buffer_region
#define WCR_FRONT_COLOR_BUFFER_BIT_ARB 0x00000001
#define WCR_BACK_COLOR_BUFFER_BIT_ARB  0x00000002
#define WCR_DEPTH_BUFFER_BIT_ARB       0x00000004
#define WCR_STENCIL_BUFFER_BIT_ARB     0x00000008
#endif

#ifndef WCR_ARB_extensions_string
#endif

#ifndef WCR_ARB_pixel_format
#define WCR_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WCR_DRAW_TO_WINDOW_ARB         0x2001
#define WCR_DRAW_TO_BITMAP_ARB         0x2002
#define WCR_ACCELERATION_ARB           0x2003
#define WCR_NEED_PALETTE_ARB           0x2004
#define WCR_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WCR_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WCR_SWAP_METHOD_ARB            0x2007
#define WCR_NUMBER_OVERLAYS_ARB        0x2008
#define WCR_NUMBER_UNDERLAYS_ARB       0x2009
#define WCR_TRANSPARENT_ARB            0x200A
#define WCR_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WCR_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WCR_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WCR_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WCR_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WCR_SHARE_DEPTH_ARB            0x200C
#define WCR_SHARE_STENCIL_ARB          0x200D
#define WCR_SHARE_ACCUM_ARB            0x200E
#define WCR_SUPPORT_GDI_ARB            0x200F
#define WCR_SUPPORT_OPENGL_ARB         0x2010
#define WCR_DOUBLE_BUFFER_ARB          0x2011
#define WCR_STEREO_ARB                 0x2012
#define WCR_PIXEL_TYPE_ARB             0x2013
#define WCR_COLOR_BITS_ARB             0x2014
#define WCR_RED_BITS_ARB               0x2015
#define WCR_RED_SHIFT_ARB              0x2016
#define WCR_GREEN_BITS_ARB             0x2017
#define WCR_GREEN_SHIFT_ARB            0x2018
#define WCR_BLUE_BITS_ARB              0x2019
#define WCR_BLUE_SHIFT_ARB             0x201A
#define WCR_ALPHA_BITS_ARB             0x201B
#define WCR_ALPHA_SHIFT_ARB            0x201C
#define WCR_ACCUM_BITS_ARB             0x201D
#define WCR_ACCUM_RED_BITS_ARB         0x201E
#define WCR_ACCUM_GREEN_BITS_ARB       0x201F
#define WCR_ACCUM_BLUE_BITS_ARB        0x2020
#define WCR_ACCUM_ALPHA_BITS_ARB       0x2021
#define WCR_DEPTH_BITS_ARB             0x2022
#define WCR_STENCIL_BITS_ARB           0x2023
#define WCR_AUX_BUFFERS_ARB            0x2024
#define WCR_NO_ACCELERATION_ARB        0x2025
#define WCR_GENERIC_ACCELERATION_ARB   0x2026
#define WCR_FULL_ACCELERATION_ARB      0x2027
#define WCR_SWAP_EXCHANGE_ARB          0x2028
#define WCR_SWAP_COPY_ARB              0x2029
#define WCR_SWAP_UNDEFINED_ARB         0x202A
#define WCR_TYPE_RGBA_ARB              0x202B
#define WCR_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WCR_ARB_make_current_read
#define ERROR_INVALID_PIXEL_TYPE_ARB   0x2043
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054
#endif

#ifndef WCR_ARB_pbuffer
#define WCR_DRAW_TO_PBUFFER_ARB        0x202D
#define WCR_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WCR_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WCR_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WCR_PBUFFER_LARGEST_ARB        0x2033
#define WCR_PBUFFER_WIDTH_ARB          0x2034
#define WCR_PBUFFER_HEIGHT_ARB         0x2035
#define WCR_PBUFFER_LOST_ARB           0x2036
#endif

#ifndef WCR_EXT_make_current_read
#define ERROR_INVALID_PIXEL_TYPE_EXT   0x2043
#endif

#ifndef WCR_EXT_pixel_format
#define WCR_NUMBER_PIXEL_FORMATS_EXT   0x2000
#define WCR_DRAW_TO_WINDOW_EXT         0x2001
#define WCR_DRAW_TO_BITMAP_EXT         0x2002
#define WCR_ACCELERATION_EXT           0x2003
#define WCR_NEED_PALETTE_EXT           0x2004
#define WCR_NEED_SYSTEM_PALETTE_EXT    0x2005
#define WCR_SWAP_LAYER_BUFFERS_EXT     0x2006
#define WCR_SWAP_METHOD_EXT            0x2007
#define WCR_NUMBER_OVERLAYS_EXT        0x2008
#define WCR_NUMBER_UNDERLAYS_EXT       0x2009
#define WCR_TRANSPARENT_EXT            0x200A
#define WCR_TRANSPARENT_VALUE_EXT      0x200B
#define WCR_SHARE_DEPTH_EXT            0x200C
#define WCR_SHARE_STENCIL_EXT          0x200D
#define WCR_SHARE_ACCUM_EXT            0x200E
#define WCR_SUPPORT_GDI_EXT            0x200F
#define WCR_SUPPORT_OPENGL_EXT         0x2010
#define WCR_DOUBLE_BUFFER_EXT          0x2011
#define WCR_STEREO_EXT                 0x2012
#define WCR_PIXEL_TYPE_EXT             0x2013
#define WCR_COLOR_BITS_EXT             0x2014
#define WCR_RED_BITS_EXT               0x2015
#define WCR_RED_SHIFT_EXT              0x2016
#define WCR_GREEN_BITS_EXT             0x2017
#define WCR_GREEN_SHIFT_EXT            0x2018
#define WCR_BLUE_BITS_EXT              0x2019
#define WCR_BLUE_SHIFT_EXT             0x201A
#define WCR_ALPHA_BITS_EXT             0x201B
#define WCR_ALPHA_SHIFT_EXT            0x201C
#define WCR_ACCUM_BITS_EXT             0x201D
#define WCR_ACCUM_RED_BITS_EXT         0x201E
#define WCR_ACCUM_GREEN_BITS_EXT       0x201F
#define WCR_ACCUM_BLUE_BITS_EXT        0x2020
#define WCR_ACCUM_ALPHA_BITS_EXT       0x2021
#define WCR_DEPTH_BITS_EXT             0x2022
#define WCR_STENCIL_BITS_EXT           0x2023
#define WCR_AUX_BUFFERS_EXT            0x2024
#define WCR_NO_ACCELERATION_EXT        0x2025
#define WCR_GENERIC_ACCELERATION_EXT   0x2026
#define WCR_FULL_ACCELERATION_EXT      0x2027
#define WCR_SWAP_EXCHANGE_EXT          0x2028
#define WCR_SWAP_COPY_EXT              0x2029
#define WCR_SWAP_UNDEFINED_EXT         0x202A
#define WCR_TYPE_RGBA_EXT              0x202B
#define WCR_TYPE_COLORINDEX_EXT        0x202C
#endif

#ifndef WCR_EXT_pbuffer
#define WCR_DRAW_TO_PBUFFER_EXT        0x202D
#define WCR_MAX_PBUFFER_PIXELS_EXT     0x202E
#define WCR_MAX_PBUFFER_WIDTH_EXT      0x202F
#define WCR_MAX_PBUFFER_HEIGHT_EXT     0x2030
#define WCR_OPTIMAL_PBUFFER_WIDTH_EXT  0x2031
#define WCR_OPTIMAL_PBUFFER_HEIGHT_EXT 0x2032
#define WCR_PBUFFER_LARGEST_EXT        0x2033
#define WCR_PBUFFER_WIDTH_EXT          0x2034
#define WCR_PBUFFER_HEIGHT_EXT         0x2035
#endif

#ifndef WCR_EXT_depth_float
#define WCR_DEPTH_FLOAT_EXT            0x2040
#endif

#ifndef WCR_3DFX_multisample
#define WCR_SAMPLE_BUFFERS_3DFX        0x2060
#define WCR_SAMPLES_3DFX               0x2061
#endif

#ifndef WCR_EXT_multisample
#define WCR_SAMPLE_BUFFERS_EXT         0x2041
#define WCR_SAMPLES_EXT                0x2042
#endif

#ifndef WCR_I3D_unknown_genlock_extension_name
#define WCR_GENLOCK_SOURCE_MULTIVIEW_I3D 0x2044
#define WCR_GENLOCK_SOURCE_EXTENAL_SYNC_I3D 0x2045
#define WCR_GENLOCK_SOURCE_EXTENAL_FIELD_I3D 0x2046
#define WCR_GENLOCK_SOURCE_EXTENAL_TTL_I3D 0x2047
#define WCR_GENLOCK_SOURCE_DIGITAL_SYNC_I3D 0x2048
#define WCR_GENLOCK_SOURCE_DIGITAL_FIELD_I3D 0x2049
#define WCR_GENLOCK_SOURCE_EDGE_FALLING_I3D 0x204A
#define WCR_GENLOCK_SOURCE_EDGE_RISING_I3D 0x204B
#define WCR_GENLOCK_SOURCE_EDGE_BOTH_I3D 0x204C
#endif

#ifndef WCR_I3D_unknown_gamma_extension_name
#define WCR_GAMMA_TABLE_SIZE_I3D       0x204E
#define WCR_GAMMA_EXCLUDE_DESKTOP_I3D  0x204F
#endif

#ifndef WCR_I3D_unknown_digital_video_cursor_extension_name
#define WCR_DIGITAL_VIDEO_CURSOR_ALPHA_FRAMEBUFFER_I3D 0x2050
#define WCR_DIGITAL_VIDEO_CURSOR_ALPHA_VALUE_I3D 0x2051
#define WCR_DIGITAL_VIDEO_CURSOR_INCLUDED_I3D 0x2052
#define WCR_DIGITAL_VIDEO_GAMMA_CORRECTED_I3D 0x2053
#endif


/*************************************************************/

#ifndef WCR_ARB_pbuffer
DECLARE_HANDLE(HPBUFFERARB);
#endif
#ifndef WCR_EXT_pbuffer
DECLARE_HANDLE(HPBUFFEREXT);
#endif

#ifndef WCR_ARB_buffer_region
#define WCR_ARB_buffer_region 1
#ifdef WCR_WCREXT_PROTOTYPES
extern HANDLE WINAPI wcrCreateBufferRegionARB (HDC, int, UINT);
extern VOID WINAPI wcrDeleteBufferRegionARB (HANDLE);
extern BOOL WINAPI wcrSaveBufferRegionARB (HANDLE, int, int, int, int);
extern BOOL WINAPI wcrRestoreBufferRegionARB (HANDLE, int, int, int, int, int, int);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef HANDLE (WINAPI * PFNWCRCREATEBUFFERREGIONARBPROC) (HDC hDC, int iLayerPlane, UINT uType);
typedef VOID (WINAPI * PFNWCRDELETEBUFFERREGIONARBPROC) (HANDLE hRegion);
typedef BOOL (WINAPI * PFNWCRSAVEBUFFERREGIONARBPROC) (HANDLE hRegion, int x, int y, int width, int height);
typedef BOOL (WINAPI * PFNWCRRESTOREBUFFERREGIONARBPROC) (HANDLE hRegion, int x, int y, int width, int height, int xSrc, int ySrc);
#endif

#ifndef WCR_ARB_extensions_string
#define WCR_ARB_extensions_string 1
#ifdef WCR_WCREXT_PROTOTYPES
extern const char * WINAPI wcrGetExtensionsStringARB (HDC);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef const char * (WINAPI * PFNWCRGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
#endif

#ifndef WCR_ARB_pixel_format
#define WCR_ARB_pixel_format 1
#ifdef WCR_WCREXT_PROTOTYPES
extern BOOL WINAPI wcrGetPixelFormatAttribivARB (HDC, int, int, UINT, const int *, int *);
extern BOOL WINAPI wcrGetPixelFormatAttribfvARB (HDC, int, int, UINT, const int *, FLOAT *);
extern BOOL WINAPI wcrChoosePixelFormatARB (HDC, const int *, const FLOAT *, UINT, int *, UINT *);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWCRGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWCRGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWCRCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#endif

#ifndef WCR_ARB_make_current_read
#define WCR_ARB_make_current_read 1
#ifdef WCR_WCREXT_PROTOTYPES
extern BOOL WINAPI wcrMakeContextCurrentARB (HDC, HDC, HGLRC);
extern HDC WINAPI wcrGetCurrentReadDCARB ();
#endif /* WCR_WCREXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWCRMAKECONTEXTCURRENTARBPROC) (HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
typedef HDC (WINAPI * PFNWCRGETCURRENTREADDCARBPROC) ();
#endif

#ifndef WCR_ARB_pbuffer
#define WCR_ARB_pbuffer 1
#ifdef WCR_WCREXT_PROTOTYPES
extern HPBUFFERARB WINAPI wcrCreatePbufferARB (HDC, int, int, int, const int *);
extern HDC WINAPI wcrGetPbufferDCARB (HPBUFFERARB);
extern int WINAPI wcrReleasePbufferDCARB (HPBUFFERARB, HDC);
extern BOOL WINAPI wcrDestroyPbufferARB (HPBUFFERARB);
extern BOOL WINAPI wcrQueryPbufferARB (HPBUFFERARB, int, int *);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef HPBUFFERARB (WINAPI * PFNWCRCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWCRGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * PFNWCRRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWCRDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWCRQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
#endif

#ifndef WCR_EXT_display_color_table
#define WCR_EXT_display_color_table 1
#ifdef WCR_WCREXT_PROTOTYPES
extern GLboolean WINAPI wcrCreateDisplayColorTableEXT (GLushort);
extern GLboolean WINAPI wcrLoadDisplayColorTableEXT (const GLushort *, GLuint);
extern GLboolean WINAPI wcrBindDisplayColorTableEXT (GLushort);
extern VOID WINAPI wcrDestroyDisplayColorTableEXT (GLushort);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef GLboolean (WINAPI * PFNWCRCREATEDISPLAYCOLORTABLEEXTPROC) (GLushort id);
typedef GLboolean (WINAPI * PFNWCRLOADDISPLAYCOLORTABLEEXTPROC) (const GLushort *table, GLuint length);
typedef GLboolean (WINAPI * PFNWCRBINDDISPLAYCOLORTABLEEXTPROC) (GLushort id);
typedef VOID (WINAPI * PFNWCRDESTROYDISPLAYCOLORTABLEEXTPROC) (GLushort id);
#endif

#ifndef WCR_EXT_extensions_string
#define WCR_EXT_extensions_string 1
#ifdef WCR_WCREXT_PROTOTYPES
extern const char * WINAPI wcrGetExtensionsStringEXT ();
#endif /* WCR_WCREXT_PROTOTYPES */
typedef const char * (WINAPI * PFNWCRGETEXTENSIONSSTRINGEXTPROC) ();
#endif

#ifndef WCR_EXT_make_current_read
#define WCR_EXT_make_current_read 1
#ifdef WCR_WCREXT_PROTOTYPES
extern BOOL WINAPI wcrMakeContextCurrentEXT (HDC, HDC, HGLRC);
extern HDC WINAPI wcrGetCurrentReadDCEXT ();
#endif /* WCR_WCREXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWCRMAKECONTEXTCURRENTEXTPROC) (HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
typedef HDC (WINAPI * PFNWCRGETCURRENTREADDCEXTPROC) ();
#endif

#ifndef WCR_EXT_pbuffer
#define WCR_EXT_pbuffer 1
#ifdef WCR_WCREXT_PROTOTYPES
extern HPBUFFEREXT WINAPI wcrCreatePbufferEXT (HDC, int, int, int, const int *);
extern HDC WINAPI wcrGetPbufferDCEXT (HPBUFFEREXT);
extern int WINAPI wcrReleasePbufferDCEXT (HPBUFFEREXT, HDC);
extern BOOL WINAPI wcrDestroyPbufferEXT (HPBUFFEREXT);
extern BOOL WINAPI wcrQueryPbufferEXT (HPBUFFEREXT, int, int *);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef HPBUFFEREXT (WINAPI * PFNWCRCREATEPBUFFEREXTPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWCRGETPBUFFERDCEXTPROC) (HPBUFFEREXT hPbuffer);
typedef int (WINAPI * PFNWCRRELEASEPBUFFERDCEXTPROC) (HPBUFFEREXT hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWCRDESTROYPBUFFEREXTPROC) (HPBUFFEREXT hPbuffer);
typedef BOOL (WINAPI * PFNWCRQUERYPBUFFEREXTPROC) (HPBUFFEREXT hPbuffer, int iAttribute, int *piValue);
#endif

#ifndef WCR_EXT_pixel_format
#define WCR_EXT_pixel_format 1
#ifdef WCR_WCREXT_PROTOTYPES
extern BOOL WINAPI wcrGetPixelFormatAttribivEXT (HDC, int, int, UINT, int *, int *);
extern BOOL WINAPI wcrGetPixelFormatAttribfvEXT (HDC, int, int, UINT, int *, FLOAT *);
extern BOOL WINAPI wcrChoosePixelFormatEXT (HDC, const int *, const FLOAT *, UINT, int *, UINT *);
#endif /* WCR_WCREXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWCRGETPIXELFORMATATTRIBIVEXTPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWCRGETPIXELFORMATATTRIBFVEXTPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWCRCHOOSEPIXELFORMATEXTPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#endif

#ifndef WCR_EXT_swap_control
#define WCR_EXT_swap_control 1
#ifdef WCR_WCREXT_PROTOTYPES
extern BOOL WINAPI wcrSwapIntervalEXT (int);
extern int WINAPI wcrGetSwapIntervalEXT ();
#endif /* WCR_WCREXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWCRSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI * PFNWCRGETSWAPINTERVALEXTPROC) ();
#endif

#ifndef WCR_WCR_EXT_depth_float
#define WCR_WCR_EXT_depth_float 1
#endif

#ifndef WCR_WCR_3DFX_multisample
#define WCR_WCR_3DFX_multisample 1
#endif

#ifndef WCR_WCR_EXT_multisample
#define WCR_WCR_EXT_multisample 1
#endif

/* added by Cass -- but this should already be in here! */
#ifndef WCR_NV_allocate_memory
#define WCR_NV_allocate_memory 1
#ifdef WCR_WCREXT_PROTOTYPES
extern void * wcrAllocateMemoryNV(int size, float readfreq, float writefreq, float priority);
extern void wcrFreeMemoryNV(void * pointer); 
#endif
typedef void * (APIENTRY * PFNWCRALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
typedef void (APIENTRY * PFNWCRFREEMEMORYNVPROC) (void *pointer);
#endif


#ifdef __cplusplus
}
#endif

#endif


