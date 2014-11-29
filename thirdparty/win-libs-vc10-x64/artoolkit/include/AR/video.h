/*  --------------------------------------------------------------------------
 *	Copyright (c) 2004-2007 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *   -------------------------------------------------------------------------*/
/**
 *  \file video.h
 *  \brief ARToolkit video subroutines.
 *
 *  This library provides multi-platform video input support for ARToolKit. 
 *  It abstracts access to hardware video input available on different machines and
 *  operating systems. 
 *
 *  The actual supported platforms (and the driver/library used) are:
 *  - Windows: with Microsoft DirectShow (VFW obsolete).
 *  - Linux: with Video4Linux library, GStreamer, IEEE1394 camera library and DV camera library.
 *  - Macintosh: with QuickTime.
 *  - SGI: with VL.
 *
 *  This library provides two sets of functions, depending on whether your program
 *  needs to use only one video stream, or more than one video stream. These
 *  two sets are functionally identical.
 *  - one camera: use the <b>arVideo*</b> functions.
 *  - multiple cameras: use the <b>ar2Video*</b> functions.
 *
 *  More information on establishing video streams is available in the ARToolKit manual.
 *
 *  \remark The arVideo* functions maintain the current video stream in a global variable
 *  and internally call the ar2Video* functions.
 *
 *  History :
 *		modified by Thomas Pintaric (pintaric@ims.tuwien.ac.at) to add
 *       a fully transparent DirectShow Driver.
 *      modified by Hartmut Seichter (hartmut@technotecture.com) to add
 *       GStreamer video support
 *
 *  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp
 *  \author Atsishi Nakazawa nakazawa@inolab.sys.es.osaka-u.ac.jp
 *  \author Thomas Pintaric pintaric@ims.tuwien.ac.at (Windows DirectShow video support).
 *  \author Philip Lamb phil@eden.net.nz (Macintosh Quicktime video support).
 *  \author Hartmut Seichter hartmut@technotecture.com (GStreamer Video support)
 *  \version 4.3b
 *  \date 03/02/02
 *
 */
/* --------------------------------------------------------------------------
 * History : 
 * Rev		Date		Who		Changes
 * 2.6.8	2004-07-20	PRL		Rewrite for ARToolKit 2.68.2
 *
 *----------------------------------------------------------------------------
 */

#ifndef AR_VIDEO_H
#define AR_VIDEO_H

#ifdef  __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <AR/config.h>
#include <AR/ar.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

#ifdef _WIN32
#  include <AR/sys/videoWin32DirectShow.h>
#  ifdef LIBARVIDEO_EXPORTS
#    define AR_DLL_API __declspec(dllexport)
#  else
#    ifdef _DLL
#      define AR_DLL_API __declspec(dllimport)
#    else
#      define AR_DLL_API extern
#    endif
#  endif
#else
#  define AR_DLL_API
#endif

#ifdef __linux
#  ifdef AR_INPUT_V4L
#    include <AR/sys/videoLinuxV4L.h>
#  endif
#  ifdef  AR_INPUT_DV
#    include <AR/sys/videoLinuxDV.h>
#  endif
#  ifdef  AR_INPUT_1394CAM
#    include <AR/sys/videoLinux1394Cam.h>
#  endif
#  ifdef  AR_INPUT_GSTREAMER
#    include <AR/sys/videoGStreamer.h>
#  endif
#endif

#ifdef __sgi
#  include <AR/sys/videoSGI.h>
#endif

#ifdef __APPLE__
#  include <AR/sys/videoMacOSX.h>
#endif

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/*
	mono-camera
*/
	
/**
 * \brief display the video option.
 *
 * The video configuration options vary by operating system and platform.
 * This function outputs to the standard output the options available
 * on the current OS and platform.
 * \return 0
 */
AR_DLL_API  int				arVideoDispOption(void);

/**
 * \brief open a video source.
 * 
 * This function opens a video input path with the
 * driver (and device) present on your platform. 
 * According to your operating system and the
 * hardware the initialization will be different : a
 * generic string structure is used for this issue.
 * This function prepares the video stream for capture,
 * but capture will not actually begin until arVideoCapStart
 * is called.
 * \param config string of the selected video configuration.
 * See the <a href="video/">video configuration documentation</a>
 * for more information on this parameter.
 * \return 0 if successful, -1 if a video path couldn't be opened
 */	
AR_DLL_API  int				arVideoOpen(char *config);

/**
 * \brief close the video source.
 * After your application has finished using a video stream,
 * this function must be called to close the link to the
 * input source, and free resources associated with the
 * capture operation.
 * \return 0 if shut down successfully, otherwise -1.
 */
AR_DLL_API  int				arVideoClose(void);

/**
 * \brief start the capture of video.
 *
 * This function starts the video capture routine.
 * \remark On some operating systems, capture operations
 * may run in a separate execution thread. This call starts that thread.
 * \remark this function coupled with arVideoCapStop, can
 * be call many times in your program (this may reduce the CPU load
 * when video processing is stopped or for long and critical operations).
 * \return 0 if successful, -1 if the capture couldn't be started.
 */
AR_DLL_API  int				arVideoCapStart(void);

/**
 * \brief stop the capture of video.
 *
 * This function stops the video capture routine.
 * \remark On some operating systems, capture operations
 * may run in a separate execution thread. This call stops that thread.
 * \remark this function coupled with arVideoCapStart, can
 * be call many times in your program (this may reduce the CPU load
 * when video processing is stopped or for long and critical operations).
 * \return 0 if successful, -1 if the capture couldn't be stopped.
 */
AR_DLL_API  int				arVideoCapStop(void);

/**
 * \brief call for the next grabbed video frame.
 *
 * This function should be called at least once per frame.
 * It has several purposes, depending on the operating system.
 * It allows the video driver to perform housekeeping tasks
 * and also signals to the video grabber that your code
 * has finished using the most recent video frame returned by
 * arVideoGetImage(), and that the video driver may re-use the
 * memory occupied by the frame.
 * The effect of this call is operating-system dependent.
 * The best place to call this function is immediately after
 * you have finished displaying the current video frame, i.e.
 * after calling arglDispImage() or argDispImage().
 * 
 * \remark On some operating systems, this function is a no-op.
 * \return 0 if successful, -1 if the video driver encountered an error.
 */
AR_DLL_API  int				arVideoCapNext(void);

/**
 * \brief get the video image.
 *
 * This function returns a buffer with a captured video image.
 * The returned data consists of a tightly-packed array of
 * pixels, beginning with the first component of the leftmost
 * pixel of the topmost row, and continuing with the remaining
 * components of that pixel, followed by the remaining pixels
 * in the topmost row, followed by the leftmost pixel of the
 * second row, and so on.
 * The arrangement of components of the pixels in the buffer is
 * determined by the configuration string passed in to the driver
 * at the time the video stream was opened. If no pixel format
 * was specified in the configuration string, then an operating-
 * system dependent default, defined in <AR/config.h> is used.
 * The memory occupied by the pixel data is owned by the video
 * driver and should not be freed by your program.
 * The pixels in the buffer remain valid until the next call to
 * arVideoCapNext, or the next call to arVideoGetImage which
 * returns a non-NULL pointer, or any call to arVideoCapStop or
 * arVideoClose.
 * \return A pointer to the pixel data of the captured video frame,
 * or NULL if no new pixel data was available at the time of calling.
 */
AR_DLL_API  ARUint8*		arVideoGetImage(void);

/**
 * \brief get the video image size, in pixels.
 *
 * This function returns the size of the captured video frame, in
 * pixels.
 * \param x a pointer to the length of the captured image
 * \param y a pointer to the width of the captured image
 * \return 0 if the dimensions are found successfully, otherwise -1
 */
AR_DLL_API  int				arVideoInqSize(int *x, int *y);

/*
	multiple cameras
 */

/**
 * \brief display the video option (multiple video inputs)
 *
 * Companion function to arVideoDispOption, for multiple video sources.
 */
AR_DLL_API  int				ar2VideoDispOption(void);

/**
 * \brief open a video source (multiple video inputs)
 *
 *  Companion function to arVideoOpen for multiple video sources.
 *  This function can be called multiple times to open multiple
 *  video streams. The maximum number of streams is dependent on
 *  the operating system and the performance characteristics of
 *  the host CPU and video capture infrastructure.
 *
 * \param config string of the selected video configuration.
 * \return If the video path was successfully opened, this
 * function returns a pointer to an AR2VideoParamT structure,
 * an opaque structure which holds information and configuration
 * for the video stream. This paramater should then be passed
 * to other ar2Video* functions to specify which video stream
 * is being operated upon. If the video path was not successfully
 * opened, NULL will be returned.
 s*/
AR_DLL_API  AR2VideoParamT  *ar2VideoOpen(char *config);

/**
 * \brief close a video source (multiple video inputs)
 *
 * Companion function to arVideoClose for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing.
 */
AR_DLL_API  int				ar2VideoClose(AR2VideoParamT *vid);

/**
 * \brief start the capture of a video source (multiple video inputs)
 *
 * Companion function to arVideoCapStart for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing
 */
AR_DLL_API  int				ar2VideoCapStart(AR2VideoParamT *vid);

/**
 * \brief call for the next grabbed video frame of a video source (multiple video inputs)
 *
 * Companion function to arVideoCapNext for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing
 */
AR_DLL_API  int				ar2VideoCapNext(AR2VideoParamT *vid);

/**
 * \brief stop the capture of a video source (multiple video inputs)
 *
 * Companion function to arVideoCapStop for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing
 */
AR_DLL_API  int				ar2VideoCapStop(AR2VideoParamT *vid);

/**
 * \brief get a video image from a video source (multiple video inputs)
 *
 * Companion function to arVideoGetImage for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing
 */
AR_DLL_API  ARUint8			*ar2VideoGetImage(AR2VideoParamT *vid);

/**
 * \brief get the video image size of a video source (multiple video inputs)
 *
 * Companion function to arVideoInqSize for multiple video sources.
 * \param vid a video handle structure for multi-camera grabbing
 */
AR_DLL_API  int				ar2VideoInqSize(AR2VideoParamT *vid, int *x, int *y);

// Functions added for Studierstube/OpenTracker.
#ifdef _WIN32
#  ifndef __MEMORY_BUFFER_HANDLE__
#  define __MEMORY_BUFFER_HANDLE__
#  define DEFAULT_NUMBER_OF_ALLOCATOR_BUFFERS 3
typedef struct _MemoryBufferHandle
{
	unsigned long  n; // sample number
	__int64 t;		  // timestamp
} MemoryBufferHandle;
#  endif // __MEMORY_BUFFER_HANDLE__
AR_DLL_API  int				ar2VideoInqFreq(AR2VideoParamT *vid, float *fps);
AR_DLL_API  int				ar2VideoInqFlipping(AR2VideoParamT *vid, int *flipH, int *flipV);
AR_DLL_API  unsigned char	*ar2VideoLockBuffer(AR2VideoParamT *vid, MemoryBufferHandle *pHandle);
AR_DLL_API  int				ar2VideoUnlockBuffer(AR2VideoParamT *vid, MemoryBufferHandle Handle);
#endif // _WIN32

#ifdef  __cplusplus
}
#endif

#endif // AR_VIDEO_H
