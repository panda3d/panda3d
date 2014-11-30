/*
 *	Video capture subrutine for Linux/libdc1394 devices
 *	author: Kiyoshi Kiyokawa ( kiyo@crl.go.jp )
 *	        Hirokazu Kato ( kato@sys.im.hiroshima-cu.ac.jp )
 *
 *	Revision: 1.0   Date: 2002/01/01
 *
 */
/*
 *	Copyright (c) 2003-2007 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *	1.1.0	2003-09-09	PRL		Based on Apple "Son of MungGrab" sample code for QuickTime 6.
 *								Added config option "-fps" to superimpose frame counter on video.
 *								Returns aligned data in ARGB pixel format.
 *  1.2.0   2004-04-28  PRL		Now one thread per video source. Versions of QuickTime
 *								prior to 6.4 are NOT thread safe, and with these earlier
 *								versions, QuickTime toolbox access will be serialised.
 *	1.2.1   2004-06-28  PRL		Support for 2vuy and yuvs pixel formats.
 *  1.3.0   2004-07-13  PRL		Code from Daniel Heckenberg to directly access vDig.
 *  1.3.1   2004-12-07  PRL		Added config option "-pixelformat=" to support pixel format
 *								specification at runtime, with default determined at compile time.
 *	1.4.0	2005-03-08	PRL		Video input settings now saved and restored.
 *  1.4.1   2005-03-15  PRL     QuickTime 6.4 or newer is now required by default. In order
 *								to allow earlier versions, AR_VIDEO_SUPPORT_OLD_QUICKTIME must
 *								be uncommented at compile time.
 *
 */
/*
 * 
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
 * 
 */

#ifndef AR_VIDEO_MACOSX_H
#define AR_VIDEO_MACOSX_H
#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _AR2VideoParamT AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif // AR_VIDEO_MACOSX_H
