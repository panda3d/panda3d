/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 1.0
 * Date: 2002/01/01
 *
*******************************************************/
#ifndef AR_VIDEO_LINUX_1394CAM_H
#define AR_VIDEO_LINUX_1394CAM_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <AR/config.h>
#include <AR/ar.h>

#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>


typedef struct {
    int      node;
    int      mode;
    int      rate;
    int      debug;  
    int      card;
    int      channel;
    int      speed;
    int      format;
    int      dma_buf_num;
    int      int_mode;
    int      int_rate;
    int      status;

    int                    internal_id;
    dc1394_feature_set     features;
    dc1394_cameracapture   camera;
    ARUint8                *image;
} AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif
