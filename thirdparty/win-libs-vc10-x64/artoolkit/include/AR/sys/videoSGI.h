/*******************************************************
 *
 * Author: Hirokazu Kato, Atsishi Nakazawa
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *         nakazawa@inolab.sys.es.osaka-u.ac.jp
 *
 * Revision: 4.2
 * Date: 2002/01/01
 *
*******************************************************/
#ifndef AR_VIDEO_SGI_H
#define AR_VIDEO_SGI_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <AR/config.h>
#include <AR/ar.h>


typedef enum{
    AR_VIDEO_INTERLEAVED,
    AR_VIDEO_NONINTERLEAVED,
    AR_VIDEO_ODD,
    AR_VIDEO_EVEN
} ARVideoFormat;

typedef enum {
    AR_VIDEO_RGB_8,
    AR_VIDEO_RGB_332,
    AR_VIDEO_MONO,
    AR_VIDEO_YVYU
} ARVideoPacking;

typedef enum {
    AR_VIDEO_1_P_1,
    AR_VIDEO_1_P_2,
    AR_VIDEO_1_P_4,
    AR_VIDEO_1_P_8
} ARVideoZoom;

typedef enum {
    AR_VIDEO_INDY,
    AR_VIDEO_O2,
    AR_VIDEO_GALILEO,
    AR_VIDEO_OCTANE,
    AR_VIDEO_IMPACT
} ARVideoDeviceType;

#define ARVideoDeviceMax   4

typedef struct {
    ARVideoDeviceType  type[ARVideoDeviceMax];
    int                num;
} ARVideoDeviceTypeList;


typedef struct {
    int             did;
    int             width;
    int             height;
    ARVideoFormat   format;
    ARVideoPacking  packing;
    ARVideoZoom     zoom;
    int             buf_size;
} AR2VideoParamT;

int  arVideoOpen2( void );
int  arVideoClose2( void );

int  arVideoInqDevice2( ARVideoDeviceTypeList *dev_list );
int  arVideoInqSize2( int dev_id, int *x, int *y );

int  arVideoSetupDevice2( int             dev_id,
                          ARVideoFormat   format,
                          ARVideoPacking  packing,
                          ARVideoZoom     zoom    );
int  arVideoCleanupDevice2( int dev_id );


int  arVideoStart2( int dev_id );
int  arVideoStop2( int dev_id );

unsigned char *arVideoGetImage2( int dev_id );

int  arVideoSetBufferSize2( int dev_id, int size );

#ifdef  __cplusplus
}
#endif
#endif
