/*  --------------------------------------------------------------------------
*   Copyright (C) 2004 Hitlab NZ.
*   The distribution policy is describe on the Copyright.txt furnish 
*    with this library.
*   -------------------------------------------------------------------------*/
/**
*  \file gsubUtil.h
*  \brief ARToolkit OpenGL utilities subroutines.
*
*  Complement routines of gsub module.
*   \remark 
*
*   History :
*
*  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp
*  \version 
*  \date 
**/
/*  --------------------------------------------------------------------------
*   History : 
*   Rev		Date		Who		Changes
*
*----------------------------------------------------------------------------*/
#ifndef AR_GSUB_UTIL_H
#define AR_GSUB_UTIL_H
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <AR/param.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/** \fn void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) )
* \brief utility function for calibrate an HMD.
*
* This function offers a full calibration run-time routines for an optical HMD (mono
* or stereo).
* It is useful for estimate transformation between user eye position and 
* camera position. You will find more informations on the calibration routine
* on opticalcalibration.html .This function modify gsub state of left and right camera
* intrinsic parameters. 
* \param targetId the target used for the calibration step.
* \param thresh2 lighting threshold value to use
* \param postFunc a callback function used to analysis computed internal camera
* parameters. if your application is mono display, only lpara contains a value.
* lpara and rpara are NULL if the calibration failed.
*/
void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) );

#ifdef __cplusplus
}
#endif
#endif
