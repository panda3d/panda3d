/*  --------------------------------------------------------------------------
*   Copyright (C) 2004 Hitlab NZ.
*   The distribution policy is describe on the Copyright.txt furnish 
*    with this library.
*   -------------------------------------------------------------------------*/
/**
*  \file param.h
*  \brief ARToolkit global structure (parameters) subroutines.
*
*  This file contains principal routines for loading, saving, and modify
*  camera parameters for of ARToolkit library. Different structures are used
*  for modify in run-time this parameters in the library. A file structure
*  is use for input/output.
*   \remark 
*
*   History :
*
*  \author Takeshi Mita tmita@inolab.sys.es.osaka-u.ac.jp
*  \author Shinsaku Hiura shinsaku@sys.es.osaka-u.ac.jp
*  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp

*  \version  4.1
*  \date 01/12/07
**/
/*  --------------------------------------------------------------------------
*   History : 
*   Rev		Date		Who		Changes
*
*----------------------------------------------------------------------------*/

#ifndef AR_PARAM_H
#define AR_PARAM_H
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <AR/config.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

/** \struct ARParam
* \brief camera intrinsic parameters.
* 
* This structure contains the main parameters for
* the intrinsic parameters of the camera
* representation. The camera used is a pinhole
* camera with standard parameters. User should
* consult a computer vision reference for more
* information. (e.g. Three-Dimensional Computer Vision 
* (Artificial Intelligence) by Olivier Faugeras).
* \param xsize length of the image (in pixels).
* \param ysize height of the image (in pixels).
* \param mat perspective matrix (K).
* \param dist_factor radial distortions factor
*          dist_factor[0]=x center of distortion
*          dist_factor[1]=y center of distortion
*          dist_factor[2]=distortion factor
*          dist_factor[3]=scale factor
*/
typedef struct {
    int      xsize, ysize;
    double   mat[3][4];
    double   dist_factor[4];
} ARParam;

typedef struct {
    int      xsize, ysize;
    double   matL[3][4];
    double   matR[3][4];
    double   matL2R[3][4];
    double   dist_factorL[4];
    double   dist_factorR[4];
} ARSParam;

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/** \fn int  arParamGet( double global[][3], double screen[][2], int data_num,
                 double mat[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param global XXXBK
* \param screen XXXBK
* \param data_num XXXBK
* \param mat XXXBK
* \return  XXXBK
*/
int  arParamGet( double global[][3], double screen[][2], int data_num,
                 double mat[3][4] );

/** \fn int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param source XXXBK
* \param icpara XXXBK
* \param trans XXXBK
* \return  XXXBK
*/
int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] );

/** \fn int arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param source input camera matrix
* \param cpara camera parameter to be set
* \param trans XXXBK
* \return  XXXBK
*/
int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] );

/** \fn int int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
						 double *ox, double *oy )

* \brief  Convert ideal screen coordinates of a vertex to observed ones. 
*
* Ideal coordinates mean that the distortion of the camera is compensated (so a straight line looks straight). 
* In observed coordinates the camera-distortion is not compensated and thus a straight line is not shown really straight.
* \param dist_factor distorsion factors of used camera
* \param ix x in ideal screen coordinates
* \param iy y in ideal screen coordinates
* \param ox resulted x in observed screen coordinates
* \param oy resulted y in observed screen coordinates
* \return 0 if success, -1 otherwise
*/
int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
                         double *ox, double *oy );

/** \fn int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy )

* \brief Convert observed screen coordinates of a vertex to ideal ones. 

* Ideal coordinates mean that the distortion of the camera is compensated (so a straight line looks straight).
* In observed coordinates the camera-distortion is not compensated and thus a straight line is not shown really straight.
* \param dist_factor distorsion factors of used camera
* \param ox x in observed screen coordinates
* \param oy y in observed screen coordinates
* \param ix resulted x in ideal screen coordinates
* \param iy resulted y in ideal screen coordinates
* \return 0 if success, -1 otherwise
*/
int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy );

/** \fn int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam )
* \brief change the camera size parameters.
*
* Change the size variable in camera intrinsic parameters.
* \param source name of the source parameters structure
* \param xsize new length size
* \param ysize new height size
* \param newparam name of the destination parameters structure.
* \return 0
*/
int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam );

/** \fn int arParamSave( char *filename, int num, ARParam *param, ...)
* \brief save a camera intrinsic parameters.
*
* Save manipulated camera intrinsic parameters in a file.
* \param filename name of the parameters file.
* \param num number of variable arguments
* \param param parameters to save
* \return 0 if success, -1 if Error (file not found, file structure problem)
*/
int    arParamSave( char *filename, int num, ARParam *param, ...);

/** \fn int arParamLoad( const char *filename, int num, ARParam *param, ...)
* \brief load the camera intrinsic parameters.
*
* Load camera intrinsic parameters in the ARToolkit Library from 
* a file (itself, an output of the calibration step). 
* \param filename name of the parameters file.
* \param num number of variable arguments
* \param param result of the loaded parameters
* \return 0 if success, -1 if Error (file not found, file structure problem)
*/
int    arParamLoad( const char *filename, int num, ARParam *param, ...);

/** \fn int arParamDisp( ARParam *param )
* \brief display parameters.
*
* Display the structure of the camera instrinsic parameters argument.
* \param param structure to display
* \return 0
*/
int    arParamDisp( ARParam *param );

/*-------------------*/

int    arsParamChangeSize( ARSParam *source, int xsize, int ysize, ARSParam *newparam );

int    arsParamSave( char *filename, ARSParam *sparam );

int    arsParamLoad( char *filename, ARSParam *sparam );

int    arsParamDisp( ARSParam *sparam );

int    arsParamGetMat( double matL[3][4], double matR[3][4],
                       double cparaL[3][4], double cparaR[3][4], double matL2R[3][4] );

#ifdef __cplusplus
}
#endif
#endif
