/*  --------------------------------------------------------------------------
*   Copyright (C) 2004 Hitlab NZ.
*   The distribution policy is describe on the Copyright.txt furnish 
*    with this library.
*   -------------------------------------------------------------------------*/
/**
*  \file arMulti.h
*  \brief ARToolkit multi pattern subroutines. 
*
*  This file extend ar core routines  for multi-pattern tracking. 
*  You can obtain position of a local coordinate 
*  system based on the estimation of multiple markers tracking (each in relative
*  position)
*
*   \remark more efficient with uniform planar configuration
*
*   History :
*
*  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp
*  \version 1.0
*  \date 01/09/05
**/
/*  --------------------------------------------------------------------------
*   History : 
*   Rev		Date		Who		Changes
*
*----------------------------------------------------------------------------*/

#ifndef AR_MULTI_H
#define AR_MULTI_H
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <stdio.h>
#include <AR/config.h>
#include <AR/param.h>
#include <AR/ar.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

	/** \struct ARMultiEachMarkerInfoT
* \brief multi-marker structure
*
* Structure for multi-marker tracking
* really similar to ARMarkerInfo
* \param patt_id identification of the pattern
* \param width width of the pattern (in mm)
* \param center center of the pattern (in mm)
* \param trans estimated position of the pattern
* \param itrans relative position of the pattern
* \param pos3d final position of the pattern
* \param visible boolean flag for visibility
* \param visibleR last state visibility
*/
typedef struct {
    int     patt_id;
    double  width;
    double  center[2];
    double  trans[3][4];
    double  itrans[3][4];
    double  pos3d[4][3];
    int     visible;
/*---*/
    int     visibleR;
} ARMultiEachMarkerInfoT;

/** \struct ARMultiMarkerInfoT
* \brief global multi-marker structure
*
* Main structure for multi-marker tracking.
* 
* \param marker list of markers of the multi-marker pattern
* \param marker_num number of markers used
* \param trans position of the multi-marker pattern (more precisely, the camera position in the multi-marker CS)
* \param prevF boolean flag for visibility
* \param transR last position
*/
typedef struct {
    ARMultiEachMarkerInfoT  *marker;
    int                     marker_num;
    double                  trans[3][4];
    int                     prevF;
/*---*/
    double                  transR[3][4];
} ARMultiMarkerInfoT;

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/**
* \brief loading multi-markers description from a file
*
* Load a configuration file for multi-markers tracking. The configuration
* file furnishs pointer to each pattern description.
*
* \param filename name of the pattern file
* \return a pattern structure, NULL if error
*/
ARMultiMarkerInfoT *arMultiReadConfigFile( const char *filename );

/**
* \brief compute camera position in function of the multi-marker patterns (based on detected markers)
* 
* calculate the transformation between the multi-marker patterns and the real camera. Based on 
* confident values of detected markers in the multi-markers patterns, a global position is return.
*
* \param marker_info list of detected markers (from arDetectMarker)
* \param marker_num number of detected markers
* \param config 
* \return 
*/
double  arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num,
                           ARMultiMarkerInfoT *config);

/**
* \brief activate a multi-marker pattern on the recognition procedure.
*
* Activate a multi-marker for be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiActivate( ARMultiMarkerInfoT *config );

/**
* \brief Desactivate a multi-marker pattern on the recognition procedure.
*
* Desactivate a multi-marker for not be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiDeactivate( ARMultiMarkerInfoT *config );

/**
* \brief remove a multi-marker pattern from memory.
*
* desactivate a pattern and remove it from memory. Post-condition
* of this function is unavailability of the multi-marker pattern.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiFreeConfig( ARMultiMarkerInfoT *config );

/*------------------------------------*/
double arsMultiGetTransMat(ARMarkerInfo *marker_infoL, int marker_numL,
                           ARMarkerInfo *marker_infoR, int marker_numR,
                           ARMultiMarkerInfoT *config);


#ifdef __cplusplus
}
#endif
#endif
