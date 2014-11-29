/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 1.1
 * Date: 2002/01/01
 *
*******************************************************/
#ifndef AR_VIDEO_LINUX_DV_H
#define AR_VIDEO_LINUX_DV_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <AR/config.h>
#include <AR/ar.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libraw1394/raw1394.h>
#include <libdv/dv.h>

typedef struct {
    int              size;
    ARUint8         *buff_in;
    ARUint8         *buff_wait;
    ARUint8         *buff_out;
    int              fill_size_in;
    int              fill_size_wait;
    int              fill_size_out;
    int              read_size;
    pthread_mutex_t  mutex;
    int              init;
} AR2VideoBufferT;

typedef struct {
    int              mode;
    int              debug;
    int              status;
    raw1394handle_t  handle;
    pthread_t        capture;
    AR2VideoBufferT *buffer;
    int              packet_num;
    dv_decoder_t    *dv_decoder;
    ARUint8         *image;
} AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif
