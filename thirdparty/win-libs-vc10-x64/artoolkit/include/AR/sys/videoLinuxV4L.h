/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 4.5
 * Date: 2002/01/01
 * 
 * 2004/11/17 Grasset  adding new parameters for better controls of
 * V4L driver
 * 2004/11/17 Grasset  adding patch done by XXX for supporting YUV 4:2:0
 * (adding #define and videoBuffer encoding parameters)

*******************************************************/
#ifndef AR_VIDEO_LINUX_V4L_H
#define AR_VIDEO_LINUX_V4L_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev.h>

#include <AR/config.h>
#include <AR/ar.h>

typedef struct {
  //device controls
    char                dev[256];
    int                 channel;
    int                 width;
    int                 height;
    int			palette;
  //image controls
    double              brightness;
    double              contrast;
    double              saturation;
    double              hue;
    double              whiteness;

  //options controls
    int                 mode;

    int                 debug;

    int                 fd;
    int                 video_cont_num;
    ARUint8             *map;
    ARUint8             *videoBuffer;
    struct video_mbuf   vm;
    struct video_mmap   vmm;
} AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif
